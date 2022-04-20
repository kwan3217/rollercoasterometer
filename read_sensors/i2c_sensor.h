#ifndef i2c_sensor_h
#define i2c_sensor_h

#include <Wire.h>

inline uint8_t get_bits(uint8_t data, uint8_t bit1, uint8_t bit0) {
  uint8_t mask=(1<<(bit1-bit0+1))-1;
  return (data>>bit0) & mask;
}

inline void put_bits(uint8_t& data, uint8_t bit1, uint8_t bit0, uint8_t newbits) {
  uint8_t mask=(1<<(bit1-bit0+1))-1;
  data=data & ~(mask<<bit0);
  data=data |  ((newbits & mask)<<bit0);
}

inline uint16_t getu16be(uint8_t buf[]) {
  uint16_t msb, lsb;
  msb = buf[0];
  lsb = buf[1];
  return msb<<8 | lsb;
}

inline int16_t geti16be(uint8_t buf[]) {
  return (int16_t)getu16be(buf);
}

inline void putu16be(uint8_t buf[], uint16_t data) {
  buf[0]=(uint8_t)((data>>8) & 0xff);
  buf[1]=(uint8_t)((data>>0) & 0xff);
}

inline void puti16be(uint8_t buf[], int16_t data) {
  putu16be(buf,(uint16_t)data);
}

inline uint16_t getu16le(uint8_t buf[]) {
  uint16_t msb, lsb;
  lsb = buf[0];
  msb = buf[1];
  return msb<<8 | lsb;
}

inline int16_t geti16le(uint8_t buf[]) {
  return (int16_t)getu16le(buf);
}

inline void putu16le(uint8_t buf[], uint16_t data) {
  buf[0]=(uint8_t)((data>>0) & 0xff);
  buf[1]=(uint8_t)((data>>8) & 0xff);
}

inline void puti16le(uint8_t buf[], int16_t data) {
  putu16le(buf,(uint16_t)data);
}

inline void putcksum(uint8_t buf[], size_t len) {
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
  int last_bank=1<<(bank_bits+1); //Make sure we use a value that is out of range for the bank value
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

#endif
