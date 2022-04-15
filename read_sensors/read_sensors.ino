#include <Wire.h>

uint16_t getu16be(uint8_t buf[]) {
  uint16_t msb, lsb;
  msb = buf[0];
  lsb = buf[1];
  return msb<<8 | lsb;
}

int16_t geti16be(uint8_t buf[]) {
  return (int16_t)getu16be(buf);
}

void putu16be(uint8_t buf[], uint16_t data) {
  buf[0]=(uint8_t)((data>>8) & 0xff);
  buf[1]=(uint8_t)((data>>0) & 0xff);
}

void puti16be(uint8_t buf[], int16_t data) {
  putu16be(buf,(uint16_t)data);
}

uint16_t getu16le(uint8_t buf[]) {
  uint16_t msb, lsb;
  lsb = buf[0];
  msb = buf[1];
  return msb<<8 | lsb;
}

int16_t geti16le(uint8_t buf[]) {
  return (int16_t)getu16le(buf);
}

void putu16le(uint8_t buf[], uint16_t data) {
  buf[0]=(uint8_t)((data>>0) & 0xff);
  buf[1]=(uint8_t)((data>>8) & 0xff);
}

void puti16le(uint8_t buf[], int16_t data) {
  putu16le(buf,(uint16_t)data);
}

void putcksum(uint8_t buf[], size_t len) {
  uint8_t ckA,ckB;
  for(size_t i=0;i<len;i++) {
    ckA+=buf[i];
    ckB+=ckA;
  }
  buf[len]=ckA;
  buf[len+1]=ckB;
}

class I2C_sensor {
public:
  TwoWire& port;
  const uint8_t i2c_addr;  // I2C address of AD799x
public:
  I2C_sensor(uint8_t Laddr, TwoWire& Lport=Wire):port(Lport),i2c_addr(Laddr) {};
  // Lowest level I/O routines -- write or read a byte buffer in the current bank
  void read(uint8_t reg_addr, uint8_t buf[], size_t len) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    port.endTransmission(false); //repeated start
  
    port.requestFrom(i2c_addr, len); //stop after read
    for(size_t i=0;i<len;i++) buf[i]=port.read();
  }
  void write(uint8_t reg_addr, uint8_t buf[], size_t len) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    for(size_t i=0;i<len;i++) port.write(buf[i]);
    port.endTransmission(); //stop
  }
  // Byte I/O
  uint8_t read(uint8_t reg_addr) {
    uint8_t result;
    read(reg_addr,&result,1);
    return result;
  }
  void write(uint8_t reg_addr, uint8_t data) {
    write(reg_addr,&data,1);
  }
  // Big-endian 16-bit I/O
  uint16_t readu16be(uint8_t reg_addr) {
    uint8_t buf[2];
    read(reg_addr,buf,2);
    return getu16be(buf);
  }
  int16_t readi16be(uint8_t reg_addr) {
    uint8_t buf[2];
    read(reg_addr,buf,2);
    return geti16be(buf);
  }
  void writeu16be(uint8_t reg_addr, uint16_t data) {
    uint8_t buf[2];
    putu16be(buf,data);
    write(reg_addr,buf,2);
  }
  void writei16be(uint8_t reg_addr, int16_t data) {
    uint8_t buf[2];
    puti16be(buf,data);
    write(reg_addr,buf,2);
  }
  // Little-endian 16-bit I/O
  uint16_t readu16le(uint8_t reg_addr) {
    uint8_t buf[2];
    read(reg_addr,buf,2);
    return getu16le(buf);
  }
  int16_t readi16le(uint8_t reg_addr) {
    uint8_t buf[2];
    read(reg_addr,buf,2);
    return geti16le(buf);
  }
  void writeu16le(uint8_t reg_addr, uint16_t data) {
    uint8_t buf[2];
    putu16le(buf,data);
    write(reg_addr,buf,2);
  }
  void writei16le(uint8_t reg_addr, int16_t data) {
    uint8_t buf[2];
    puti16le(buf,data);
    write(reg_addr,buf,2);
  }
};

template<int bank_reg, int bank_shift, int bank_bits>
class I2C_banked_sensor: public I2C_sensor {
public:
  using I2C_sensor::read;
  using I2C_sensor::readi16be;
  using I2C_sensor::readu16be;
  using I2C_sensor::readi16le;
  using I2C_sensor::readu16le;
  using I2C_sensor::write;
  using I2C_sensor::writei16be;
  using I2C_sensor::writeu16be;
  using I2C_sensor::writei16le;
  using I2C_sensor::writeu16le;
  I2C_banked_sensor(uint8_t Laddr,TwoWire& Lport=Wire):I2C_sensor(Laddr,Lport) {};
  int last_bank=0;
  // Set the current register bank. The current bank number is always
  // the same 2 bits in the same register in all banks. Note that we keep
  // track on the host of which bank we are currently in, and only write
  // if we are actually changing bank, so this can be very efficient.
  void set_bank(uint8_t bank) {
    if(bank==last_bank) return;
    last_bank=bank;
    write(bank_reg,(last_bank & ((1<<bank_bits)-1)) << bank_shift);
  }
  uint8_t get_bank() {
    last_bank=(read(bank_reg)>>bank_shift) & ((1<<bank_bits)-1);
    return last_bank;
  }
  // Read from a specified bank
  void     read      (uint8_t bank, uint8_t reg_addr, uint8_t  buf[], size_t len) {set_bank(bank);       read      (reg_addr,buf,len);}
  uint8_t  read      (uint8_t bank, uint8_t reg_addr)                             {set_bank(bank);return read      (reg_addr);        }
  uint16_t readu16be (uint8_t bank, uint8_t reg_addr)                             {set_bank(bank);return readu16be (reg_addr);        }
   int16_t readi16be (uint8_t bank, uint8_t reg_addr)                             {set_bank(bank);return readi16be (reg_addr);        }
  uint16_t readu16le (uint8_t bank, uint8_t reg_addr)                             {set_bank(bank);return readu16le (reg_addr);        }
   int16_t readi16le (uint8_t bank, uint8_t reg_addr)                             {set_bank(bank);return readi16le (reg_addr);        }
  // Write to a specified bank
  void     write     (uint8_t bank, uint8_t reg_addr, uint8_t  buf[], size_t len) {set_bank(bank);       write     (reg_addr,buf,len);}
  void     write     (uint8_t bank, uint8_t reg_addr, uint8_t  data)              {set_bank(bank);       write     (reg_addr,data);   }
  void     writeu16be(uint8_t bank, uint8_t reg_addr, uint16_t data)              {set_bank(bank);       writeu16be(reg_addr,data);   }
  void     writei16be(uint8_t bank, uint8_t reg_addr,  int16_t data)              {set_bank(bank);       writei16be(reg_addr,data);   }
  void     writeu16le(uint8_t bank, uint8_t reg_addr, uint16_t data)              {set_bank(bank);       writeu16le(reg_addr,data);   }
  void     writei16le(uint8_t bank, uint8_t reg_addr,  int16_t data)              {set_bank(bank);       writei16le(reg_addr,data);   }
};


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
            int16_t& T) {
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
  }
};

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

ICM20948 imu;
AK09916 mag;
BME680 bme;
uint8_t buf[2+2+2+4+2*6+2+2];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  Wire.begin();
  Wire.setClock(400000);
  imu.begin();
  mag.begin();
  bme.begin(0,0,0,0);
  Serial.print("IMU address:  0x");
  Serial.println(imu.i2c_addr,HEX);
  Serial.print("IMU page reg: 0x");
  Serial.println(imu.read(0,0x7f),HEX);
  Serial.print("IMU whoami (should be 0xEA):   0x");
  Serial.println(imu.read(0,(uint8_t)0x00),HEX);
  buf[0]=0xb5;buf[1]=0x62;buf[2]='k';buf[3]=1;buf[4]=sizeof(buf);buf[5]=0;
  Serial.print("Mag whoami (should be 0x948):  0x");
  Serial.println(mag.whoami(),HEX);
  Serial.print("BME whoami (should be 0x61) :  0x");
  Serial.println(bme.whoami(),HEX);
  uint8_t bbuf[256];
  bme.read(0,bbuf,sizeof(bbuf));
  for(int i=0;i<256;i+=16) {
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
  uint32_t ta,tb,tp;
  uint8_t bst1,bst2,pst;
  int16_t ax,ay,az,bx,by,bz,gx,gy,gz,T;
  uint32_t rawT,rawP;
  uint16_t rawH;
  imu.read(ta,ax,ay,az,gx,gy,gz,T);
  bool bready=mag.read(tb,bst1,bx,by,bz,bst2);
  bool pready=bme.read(tp,pst,rawT,rawP,rawH);
  if(true) {
    Serial.print("ta: ");
    Serial.print(ta);
    Serial.print("\u03bcs   ax: ");
    Serial.print(ax);
    Serial.print(" DN  ay: ");
    Serial.print(ay);
    Serial.print(" DN  az: ");
    Serial.print(az);
    Serial.print(" DN  gx: ");
    Serial.print(gx);
    Serial.print(" DN  gy: ");
    Serial.print(gy);
    Serial.print(" DN  gz: ");
    Serial.print(gz);
    Serial.print(" DN  T: ");
    Serial.print(T);
    Serial.println(" DN");
    if(bready) {
      Serial.print("tb: ");
      Serial.print(tb);
      Serial.print("\u03bcs  bst1: 0x");
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
      Serial.print("\u03bcs  pst: 0x");
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
    memcpy(  buf+2+2+2     ,&ta,sizeof(ta));
    puti16le(buf+2+2+2+4+ 0,ax);
    puti16le(buf+2+2+2+4+ 2,ay);
    puti16le(buf+2+2+2+4+ 4,az);
    puti16le(buf+2+2+2+4+ 6,gx);
    puti16le(buf+2+2+2+4+ 8,gy);
    puti16le(buf+2+2+2+4+10,gz);
    puti16le(buf+2+2+2+4+12,T);
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
  delay(1000);
}
