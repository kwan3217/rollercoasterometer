#include "icm20948.h"
#include "icm42688.h"
#include "bme680.h"
#include "lsm9ds1.h"
#include <TeensyView.h>
#include "bits.h"
#include "packet.h"
#include "gpt.h"
#include "clocktree.h"
#include "gps.h"
#include "output.h"
#include <Watchdog_t4.h>

GPT gpt(2);

ICM20948 icm2(1,Wire2);
AK09916 ak(gpt,Wire2);
//ICM42688 icm4(1);
BME680 bme(gpt,Wire2);
HardwareSerial& GPSSerial=Serial2;
LSM9DS1 lsm(1,Wire2);

static const int TV_SCK  =13;
static const int TV_MOSI =11;
static const int TV_RESET= 2; //This is alternate -- standard 15 is used by PPS
static const int TV_DC   = 5;
static const int TV_CS   =10;
#define FIRMWARE_NAME  "Rollercoasterometer 22.06"
#define TZ "MST/MDT"
TeensyView tv(TV_RESET, TV_DC, TV_CS, TV_SCK, TV_MOSI);

#include <SD.h>
#include <SPI.h>
Sd2Card card;
SdVolume volume;
File ouf;
OutputBuffer<0x10000> outbuf;
ubloxPacket<> packet;
GPSPacketBuffer gpsbuf;
WDT_T4<WDT1> wdt;

uint32_t pps_pulses =0;    ///<Number of pulses seen
uint32_t pps_ticks  =0;    ///<Number of timer counts seen 
uint32_t pps_dt     =0;    ///< timer ticks between last two pulses
bool     has_pps    =false;
uint32_t intr_pulses=0;
uint32_t intr_ticks =0;
uint32_t intr_dt    =0;
bool     has_intr   =false;
uint32_t imu_pkts   =0;
uint32_t mag_pkts   =0;
uint32_t bmp_pkts   =0;
uint32_t pps_pkts   =0;
uint32_t lsm_pkts   =0;

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// Teensy audio board: pin 10
// Teensy 3.5 & 3.6 on-board: BUILTIN_SDCARD
// Wiz820+SD board: pin 4
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
const int chipSelect = BUILTIN_SDCARD;

int filenum=1;

void num_filename(char* fn, int num) {
  const int digits=5;
  int mod0=1;
  for(int i=0;i<digits;i++) {
    int digit=num/mod0%10;
    fn[digits-i-1]=digit+'0';
    mod0*=10;
  }
}

char oufn[]="LOG00000.TXT";

void setupPins() {
  //Run this BEFORE setting any special pins for special purposes. Normally pinMode(...,INPUT) is
  //considered "safe" because it sets the pin to HiZ, but it also messes up any special pin 
  //settings IE for GPT etc.
  //USB Micro-B end
  //Teensy GND               
  pinMode( 0,INPUT); 
  pinMode( 1,INPUT); 
  pinMode( 2,INPUT); //TeensyView ~RESET *ALT*
  pinMode( 3,INPUT); //P0 INT
  pinMode( 4,INPUT); //P0 ID
  pinMode( 5,INPUT); //TeensyView D/~C STD
  pinMode( 6,INPUT); 
  pinMode( 7,INPUT); //RX2/TX_GPS
  pinMode( 8,INPUT); //TX2/RX_GPS
  pinMode( 9,INPUT); //P1 ID
  pinMode(10,INPUT); //TV_~CS
  pinMode(11,INPUT); //TV_DA
  pinMode(12,INPUT); //P1_INT
  //Teensy 3.3V      
  pinMode(24,INPUT); //SCL2
  pinMode(25,INPUT); //SDA2
  pinMode(26,INPUT); //P2 INT
  pinMode(27,INPUT); //BTN_3V3
  pinMode(28,INPUT); //UP
  pinMode(29,OUTPUT); //Buzzer
  pinMode(30,INPUT); //LEFT
  pinMode(31,INPUT); //RIGHT
  pinMode(32,INPUT); //PUSH
  //MicroSD end      
                     
  //USB Micro-B end  
  //Vin (5V USB)     
  //Analog GND       
  //3.3V             
  pinMode(23,INPUT); 
  pinMode(22,INPUT); 
  pinMode(21,INPUT); 
  pinMode(20,INPUT); 
  pinMode(19,INPUT);   //SCL0
  pinMode(18,INPUT);   //SDA0
  pinMode(17,INPUT);   //SDA1
  pinMode(16,INPUT);   //SCL1
  pinMode(15,INPUT);   //PPS
  pinMode(14,INPUT); 
  pinMode(13,OUTPUT);  //TV_SCK STD
  //GND              
  pinMode(41,INPUT);   
  pinMode(40,INPUT); 
  pinMode(39,INPUT); 
  pinMode(38,INPUT); 
  pinMode(37,INPUT); 
  pinMode(36,INPUT); 
  pinMode(35,INPUT); 
  pinMode(34,INPUT); 
  pinMode(33,INPUT); 
  //MicroSD end
}

void setupTeensyView() {
  tv.setClockRate(10'000'000);
  tv.begin();
  tv.flipVertical(true);
  tv.flipHorizontal(true);
  tv.clear(PAGE);
  tv.setFontType(0);
  tv.setCursor(0, 0);
  tv.print(FIRMWARE_NAME);
  tv.setCursor(0, 8);
  tv.print(__DATE__);
  tv.setCursor(0,16);
  tv.print(__TIME__ " " TZ);
  tv.display();
}

void setupSD() {
  FsDateTime::setCallback(dateTime);
  SD.begin(chipSelect);

  //Decide on the filename, but don't open the file yet
  num_filename(oufn+3,filenum);
  Serial.print("Checking filename: ");
  Serial.println(oufn);
  while (SD.exists(oufn)) {
    filenum++;
    num_filename(oufn+3,filenum);
    Serial.print("Checking filename: ");
    Serial.println(oufn);
  }

  //Open the file if needed
  ouf=SD.open(oufn, FILE_WRITE);
  if(ouf) {
    Serial.print("Opened file ");
    Serial.println(oufn);
    outbuf.begin(&ouf);
    gpsbuf.begin(&outbuf);
    packet.begin(&outbuf);
    packet.start('K',0x01,"VER","Name of firmware being used");
    packet.print(FIRMWARE_NAME,"firmware_name","Name of firmware being used");
    packet.finish();
    uint32_t srsr=SRC_SRSR;
    Serial.printf("SRSR: 0x%08x\n",srsr);
    SRC_SRSR=SRC_SRSR;
    packet.start('K',0x05,"RST","Reset Cause");
    packet.write_le<uint32_t>(srsr,"src_srsr","Value of SRC_SRSR register. Set bits indicate what caused the reset event.");
    packet.finish();
  } else {
    Serial.print("Couldn't open ");
    Serial.println(oufn);
  }

}

void dateTime(uint16_t* date, uint16_t* time, uint8_t* ms10) {
  
  // Return date using FS_DATE macro to format fields.
  *date = FS_DATE(gpsbuf.year, gpsbuf.month, gpsbuf.day);

  // Return time using FS_TIME macro to format fields.
  *time = FS_TIME(gpsbuf.hour, gpsbuf.minute, gpsbuf.second);

  // Return low time bits in units of 10 ms.
  *ms10 = 0;
}

void watchdogWarning() {
  Serial.println("Watchdog warning");yield();
}

void setupWatchdog() {
  WDT_timings_t config;
  config.trigger = 1; /* in seconds, 0->128 */
  config.timeout = 1; /* in seconds, 0->128 */
  config.callback = nullptr;
  wdt.begin(config);
}

uint8_t rxbuf[0x8000];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  ClockTree::begin();
  Serial.print("put_bits(0x12345678,19,12,0) (should be 0x12300678): 0x");
  Serial.println(put_bits(0x12345678,19,12,0),HEX);
  Serial.print("put_bits(0x12345678,19,12,0xFFF) (should be 0x123FF678): 0x");
  Serial.println(put_bits(0x12345678,19,12,0xFFF),HEX);
  Serial.print("get_bits(0x12345678,19,12) (should be 0x45): 0x");
  Serial.println(get_bits(0x12345678,19,12),HEX);
  Serial.print("CCM_CSCMR1: 0x");
  Serial.println(CCM_CSCMR1,HEX);
  Serial.print("CCM_CCGR0: 0x");
  Serial.println(CCM_CCGR0,HEX);
  Serial.print("CCM_CCGR1: 0x");
  Serial.println(CCM_CCGR1,HEX);
  Serial.print("CCM_CCGR2: 0x");
  Serial.println(CCM_CCGR2,HEX);
  Serial.print("CCM_CCGR3: 0x");
  Serial.println(CCM_CCGR3,HEX);
  Serial.print("CCM_CCGR4: 0x");
  Serial.println(CCM_CCGR4,HEX);
  Serial.print("CCM_CCGR5: 0x");
  Serial.println(CCM_CCGR5,HEX);
  Serial.print("CCM_CCGR6: 0x");
  Serial.println(CCM_CCGR6,HEX);
  Serial.print("CCM_CCGR7: 0x");
  Serial.println(CCM_CCGR7,HEX);
  digitalWrite(13,LOW);
  setupPins();
  gpt.begin();
  gpt.setCaptureEnable(1,GPT::RISE,false);
  gpt.setCaptureEnable(2,GPT::RISE,false);
  setupTeensyView();
  setupSD();
  Serial.print("Here");yield();
  Wire2.begin();
  Wire2.setClock(400000);
 // Wire2.setWireTimeout(3000,false); //3ms timeout, no reset
  icm2.begin();
  ak.begin(1);
  bme.begin();
  bme.query_coef(true);
  lsm.begin();
  lsm.dumpRegs();
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.print("ICM2 (sb 0xEA): 0x");
  tv.println(icm2.whoami(),HEX);
  tv.setCursor(0,8);
  tv.print("AK (sb 0x948): 0x");
  tv.println(ak.whoami(),HEX);
  tv.setCursor(0,16);
  tv.print("BME (sb 0x61): 0x");
  tv.println(bme.whoami(),HEX);
  tv.setCursor(0,24);
  tv.print("LSM (sb 0x68): 0x");
  tv.println(lsm.whoami(),HEX);
  tv.display();

  delay(5000);

  //Start up serial port, with extra memory for taking care of things while we are
  //doing stuff like writing the SD card
  GPSSerial.begin(230400);
  GPSSerial.addMemoryForRead(rxbuf,sizeof(rxbuf));

  setupWatchdog();
  writePressureCal_packet();
}

uint32_t ta,tb,tl,tp0,tp1;
int16_t ax,ay,az,bx,by,bz,gx,gy,gz,iT;
int16_t axl,ayl,azl,bxl,byl,bzl,gxl,gyl,gzl,lT;
uint8_t ast,lst,bst1,bst2,pst;
uint32_t rP,rT;
uint16_t rh;
float P,T,h;


void writePressureCal_packet() {
  packet.start('K',0x07,"PCAL","Calibration parameters for BME680");
  packet.write_le(bme.par_t1,"t1" ,"Temperature calibration coefficient 1");
  packet.write_le(bme.par_t2,"t2" ,"Temperature calibration coefficient 2");
  packet.write_le(bme.par_t3,"t3" ,"Temperature calibration coefficient 3");
  packet.write_le(bme.par_p1,"p1" ,"Pressure calibration coefficient 1");
  packet.write_le(bme.par_p2,"p2" ,"Pressure calibration coefficient 2");
  packet.write_le(bme.par_p3,"p3" ,"Pressure calibration coefficient 3");
  packet.write_le(bme.par_p4,"p4" ,"Pressure calibration coefficient 4");
  packet.write_le(bme.par_p5,"p5" ,"Pressure calibration coefficient 5");
  packet.write_le(bme.par_p6,"p6" ,"Pressure calibration coefficient 6");
  packet.write_le(bme.par_p7,"p7" ,"Pressure calibration coefficient 7");
  packet.write_le(bme.par_p8,"p8" ,"Pressure calibration coefficient 8");
  packet.write_le(bme.par_p9,"p9" ,"Pressure calibration coefficient 9");
  packet.write_le(bme.par_p10,"p10","Pressure calibration coefficient 10");
  packet.write_le(bme.par_h1,"h1" ,"Humidity calibration coefficient 1");
  packet.write_le(bme.par_h2,"h2" ,"Humidity calibration coefficient 2");
  packet.write_le(bme.par_h3,"h3" ,"Humidity calibration coefficient 3");
  packet.write_le(bme.par_h4,"h4" ,"Humidity calibration coefficient 4");
  packet.write_le(bme.par_h5,"h5" ,"Humidity calibration coefficient 5");
  packet.write_le(bme.par_h6,"h6" ,"Humidity calibration coefficient 6");
  packet.write_le(bme.par_h7,"h7" ,"Humidity calibration coefficient 7");
  packet.finish();
}

void writeInertial_packet() {
  packet.start('K',0x02,"ICM2","Inertial measurement from ICM-20948");
  packet.write_le(ta,"t","Timestamp of accelerometer measurement");
  packet.write_le(ax,"ax","Accelerometer X measurement");
  packet.write_le(ay,"ay","Accelerometer Y measurement");
  packet.write_le(az,"az","Accelerometer Z measurement");
  packet.write_le(gx,"gx","Gyroscope X measurement");
  packet.write_le(gy,"gy","Gyroscope Y measurement");
  packet.write_le(gz,"gz","Gyroscope Z measurement");
  packet.write_le(iT,"T","Inertial sensor temperature measurement");
  packet.write_le(ast,"st","Sensor status");
  packet.finish();
  imu_pkts++;
//  Serial.println(ax);
}

void writeLsm_packet() {
  packet.start('K',0x08,"LSM","Inertial measurement from LSM9DS1");
  packet.write_le(tl,"t","Timestamp of accelerometer measurement");
  packet.write_le(axl,"ax","Accelerometer X measurement");
  packet.write_le(ayl,"ay","Accelerometer Y measurement");
  packet.write_le(azl,"az","Accelerometer Z measurement");
  packet.write_le(gxl,"gx","Gyroscope X measurement");
  packet.write_le(gyl,"gy","Gyroscope Y measurement");
  packet.write_le(gzl,"gz","Gyroscope Z measurement");
  packet.write_le(lT,"T","Inertial sensor temperature measurement");
  packet.write_le(lst,"st","Sensor status");
  packet.finish();
  lsm_pkts++;
//  Serial.println(ax);
}

void writeMagnetic_packet() {
  packet.start('K',0x03,"MAG","Magnetic measurement from AK09916");
  packet.write_le(tb,"tb","Timestamp of magnetic measurement");
  packet.write_le(bst1,"st1","Magnetic status1");
  packet.write_le(bx,"bx","Magnetic X measurement");
  packet.write_le(by,"by","Magnetic Y measurement");
  packet.write_le(bz,"bz","Magnetic Z measurement");
  packet.write_le(bst2,"st2","Magnetic status2");
  packet.finish();
  mag_pkts++;
}

void writePressure_packet() {
  packet.start('K',0x06,"PRES","Atmosphere measurement from BME680");
  packet.write_le(tp0,"tp0","Timestamp of beginning of measurement");
  packet.write_le(tp1,"tp1","Timestamp of ending of measurement");
  packet.write_le(pst,"pst","Measurement status");
  packet.write_le(rT,"rT","Raw temperature measurement");
  packet.write_le(rP,"rP","Raw pressure measurement");
  packet.write_le(rh,"rh","Raw humidity measurement");
  packet.write_le(T,"T","Calibrated temperature, degC");
  packet.write_le(P,"P","Calibrated pressure, Pa");
  packet.write_le(h,"h","Calibrated relative humidity, percent");
  packet.finish();
  bmp_pkts++;
}

void tv_icm2() {
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.printf("ax:%6d gx:%6d",ax,gx);
  tv.setCursor(0,8);
  tv.printf("ay:%6d gy:%6d",ay,gy);
  tv.setCursor(0,16);
  tv.printf("az:%6d gz:%6d",az,gz);
  tv.setCursor(0,24);
  tv.print("iT: ");
  tv.print(iT);
  tv.display();
}

void tv_lsm() {
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.printf("lax:%6d gx:%6d",axl,gxl);
  tv.setCursor(0,8);
  tv.printf("lay:%6d gy:%6d",ayl,gyl);
  tv.setCursor(0,16);
  tv.printf("laz:%6d gz:%6d",azl,gzl);
  tv.setCursor(0,24);
  tv.printf("st 0x%02x lT: %6d",lst,lT);
  tv.print(iT);
  tv.display();
}

void tv_akbme() {
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.printf("bx:%6d P:%f",bx,P);
  tv.setCursor(0,8);
  tv.printf("by:%6d T:%f",by,T);
  tv.setCursor(0,16);
  tv.printf("bz:%6d h:%f",bz,h);
  tv.setCursor(0,24);
  tv.printf("st1:0x%02x st2:0x%02x",bst1,bst2);
  tv.display();
}

void tv_serial() {
  tv.setCursor(0,0);
  tv.printf("badPkt:%2d ppkt: %4d",gpsbuf.badPkt,pps_pkts);
  tv.setCursor(0,8);
  tv.printf("ap: %6d bp: %6d",imu_pkts,mag_pkts);
  tv.setCursor(0,16);
  tv.printf("Pp: %6d lp: %6d",bmp_pkts,lsm_pkts);
  tv.setCursor(0,24);
  tv.print("rxBytes: ");
  print_comma(tv,gpsbuf.data_received);
}

void tv_sd() {
  tv.setCursor(0,0);
  tv.printf("%s %c",oufn,ouf?'A':'V');
  if(ouf) {
    tv.setCursor(0,8);
    tv.print("fpos: ");
    print_comma(tv,ouf.position());
    tv.setCursor(0,16);
    tv.print("pend: ");
    print_comma(tv,outbuf.get_ptr());
  }
}

bool countPulse(uint32_t channel, bool& has_pulse, uint32_t& pulses, uint32_t& ticks, uint32_t& dt) {
  uint32_t this_ticks=gpt.ICR(channel);
  if(this_ticks==ticks) return;
  dt=GPT::dt(ticks,this_ticks);
  ticks=this_ticks;
  pulses++;
  has_pulse=true;
}

void tv_pps() { 
  tv.setCursor(0,0);
  tv.print("pulses: ");
  print_comma(tv,pps_pulses);
  tv.setCursor(0,8);
  tv.print("t1: ");
  print_comma(tv,pps_ticks);
  tv.setCursor(0,16);
  tv.print("dt: ");
  print_comma(tv,pps_dt);
  tv.setCursor(0,24);
  tv.print("freq: ");
  tv.print(60'000'000.0/double(pps_dt),7);
}

void tv_intr() {
  tv.setCursor(0,0);
  tv.print("intrs: ");
  print_comma(tv,intr_pulses);
  tv.setCursor(0,8);
  tv.print("t2: ");
  print_comma(tv,intr_ticks);
  tv.setCursor(0,16);
  tv.print("dt: ");
  print_comma(tv,intr_dt);
  tv.setCursor(0,24);
  tv.print("freq: ");
  tv.print(60'000'000.0/double(intr_dt));
}

void tv_gps() {
  tv.setCursor(0,0);
  tv.printf("lon:  %14.9f",gpsbuf.lon);
  tv.setCursor(0,8);
  tv.printf("lat:  %14.9f",gpsbuf.lat);
  tv.setCursor(0,16);
  tv.printf("hMSL: %11.4f",gpsbuf.hMSL);
  tv.setCursor(0,24);
  tv.printf("%04d-%02d-%02d %02d:%02d:%04.1f",gpsbuf.year,gpsbuf.month,gpsbuf.day,gpsbuf.hour,gpsbuf.minute,gpsbuf.second);
}

typedef void (*fvoid)();

fvoid tv_screens[]={tv_lsm,tv_gps,tv_lsm,tv_icm2,tv_lsm,tv_akbme,tv_lsm,tv_serial,tv_lsm,tv_sd,tv_lsm,tv_pps,tv_lsm,tv_intr};

/**
 * Write to the TeensyView
 */
void writeTv() {
  static uint32_t prev_slice=0;
  uint32_t slice=millis()/250;
  if(slice==prev_slice) return;
  prev_slice=slice;
  uint32_t screen=slice/20;
  screen=screen%(sizeof(tv_screens)/sizeof(fvoid));
  //Serial.printf("Screen %d\n",screen);
  tv.clear(PAGE);
  tv_screens[screen%(sizeof(tv_screens)/sizeof(fvoid))]();
  tv.display();
}

void readSensors() {
  if(!has_intr) return;
  has_intr=false;
  ta=intr_ticks; //Note the capture time as the official time of the measurement
  if(icm2.query(ast,ax,ay,az,gx,gy,gz,iT)) {
    writeInertial_packet();
  }
  if(lsm.query(lst,axl,ayl,azl,gxl,gyl,gzl,lT)) {
    tl=gpt.CNT();
    writeLsm_packet();
  }
  if (ak.query(bst1,bx,by,bz,bst2)) {
    //Timing is approximately the measurement time plus the readout time
    tb=gpt.CNT();
    writeMagnetic_packet();
  }
  if(bme.query(tp0, tp1, pst, rT, rP, rh,T,P,h)) {
    writePressure_packet();
  }
}

void print_comma(Print& out, uint32_t val) {
  uint32_t digit_pos=10;
  uint32_t digit_mod=1'000'000'000;
  bool seen_digit=false;
  while(digit_pos>0) {
    uint32_t this_digit=val/digit_mod;
    val=val%digit_mod;
    digit_pos--;
    digit_mod/=10;
    if(this_digit>0) seen_digit=true;
    if(seen_digit) {
      out.print(this_digit,DEC);
      if(digit_pos%3==0 && digit_pos>0) out.print(',');
    }
  }
  if(!seen_digit) out.print('0');
}

void countPulses() {
  countPulse(1,has_pps, pps_pulses, pps_ticks, pps_dt);
  countPulse(2,has_intr,intr_pulses,intr_ticks,intr_dt);
}

void readPPS() {
  if(!has_pps) return;
  has_pps=false;
  pps_pkts++;
  packet.start('K',0x04,"TP","GPS time pulse");
  packet.write_le(pps_pulses,"pulses","Time pulses since reset");
  packet.write_le(pps_ticks,"t","Timer capture measurement of time pulse");
  packet.write_le(pps_dt,"dt","Timer ticks since last time pulse");
  packet.finish();
}

void readGPS() {
  int incomingByte;
  while(GPSSerial.available()) {
    gpsbuf.write(GPSSerial.read());
  }
}

void click() {
  wdt.feed();
  static uint64_t clickCount=0;
  static uint32_t tlast=0;
  static uint64_t clickCount_last=0;
  if(clickCount%2000==0) {
    digitalWrite(29,(clickCount / 2000) & 0x01);
  }
  if(clickCount%1'000'000==0) {
    packet.start('K',0x09,"LOOP","Loop count");
    uint32_t t=gpt.CNT();
    packet.write_le(t,"t","Timestamp");
    packet.write_le(clickCount,"loopCount","Number of calls to loop() mod 2^32"); 
    packet.finish();
    Serial.printf("t: %f  count: %d  rate: %f Hz\n",double(t)/60'000'000.00,clickCount,float(clickCount-clickCount_last)/(float(GPT::dt(tlast,t))/60'000'000.0));
    tlast=t;
    clickCount_last=clickCount;
  }
  clickCount++;
}

void loop() {
  // put your main code here, to run repeatedly:
  click();
  countPulses();
  readSensors();
  readPPS();
  readGPS();
  writeTv();
}
