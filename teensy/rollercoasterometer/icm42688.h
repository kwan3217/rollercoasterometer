#include "i2c_sensor.h"

class ICM42688:public I2C_banked_sensor<I2C_sensor_be,0x76,0,3> {
public:
  static const uint8_t addr_msb=0x68; //ORed with low bit of a0 to get actual address
public:
  using I2C_banked_sensor::read;
  using I2C_banked_sensor::write;
  static uint8_t const DEVICE_CONFIG=0x11;
  static uint8_t const WHO_AM_I     =0x75;
  ICM42688(int LA0=1,TwoWire& Lport=Wire):I2C_banked_sensor(addr_msb+LA0,Lport) {};
  uint8_t whoami() {
    return read<uint8_t>(0,WHO_AM_I);
  }
  void begin() {
    set_bank(0);
    uint8_t device_config=read<uint8_t>(0,DEVICE_CONFIG);
    Serial.print("DEVICE_CONFIG before reset: ");
    Serial.println(device_config,HEX);
    device_config|= (1<<0); //Do a device reset
    Serial.print("DEVICE_CONFIG after reset: ");
    Serial.println(device_config,HEX);
    write<uint8_t>(0,DEVICE_CONFIG,device_config);
    delay(50); //wait for part to reset
  }
  bool query(uint8_t* buf) {
//    read(0,ACCEL_XOUT_H,buf,7*2);
    return false;
  }
  void query(uint32_t& t,
            int16_t& ax, int16_t& ay, int16_t& az, 
            int16_t& gx, int16_t& gy, int16_t& gz,
            int16_t& T) {
    uint8_t buf[14];
    t=micros();
    query(buf);
    ax=get<int16_t>(buf+ 0);
    ay=get<int16_t>(buf+ 2);
    az=get<int16_t>(buf+ 4);
    gx=get<int16_t>(buf+ 6);
    gy=get<int16_t>(buf+ 8);
    gz=get<int16_t>(buf+10);
    T =get<int16_t>(buf+12);
  }
};
