#ifndef icm20948_h
#define icm20948_h

#include "i2c_sensor.h"
#include "bits.h"

class ICM20948:public I2C_banked_sensor<I2C_sensor_be,0x7f,4,2> {
public:
  static const uint8_t addr_msb=0x68; //ORed with low bit of a0 to get actual address
public:
  using I2C_banked_sensor::read;
  using I2C_banked_sensor::write;
  static uint8_t const WHO_AM_I    =0x00;
  static uint8_t const USER_CTRL   =0x03;
  static uint8_t const LP_CONFIG   =0x05;
  static uint8_t const PWR_MGMT_1  =0x06;
  static uint8_t const INT_PIN_CFG =0x0F;
  static uint8_t const INT_ENABLE_1=0x11;
  static uint8_t const INT_STATUS_1=0x1A;
  static uint8_t const ACCEL_XOUT_H=0x2D;
  //bank 2
  static uint8_t const GYRO_SMPLRT_DIV=0x00;
  static uint8_t const GYRO_CONFIG_1=0x01;
  static uint8_t const ODR_ALIGN_EN=0x09;
  static uint8_t const ACCEL_SMPLRT_DIV_1=0x10;
  static uint8_t const ACCEL_CONFIG=0x14;
  ICM20948(int LA0=1,TwoWire& Lport=Wire):I2C_banked_sensor(addr_msb+LA0,Lport) {};
  uint8_t whoami() {
    return read<uint8_t>(0,WHO_AM_I);
  }
  void begin() {
    set_bank(0);
    uint8_t pwr_mgmt1=read<uint8_t>(0,PWR_MGMT_1);
    Serial.print("pwr_mgmt1 before reset: ");
    Serial.println(pwr_mgmt1,HEX);
    pwr_mgmt1=put_bits(pwr_mgmt1,7,7,uint8_t(1)); //Do a device reset
    Serial.print("pwr_mgmt1 after reset: ");
    Serial.println(pwr_mgmt1,HEX);
    write<uint8_t>(0,PWR_MGMT_1,pwr_mgmt1);
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
    write<uint8_t>(0,LP_CONFIG,0); //Disable duty cycles on accelerometer, gyro, and I2C master
    pwr_mgmt1=read<uint8_t>(0,PWR_MGMT_1);
    pwr_mgmt1&=~(1<<6) &  //Clear the sleep bit. This enables power to the analog circuitry
               ~(1<<5) &  //Clear the LP_EN, shifting the digital circuitry out of low-power mode.
               ~(1<<3) ;  //Clear the TEMP_DIS bit, enabling the temperature sensor
    write<uint8_t>(0,PWR_MGMT_1,pwr_mgmt1);
    write<uint8_t>(0,INT_PIN_CFG,uint8_t((0<<7) | //Logic level for INT pin is low
                                         (0<<6) | //INT is push/pull
                                         (0<<5) | //INT pin is pulsed
                                         (1<<4) | //Any read clears interrupt
                                         (0<<3) | //FSYNC active high
                                         (0<<2) | //FSYNC pin does not interrupt
                                         (1<<1))); //Enable I2C bypass
    write<uint8_t>(2,ODR_ALIGN_EN,1);// Any write that changes ODR stuff re-aligns start time of accel/gyro
    write<uint8_t>(0,INT_ENABLE_1,1<<0); //Enable interrupt on data ready
    write<uint8_t>(2,GYRO_CONFIG_1,uint8_t((3<<3) |  //Digital Low-Pass filter config, 51.2 3dB BW, 73.3 NBW (nosie bandwidth)
                                           (0<<1) |  //Gyro full-scale select, +-250deg/s
                                           (1<<0))); //Enable DLPF
    write<uint8_t>(2,GYRO_SMPLRT_DIV,9); //Output data rate=1125/(1+divisor), 112.5Hz. We want ODR to be greater than 2x filter bandwidth, so as to capture all the data the filter provides.
    write<uint16_t>(2,ACCEL_SMPLRT_DIV_1,9); //Accel output data rate=1125/(1+divisor), 112.5Hz. 
    write<uint8_t>(2,ACCEL_CONFIG,uint8_t((3<<3) |  //Digital Low-Pass filter config, 50.4 3dB BW, 68.8 NBW (nosie bandwidth)
                                          (2<<1) |  //Accel full-scale select, +-8g
                                          (1<<0))); //Enable DLPF
    //write(0,PWR_MGMT_2,0); //Enable all axes of accel and gyro
  }
  bool query(uint8_t buf[]) {
    if((read<uint8_t>(0,INT_STATUS_1)& 0x1)==0) return false;
    read(0,ACCEL_XOUT_H,buf,7*2);
    return true;
  }
  bool query(uint8_t& st, int16_t& ax, int16_t& ay, int16_t& az, 
            int16_t& gx, int16_t& gy, int16_t& gz,
            int16_t& T) {
    uint8_t buf[14];
    if(!query(buf)) return false;
    ax=get<int16_t>(buf+ 0);
    ay=get<int16_t>(buf+ 2);
    az=get<int16_t>(buf+ 4);
    gx=get<int16_t>(buf+ 6);
    gy=get<int16_t>(buf+ 8);
    gz=get<int16_t>(buf+10);
    T =get<int16_t>(buf+12);
    return true;
  }
};



/* Even though the AK sensor is logically a distinct device on the I2C bus,
 *  it is physically in the same package as the ICM20948, so we will physically
 *  include the code for it here too.
 */
#include "ak09916.h"

#endif
