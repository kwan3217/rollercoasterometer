
#include <Wire.h>

static const uint8_t ICM2_addr=0x68;
static const uint8_t ICM2_bankaddr=0x7f;
static const uint8_t ICM2_whoami=0x00;

static const uint8_t ICM4_addr=0x69;
static const uint8_t ICM4_bankaddr=0x76;
static const uint8_t ICM4_whoami=0x75;

void setup() {
  Serial.begin(230400);
  Wire.begin();
  Wire.setClock(400000);
  pinMode(19,INPUT);   //SCL0
  pinMode(18,INPUT);   //SDA0

}

void loop() {
  Serial.print("t: ");
  Serial.println(millis());
  Serial.print("ICM20948 who_am_i: (sb 0xEA) 0x");
  Wire.beginTransmission(ICM2_addr);
  Wire.write(ICM2_bankaddr);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(ICM2_addr);
  Wire.write(ICM2_whoami);
  Wire.endTransmission(false);
  Wire.requestFrom(ICM2_addr,1);
  Serial.println(Wire.read(),HEX);

  Serial.print('0');
  Wire.beginTransmission(ICM4_addr);
  Serial.print('1');
  Wire.write(ICM4_bankaddr);
  Serial.print('2');
  Wire.write(0x00);
  Serial.print('3');
  Wire.endTransmission();
  Serial.print('4');
  delay(10);
  Serial.print('5');
  Wire.beginTransmission(ICM4_addr);
  Serial.print('6');
  Wire.write(ICM4_whoami);
  Serial.print('7');
  Wire.endTransmission(false);
  Serial.print('8');
  Wire.requestFrom(ICM4_addr,1);
  Serial.print('9');
  uint8_t result=Wire.read();
  Serial.print("ICM42688 who_am_i: (sb 0x47) 0x");
  Serial.println(result,HEX);

  Serial.println(float(analogRead(A0))*5.0/1024);
  delay(1000);
}
