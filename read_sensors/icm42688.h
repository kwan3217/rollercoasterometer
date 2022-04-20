#ifndef icm42688_h
#define icm42688_h

#include "i2c_sensor.h"

class ICM42688:public I2C_banked_sensor<0x76,0,3> {
public:
  static const uint8_t addr_msb=0x68; //ORed with low bit of a0 to get actual address
public:
  using I2C_banked_sensor::read;
  using I2C_banked_sensor::readi16be;
  using I2C_banked_sensor::readu16be;
  using I2C_banked_sensor::write;
  using I2C_banked_sensor::writei16be;
  using I2C_banked_sensor::writeu16be;
  static uint8_t const DEVICE_CONFIG = 0x11;
  static uint8_t const TEMP_DATA1    = 0x1D;
  static uint8_t const INTF_CONFIG1  = 0x4D;
  static uint8_t const PWR_MGMT0     = 0x4E;
  static uint8_t const GYRO_CONFIG0  = 0x4F;
  static uint8_t const ACCEL_CONFIG0 = 0x50;
  static uint8_t const WHO_AM_I      = 0x75;
  ICM42688(int LA0=1,TwoWire& Lport=Wire):I2C_banked_sensor(addr_msb+LA0,Lport) {};
  uint8_t whoami() {
    return read(0,WHO_AM_I);
  }
  /** Initialize the ICM42688
   *  @param accel_fs_sel [in] coded accelerometer sensitivity. Actual is +-2**(3-code) g, so
   *                         default is 16g
   *  @param accel_odr [in] consult table 14.38 in datasheet, default is 1kHz
   */
  void begin(uint8_t accel_fs_sel=0b000, uint8_t accel_odr=0b0110,
             uint8_t  gyro_fs_sel=0b000, uint8_t  gyro_odr=0b0110
  ) {
    Serial.println("set_bank(0)");
    set_bank(0);
    uint8_t rmw=0;
    put_bits(rmw,4,4,0); // Don't care about SPI mode, will go to default after reset anyway
    put_bits(rmw,0,0,1); // Command the device to reset
    Serial.print("write(0,0x");
    Serial.print(DEVICE_CONFIG,HEX);
    Serial.print("=DEVICE_CONFIG,0b");
    Serial.print(rmw,BIN);
    Serial.println(")");
    write(0,DEVICE_CONFIG,rmw); 
    delay(50); // Datasheet says wait 1ms for soft reset to be effective

    rmw=read(0,INTF_CONFIG1);
    put_bits(rmw,1,0,0b01); //Use PLL for time source
    Serial.print("write(0,0x");
    Serial.print(INTF_CONFIG1,HEX);
    Serial.print("=INTF_CONFIG1,0b");
    Serial.print(rmw,BIN);
    Serial.println(")");
    write(0,INTF_CONFIG1,rmw);

    rmw=read(0,PWR_MGMT0);
    put_bits(rmw,5,5,0); //Enable temperature (negative logic)
    put_bits(rmw,4,4,0); //don't power RC timing circuit if accel and gyro are both idle
    put_bits(rmw,3,2,0b11); //Turn gyro on in low noise mode (as opposed to low power mode)
    put_bits(rmw,1,0,0b11); //Same for acc
    write(0,PWR_MGMT0,rmw);
    delay(50); //Datasheet says gyro needs to stay on for 45ms, 
               //and no other register writes for 0.2ms

    rmw=read(0,ACCEL_CONFIG0);
    put_bits(rmw,7,5,accel_fs_sel);
    put_bits(rmw,3,0,accel_odr);
    write(0,ACCEL_CONFIG0,rmw);

    rmw=read(0,GYRO_CONFIG0);
    put_bits(rmw,7,5,gyro_fs_sel);
    put_bits(rmw,3,0,gyro_odr);
    write(0,GYRO_CONFIG0,rmw);
  }
  void readSens(uint8_t buf[]) {
    read(0,TEMP_DATA1,buf,8*2+1);
  }
  void read(uint32_t& t,
            int16_t& ax, int16_t& ay, int16_t& az, 
            int16_t& gx, int16_t& gy, int16_t& gz,
            int16_t& T, uint8_t& st) {
    uint8_t buf[17];
    t=micros();
    readSens(buf);
    T =geti16be(buf+ 0);
    ax=geti16be(buf+ 2);
    ay=geti16be(buf+ 4);
    az=geti16be(buf+ 6);
    gx=geti16be(buf+ 8);
    gy=geti16be(buf+10);
    gz=geti16be(buf+12);
    //uint8_t t_fsync=geti16be(buf+14);
    st=buf[16];
  }
};



#endif
