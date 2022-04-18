#include "i2c_sensor.h"

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
  static uint8_t const ACCEL_XOUT_H=0x2D;
  ICM20948(int LA0=1,TwoWire& Lport=Wire):I2C_banked_sensor(addr_msb+LA0,Lport) {};
  uint8_t whoami() {
    return read<uint8_t>(0,WHO_AM_I);
  }
  void begin() {
    set_bank(0);
    uint8_t pwr_mgmt1=read<uint8_t>(0,PWR_MGMT_1);
    Serial.print("pwr_mgmt1 before reset: ");
    Serial.println(pwr_mgmt1,HEX);
    pwr_mgmt1|= (1<<7); //Do a device reset
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
                        
    //write(0,PWR_MGMT_2,0); //Enable all axes of accel and gyro
  }
  void query(uint8_t buf[]) {
    read(0,ACCEL_XOUT_H,buf,7*2);
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
