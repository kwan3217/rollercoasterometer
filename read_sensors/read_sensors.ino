#include <Wire.h>
#include "i2c_sensor.h"
#include "icm20948.h"
#include "icm42688.h"
#include "bme680.h"

ICM20948 imu2(0);
ICM42688 imu4(1);
AK09916 mag;
BME680 bme;
uint8_t buf[2+2+2+4+2*6+2+2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  //  76543210
  //  10101010
  //       
  Wire.begin();
  Wire.setClock(400000);
  Serial.print("IMU2 address:  0x");
  Serial.println(imu2.i2c_addr,HEX);
  Serial.print("IMU2 page reg: 0x");
  Serial.println(imu2.get_bank(),HEX);
  Serial.print("IMU2 whoami (should be 0xEA):   0x");
  Serial.println(imu2.whoami(),HEX);
  Serial.println("imu2.begin()");
  imu2.begin();
  Serial.print("IMU4 address:  0x");
  Serial.println(imu4.i2c_addr,HEX);
  Serial.print("IMU4 page reg: 0x");
  Serial.println(imu4.get_bank(),HEX);
  Serial.print("IMU4 whoami (should be 0x47):   0x");
  Serial.println(imu4.whoami(),HEX);
  Serial.println("imu4.begin()");
  imu4.begin();
  Serial.println("mag.begin()");
  mag.begin();
  Serial.println("bme.begin()");
  bme.begin(0,0,0,0);
  delay(100);
  buf[0]=0xb5;buf[1]=0x62;buf[2]='k';buf[3]=1;buf[4]=sizeof(buf);buf[5]=0;
  Serial.print("Mag whoami (should be 0x948):  0x");
  Serial.println(mag.whoami(),HEX);
  Serial.print("BME whoami (should be 0x61) :  0x");
  Serial.println(bme.whoami(),HEX);
  uint8_t bbuf[16];
  for(int i=0;i<256;i+=16) {
    if(i==0) Serial.print("0");
    Serial.print(i,HEX);
    Serial.print(":  ");
    bme.read(i,bbuf,sizeof(bbuf));
    for(int j=0;j<16;j++) {
      if(bbuf[i+j]<0x10) Serial.print("0");
      Serial.print(bbuf[i+j],HEX);
      if(j==7) Serial.print("-");
    }
    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t ta2,ta4,tb,tp;
  uint8_t ast2,ast4,bst1,bst2,pst;
  uint8_t awho2=imu2.whoami();
  uint8_t awho4=imu4.whoami();
  uint16_t bwho=mag.whoami();
  uint8_t pwho=bme.whoami();
  int16_t ax2,ay2,az2,bx,by,bz,gx2,gy2,gz2,T2;
  int16_t ax4,ay4,az4,         gx4,gy4,gz4,T4;
  uint32_t rawT,rawP;
  uint16_t rawH;
  imu2.read(ta2,ax2,ay2,az2,gx2,gy2,gz2,T2,ast2);
  imu4.read(ta4,ax4,ay4,az4,gx4,gy4,gz4,T4,ast4);
  bool bready=mag.read(tb,bst1,bx,by,bz,bst2);
  bool pready=bme.read(tp,pst,rawT,rawP,rawH);
  if(true) {
    Serial.println('---');
    Serial.print("ta2: ");
    Serial.print(ta2);
    Serial.print("\u03bcs awho2(sb 0xEA): 0x");
    Serial.print(awho2 & 0xFF, HEX);
    Serial.print(" ax2: ");
    Serial.print(ax2);
    Serial.print(" DN  ay2: ");
    Serial.print(ay2);
    Serial.print(" DN  az2: ");
    Serial.print(az2);
    Serial.print(" DN  gx2: ");
    Serial.print(gx2);
    Serial.print(" DN  gy2: ");
    Serial.print(gy2);
    Serial.print(" DN  gz2: ");
    Serial.print(gz2);
    Serial.print(" DN  T2: ");
    Serial.print(T2);
    Serial.println(" DN");
    Serial.print("ta2: ");
    Serial.print(ta4);
    Serial.print("\u03bcs awho4(sb 0x47): 0x");
    Serial.print(awho4 & 0xFF, HEX);
    Serial.print(" ax4: ");
    Serial.print(ax4);
    Serial.print(" DN  ay4: ");
    Serial.print(ay4);
    Serial.print(" DN  az4: ");
    Serial.print(az4);
    Serial.print(" DN  gx4: ");
    Serial.print(gx4);
    Serial.print(" DN  gy4: ");
    Serial.print(gy4);
    Serial.print(" DN  gz4: ");
    Serial.print(gz4);
    Serial.print(" DN  T4: ");
    Serial.print(T4);
    Serial.println(" DN");
    if(bready) {
      Serial.print("tb: ");
      Serial.print(tb);
      Serial.print("\u03bcs  bwho(sb 0x948): ");
      Serial.print(bwho,HEX);
      Serial.print(" bst1: 0x");
      Serial.print(bst1,HEX);
      Serial.print("  bx: ");
      Serial.print(bx);
      Serial.print(" DN  by: ");
      Serial.print(by);
      Serial.print(" DN  bz: ");
      Serial.print(bz);
      Serial.print(" DN  bst2: 0x");
      Serial.println(bst2,HEX);
    }
    if(pready) {
      Serial.print("tp: ");
      Serial.print(tp);
      Serial.print("\u03bcs bwho(sb 0x61): ");
      Serial.print(pwho,HEX);
      Serial.print(" pst: 0x");
      Serial.print(pst,HEX);
      Serial.print("  T: ");
      Serial.print(rawT);
      Serial.print(" DN  P: ");
      Serial.print(rawP);
      Serial.print(" DN  H: ");
      Serial.print(rawH);
      Serial.println(" DN");
    }
  } else {
    buf[3]=1;
    uint16_t payload_len=4+6*2+2; //payload length only -- timestamp, 6 acc and gyro readings, 1 temperature
    putu16le(buf+4,payload_len);
    memcpy(  buf+2+2+2     ,&ta2,sizeof(ta2));
    puti16le(buf+2+2+2+4+ 0,ax2);
    puti16le(buf+2+2+2+4+ 2,ay2);
    puti16le(buf+2+2+2+4+ 4,az2);
    puti16le(buf+2+2+2+4+ 6,gx2);
    puti16le(buf+2+2+2+4+ 8,gy2);
    puti16le(buf+2+2+2+4+10,gz2);
    puti16le(buf+2+2+2+4+12,T2);
    putcksum(buf+2,2+2+payload_len);
    Serial.write(buf,payload_len+8);
    if (bready) {
      buf[3]=2;
      payload_len=       4+1+3*2+1; //payload length only -- timestamp, st1, 3 mag readings, st2
      putu16le(buf+4,payload_len);
      memcpy(buf+2+2+2,&tb,sizeof(tb));
               buf[2+2+2+4+ 0]=bst1;
      puti16le(buf+2+2+2+4+ 1,bx);
      puti16le(buf+2+2+2+4+ 3,by);
      puti16le(buf+2+2+2+4+ 5,bz);
               buf[2+2+2+4+ 6]=bst2;
      putcksum(buf+2,2+2+payload_len);
      Serial.write(buf,payload_len+8);
    }
  }
  delay(5000);
}
