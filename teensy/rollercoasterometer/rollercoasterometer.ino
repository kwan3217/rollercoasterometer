#include "icm20948.h"
#include "icm42688.h"

#ifdef ARDUINO_TEENSY41
#include <TeensyView.h>
static const int TV_SCK  =13;
static const int TV_MOSI =11;
static const int TV_RESET= 2; //This is alternate -- standard 15 is used by PPS
static const int TV_DC   = 5;
static const int TV_CS   =10;
#define FIRMWARE_NAME  "Rollercoasterometer 22.06"
#define TZ "MST/MDT"
TeensyView tv(TV_RESET, TV_DC, TV_CS, TV_SCK, TV_MOSI);
#endif
ICM20948 icm2(0);
ICM42688 icm4(1);

void setupPins() {
  //USB Micro-B end
  //Teensy GND               
  pinMode( 0,INPUT); 
  pinMode( 1,INPUT); 
  pinMode( 2,INPUT); //TeensyView ~RESET *ALT*
  pinMode( 3,INPUT); 
  pinMode( 4,INPUT); 
  pinMode( 5,INPUT); //TeensyView D/~C STD
  pinMode( 6,INPUT); 
  pinMode( 7,INPUT); 
  pinMode( 8,INPUT); 
  pinMode( 9,INPUT); 
  pinMode(10,INPUT); 
  pinMode(11,INPUT); 
  pinMode(12,INPUT); 
  //Teensy 3.3V      
  pinMode(24,INPUT); 
  pinMode(25,INPUT); 
  pinMode(26,INPUT); 
  pinMode(27,INPUT); 
  pinMode(28,INPUT); 
  pinMode(29,INPUT); 
  pinMode(30,INPUT); 
  pinMode(31,INPUT); 
  pinMode(32,INPUT); 
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
  pinMode(17,INPUT); 
  pinMode(16,INPUT); 
  pinMode(15,INPUT); 
  pinMode(14,INPUT); 
  pinMode(13,OUTPUT);
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


void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  setupPins();
  setupTeensyView();
  Wire.begin();
  Wire.setClock(400000);

  Serial.print("ICM42688 who_am_i: 0x");
  Wire.beginTransmission(0x69);
  Wire.write(0x76);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(0x69);
  Wire.write(0x75);
  Wire.endTransmission(false);
  Wire.requestFrom(0x69,1);
  Serial.println(Wire.read(),HEX);

  icm2.begin();
  tv.clear(PAGE);
  tv.setCursor(0,0);
  tv.print("ICM2 whoami (sb 0xEA): 0x");
  tv.println(icm2.whoami(),HEX);
  tv.display();
  delay(5000);
}

uint32_t ota=0;

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t ta;
  int16_t ax,ay,az,gx,gy,gz,T;
  Serial.println("-----");
  icm2.query(ta,ax,ay,az,gx,gy,gz,T);
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
  ota=ta;
  tv.print("  T: ");
  tv.print(T);
  tv.display();
  delay(1000);
}
