#ifndef bme680_h
#define bme680_h

#include "i2c_sensor.h"

class BME680:public I2C_sensor {
public:
  static const uint8_t addr=0x77; 
  uint8_t osrs_t;
  uint8_t osrs_p;
  uint8_t osrs_h;
  uint8_t osrs_g;
  uint32_t t; //time of kickoff of measurement
public:
  using I2C_sensor::read;
  using I2C_sensor::readi16le;
  using I2C_sensor::readu16le;
  using I2C_sensor::write;
  using I2C_sensor::writei16le;
  using I2C_sensor::writeu16le;
  static const uint8_t chip_id=0xD0;
  static const uint8_t reset  =0xE0;
  static const uint8_t ctrl_hum=0x72;
  static const uint8_t ctrl_meas=0x74;
  static const uint8_t config=0x75;
  static const uint8_t ctrl_gas_1=0x71;
  static const uint8_t meas_status_0=0x1d;
  static const uint8_t pres_msb=0x1f;
  BME680(TwoWire& Lport=Wire):I2C_sensor(addr,Lport) {};
  uint8_t whoami() {
    return read(0xD0);
  }
  void begin(uint8_t Losrs_t=5, ///< 0 for disable, or set oversampling to 2**(osrs_t-1). Default is highest oversampling.
             uint8_t Losrs_p=5, ///< 0 for disable, or set oversampling to 2**(osrs_p-1). Default is highest oversampling.
             uint8_t Losrs_h=0, ///< 0 for disable, or set oversampling to 2**(osrs_h-1). Default is disabled.
             uint8_t Losrs_g=0) {
    osrs_t=Losrs_t;
    osrs_p=Losrs_p;
    osrs_h=Losrs_h;
    osrs_g=Losrs_g;
    
    // Reset the device
    write(reset,0xB6);
    delay(10);

    // Set oversampling for T, P, H. T and P in same register, H in a different register
    write(ctrl_hum,(0 & 0x01)      << 6 | //Disable spi 3wire interrupt
                   (osrs_h & 0x07) << 0); 

    write(ctrl_meas,(osrs_t & 0x07)<<5 | 
                    (osrs_p & 0x07)<<2 |
                    (0 & 0x03) <<0);
                    
    // Disable IIR for temperature
    write(config,(0 & 0x07) << 2 |  //Disable IIR by setting coefficient to 0
                 (0 & 0x01) << 0 ); //Disable SPI 3wire interface

    // Disable gas measurement
    write(ctrl_gas_1,(0 & 0x01) << 4); //Disable run_gas measurement

    //Do a priming read -- this will eventually set the data-ready bit.
    kickoff();
    
  }
  void kickoff() {
    t=micros();
    //Since we know all the bits, just write them rather than r/m/w.
    write(ctrl_meas,(osrs_t & 0x07)<<5 | 
                    (osrs_p & 0x07)<<2 |
                    (1 & 0x03) <<0); // Force a measurement to start
  }
  bool readSens(uint8_t buf[]) {
    read(meas_status_0,buf,1);
    bool ready=bool((buf[0]>>7) & 0x01);
    if(!ready) {
      return false;
    }
    read(pres_msb,buf+1,8);
    return true;
  }
  bool read(uint32_t& Lt, uint8_t& st, uint32_t& T, uint32_t& P, uint16_t& h, bool rekick=true) {
    uint8_t buf[9];
    if(!readSens(buf)) return false;
    Lt=t;
    st=buf[0];
    P=(((uint32_t)getu16be(buf+1))<<4 | buf[3]) & (0xF0>>4);
    T=(((uint32_t)getu16be(buf+4))<<4 | buf[6]) & (0xF0>>4);
    h=getu16be(buf+7);
    if(rekick) kickoff();
    return true;
  }
};


#endif
