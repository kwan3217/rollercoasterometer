#ifndef icm20948_h
#define icm20948_h

#include "i2c_sensor.h"

class ICM20948:public I2C_banked_sensor<0x7f,4,2> {
public:
  static const uint8_t addr_msb=0x68; //ORed with low bit of a0 to get actual address
public:
  using I2C_banked_sensor::read;
  using I2C_banked_sensor::readi16be;
  using I2C_banked_sensor::readu16be;
  using I2C_banked_sensor::write;
  using I2C_banked_sensor::writei16be;
  using I2C_banked_sensor::writeu16be;
  static uint8_t const WHO_AM_I    =0x00;
  static uint8_t const USER_CTRL   =0x03;
  static uint8_t const LP_CONFIG   =0x05;
  static uint8_t const PWR_MGMT_1  =0x06;
  static uint8_t const INT_PIN_CFG =0x0F;
  static uint8_t const ACCEL_XOUT_H=0x2D;
  ICM20948(int LA0=1,TwoWire& Lport=Wire):I2C_banked_sensor(addr_msb+LA0,Lport) {};
  uint8_t whoami() {
    return read(0,WHO_AM_I);
  }
  void begin() {
    set_bank(0);
    uint8_t pwr_mgmt1=read(0,PWR_MGMT_1);
    Serial.print("pwr_mgmt1 before reset: ");
    Serial.println(pwr_mgmt1,HEX);
    pwr_mgmt1|= (1<<7); //Do a device reset
    Serial.print("pwr_mgmt1 after reset: ");
    Serial.println(pwr_mgmt1,HEX);
    write(0,PWR_MGMT_1,pwr_mgmt1);
    delay(50); //wait for part to reset
    /* It turns out that USER_CTRL is correct automatically after reset
    write(0,USER_CTRL,(0<<7) | //Disable DMP
                      (0<<6) | //Disable FIFO
                      (0<<5) | //Disable I2C master, magnetometer bus is then passed through
                      (0<<4) | //Enable I2C mode (1 would disable and enable SPI only)
                      (0<<3) | //Don't reset DMP
                      (0<<2) | //Don't reset SRAM
                      (0<<1) | //Don't reset I2C master
                      (0<<0)); //Reserved
     */
    write(0,LP_CONFIG,0); //Disable duty cycles on accelerometer, gyro, and I2C master
    pwr_mgmt1=read(0,PWR_MGMT_1);
    pwr_mgmt1&=~(1<<6) &  //Clear the sleep bit. This enables power to the analog circuitry
               ~(1<<5) &  //Clear the LP_EN, shifting the digital circuitry out of low-power mode.
               ~(1<<3) ;  //Clear the TEMP_DIS bit, enabling the temperature sensor
    write(0,PWR_MGMT_1,pwr_mgmt1);
    write(0,INT_PIN_CFG,(0<<7) | //Logic level for INT pin is low
                        (0<<6) | //INT is push/pull
                        (0<<5) | //INT pin is pulsed
                        (1<<4) | //Any read clears interrupt
                        (0<<3) | //FSYNC active high
                        (0<<2) | //FSYNC pin does not interrupt
                        (1<<1)); //Enable I2C bypass
                        
    //write(0,PWR_MGMT_2,0); //Enable all axes of accel and gyro
  }
  void readSens(uint8_t buf[]) {
    read(0,ACCEL_XOUT_H,buf,7*2);
  }
  void read(uint32_t& t,
            int16_t& ax, int16_t& ay, int16_t& az, 
            int16_t& gx, int16_t& gy, int16_t& gz,
            int16_t& T, uint8_t& st) {
    uint8_t buf[14];
    t=micros();
    readSens(buf);
    ax=geti16be(buf+ 0);
    ay=geti16be(buf+ 2);
    az=geti16be(buf+ 4);
    gx=geti16be(buf+ 6);
    gy=geti16be(buf+ 8);
    gz=geti16be(buf+10);
    T =geti16be(buf+12);
    st=0xff;
  }
};


/* Even though the AK sensor is logically a distinct device on the I2C bus,
 *  it is physically in the same package as the ICM20948, so we will physically
 *  include the code for it here too.
 */

class AK09916:public I2C_sensor {
public:
  static const uint8_t addr=0x0c; 
  uint8_t mode;
public:
  using I2C_sensor::read;
  using I2C_sensor::readi16le;
  using I2C_sensor::readu16le;
  using I2C_sensor::write;
  using I2C_sensor::writei16le;
  using I2C_sensor::writeu16le;
  static const uint8_t WIA1=0x00;
  static const uint8_t WIA2=0x01;
  static const uint8_t ST1=0x10;
  static const uint8_t CNTL2=0x31;
  static const uint8_t CNTL3=0x32;
  AK09916(TwoWire& Lport=Wire):I2C_sensor(addr,Lport) {};
  uint16_t whoami() {
    return readi16le(WIA1);
  }
  void begin(uint8_t Lmode=0) {
    mode=Lmode;
    write(CNTL3,1);
    delay(100);
    write(CNTL2,mode==0?0x01:mode<<1);
    delay(10);
  }
  bool readSens(uint8_t buf[]) {
    read(ST1,buf,1);
    bool ready=bool(buf[0] & 0x01);
    if(!ready) {
      return false;
    }
    if(mode==0) {
      write(CNTL2,0x01); //Set up the next measurement, takes about 8ms
    } //Otherwise, in some continuous mode, so don't need to re-shoot
    read(ST1+1,buf+1,8);
    return true;
  }
  bool read(uint32_t& t, uint8_t& st1, int16_t& bx, int16_t& by, int16_t& bz, uint8_t& st2) {
    uint8_t buf[9];
    if(!readSens(buf)) return false;
    t=micros();
    bx=geti16le(buf+1);
    by=geti16le(buf+3);
    bz=geti16le(buf+5);
    st1=buf[0];
    st2=buf[8];
    return true;
  }
};



#endif
