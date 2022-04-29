#include "icm20948.h"
#include "icm42688.h"
#include <TeensyView.h>
#include "packet.h"

ICM20948 icm2(1);
//ICM42688 icm4(1);

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
SdFile root;
File ouf;
ubloxPacket packet(ouf);

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
  pinMode(29,INPUT); //DOWN
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
  } else {
    Serial.print("Couldn't open ");
    Serial.println(oufn);
  }


  packet.start('K',0x01);
  packet.print(FIRMWARE_NAME);
  packet.finish();
  ouf.flush();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  setupPins();
  setupTeensyView();
  setupSD();
  Wire.begin();
  Wire.setClock(400000);

  icm2.begin();
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.print("ICM2 whoami (sb 0xEA): 0x");
  tv.println(icm2.whoami(),HEX);
  tv.display();
  delay(5000);
}

uint32_t ota=0;
const int flush_size=32768;
int to_write=flush_size;

void tv_icm2(uint32_t ta, uint32_t ota, int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz, int16_t T) {
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.print("ax: ");
  tv.print(ax);
  tv.print("  gx: ");
  tv.print(gx);
  tv.setCursor(0,8);
  tv.print("ay: ");
  tv.print(ay);
  tv.print("  gy: ");
  tv.print(gy);
  tv.setCursor(0,16);
  tv.print("az: ");
  tv.print(az);
  tv.print("  gz: ");
  tv.print(gz);
  tv.setCursor(0,24);
  tv.print("dt: ");
  tv.print(ta-ota);
  tv.print("  T: ");
  tv.print(T);
  tv.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t ta;
  int16_t ax,ay,az,gx,gy,gz,T;
  icm2.query(ta,ax,ay,az,gx,gy,gz,T);
  packet.start('K',0x02);
  packet.write_le(ta);
  packet.write_le(ta-ota);
  packet.write_le(ax);
  packet.write_le(ay);
  packet.write_le(az);
  packet.write_le(gx);
  packet.write_le(gy);
  packet.write_le(gz);
  packet.write_le(T);
  to_write-=packet.finish();
  if(to_write<=0) {
    ouf.flush();
    to_write=flush_size;
    tv_icm2(ta,ota,ax,ay,az,gx,gy,gz,T);
  }
  ota=ta;
  //delay(1000);
}
