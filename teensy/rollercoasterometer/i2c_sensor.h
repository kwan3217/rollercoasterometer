#include <Wire.h>

#ifndef i2c_sensor_h
#define i2c_sensor_h

template<class T>
class I2C_sensor {
  /** Software interface for an I2C device with registers. A register
   *  is a kind of virtual address space, so each I2C bus has several
   *  slave devices, each with its own address. Slaves which are used
   *  through this class are further structured in that they have
   *  registers, each with its own address. To address a register,
   *  we write the register address *to* the device address. We can
   *  then either keep writing, or re-address the device in read mode
   *  and get back the value in the address.
   *  
   *  Most devices have a burst mode, where you can read or write more
   *  than one byte of data at a time. The device keeps a register pointer,
   *  and each time it reads or writes a byte, it updates the register
   *  pointer so that it points to the next register that makes sense. For
   *  instance, many sensors have all of their measurement values visible
   *  as a contiguous block of registers. To read the block, you would
   *  address the device, write the register address of the beginning of
   *  the block, re-address the device, then read enough bytes to cover
   *  the entire block. The device itself will update its internal register 
   *  pointer each time, so that the next byte comes from the next register,
   *  etc. Some devices are smart about updating the register pointer --
   *  for instance, if you reach the end of the block and keep reading,
   *  the pointer automatically skips back to the beginning of the block
   *  so you get a fresh sensor readout.
   *  
   *  This software interface supports this kind of register and burst
   *  mode. It provides the following functions, all using C++ name overloading
   *  and mangling to give all the functions of a group the same name:
   *  
   *    * read() -- read data from the device and translate to host format
   *    * write() -- write data to the device, translating from host to device format
   *    * get() -- extract data from a byte buffer into host format
   *    * put() -- inject data into a byte buffer, translating from host format
   *    
   *  The format referred to above handles things like endian-translation. This
   *  code should work on any host, whatever its own endian format is. The code
   *  explicitly handles device format, but uses the standard C++ operators
   *  like arithmetic and bitwise operators to compose bytes into the host
   *  format. 
   *  
   *  The code *does* assume that the host and device use the same signed-integer 
   *  convention, (EG twos complement, which almost all machines have used for 
   *  the last 50 years). It relies on casting on the host to properly cast device 
   *  data back and forth.
   *  
   *  This uses CRTP to be extended. If it is not extended, it can only handle
   *  byte-sized values and byte buffers.
   */
public:
  TwoWire& port;           ///< Interface to I2C port
  const uint8_t i2c_addr;  ///< I2C address of this sensor -- cannot be changed after construction
public:
  /** Construct the sensor. This isn't allowed to actually use the port,
   *  as the port may not have been initialized (IE it might need to have
   *  begin() called on it).
   *  @param Laddr [in] I2C address of the device
   *  @param Lport [in] TwoWire interface to I2C port used to talk to this device
   */
  I2C_sensor(uint8_t Laddr, TwoWire& Lport=Wire):port(Lport),i2c_addr(Laddr) {};

  /**  Read from the device in burst mode, starting at the given address
   *   @param reg_addr [in] initial register address on the device
   *   @param buf [out] buffer on the host to read the data into
   *   @param len [in] number of bytes to read. buf needs to be at least this long.
   */
  void read(uint8_t reg_addr, uint8_t buf[], size_t len) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    port.endTransmission(false); //repeated start
  
    port.requestFrom(i2c_addr, len); //stop after read
    for(size_t i=0;i<len;i++) buf[i]=port.read();
  }
  uint8_t read(uint8_t reg_addr) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    port.endTransmission(false); //repeated start
  
    port.requestFrom(i2c_addr, 1); //stop after read
    return port.read();
  }
  
  /**  Write to the device in burst mode, starting at the given address
   *   @param reg_addr [in] initial register address on the device
   *   @param buf [in] buffer on the host containing the data to write
   *   @param len [in] number of bytes to write. buf needs to be at least this long.
   */
  void write(uint8_t reg_addr, uint8_t buf[], size_t len) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    for(size_t i=0;i<len;i++) port.write(buf[i]);
    port.endTransmission(); //stop
  }
  void write(uint8_t reg_addr, uint8_t data) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    port.write(data);
    port.endTransmission(); //stop
  }

  /** Get a data value from a buffer. Specializations of this template
   *  will handle such things as endian transformation.
   * @tparam U the data type to get from the buffer
   * @param buf [in] buffer to get from
   */
  template<typename U>
  U get(uint8_t buf[]);
  
  /** Put a data value into a buffer. Specializations of this template
   *  will handle such things as endian transformation.
   * @tparam U the data type to put into the buffer
   * @param buf [out] buffer to put into
   * @param data [in] data to put into the buffer
   */
  template<typename U>
  void put(uint8_t buf[], U data);
  
  /** Read a value from the device
   * @reg_addr [in] register address to read from
   * 
   * This reads exactly enough data to fill this data item. For
   * multiple items, you should read() into a buffer, then get()
   * each data item out of it.
   */
  template<typename U>
  U read(uint8_t reg_addr) {
    uint8_t buf[sizeof(U)];
    read(reg_addr,buf,sizeof(U));
    return static_cast<T*>(this)->get<U>(buf);
  }
  
  /** Read a value from the device
   * @reg_addr [in] register address to read from
   * 
   * This reads exactly enough data to fill this data item. For
   * multiple items, you should read() into a buffer, then get()
   * each data item out of it.
   */
  template<typename U>
  void write(uint8_t reg_addr, U data) {
    uint8_t buf[sizeof(U)];
    static_cast<T*>(this)->put<U>(buf,data);
    write(reg_addr,buf,sizeof(U));
  }
  
  /** Specialization to read a single byte from the device
   * @reg_addr [in] register address to read from
   * 
   * This doesn't use the burst-mode read() function, so
   * it is less efficient (4 byte-times to read each byte)
   * and should only be used if you really know that
   * you can't do the read in burst mode.

  template <> uint8_t read<uint8_t>(uint8_t reg_addr) {
    port.beginTransmission(i2c_addr);
    port.write(reg_addr);
    port.endTransmission(false); //repeated start
  
    port.requestFrom(i2c_addr, len); //stop after read
    return port.read();
  }
   */
  
};

class I2C_sensor_be:public I2C_sensor<I2C_sensor_be> {
public:
  using I2C_sensor::read;
  using I2C_sensor::write;
  I2C_sensor_be(uint8_t Laddr,TwoWire& Lport=Wire):I2C_sensor(Laddr,Lport) {};
};

template<> template<>
inline uint8_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) {return buf[0];}

template<> template<>
inline void    I2C_sensor<I2C_sensor_be>::put(uint8_t buf[], uint8_t data) {buf[0]=data;}

template<> template<>
inline uint16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]) {
  uint16_t msb, lsb;
  msb = buf[0];
  lsb = buf[1];
  uint16_t result=msb<<8 | lsb;
  return result;
}

template<> template<>
inline void I2C_sensor<I2C_sensor_be>::put(uint8_t buf[], uint16_t data) {
  uint8_t msb=(uint8_t)((data>>8) & 0xff);
  uint8_t lsb=(uint8_t)((data>>0) & 0xff);
  buf[0]=msb;
  buf[1]=lsb;
}

template<> template<> inline int8_t  I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]              ) {return int8_t (get<uint8_t>(buf));}
template<> template<> inline void    I2C_sensor<I2C_sensor_be>::put(uint8_t buf[], int8_t  data) {               put(buf,uint8_t(data));}
template<> template<> inline int16_t I2C_sensor<I2C_sensor_be>::get(uint8_t buf[]              ) {return int16_t(get<uint16_t>(buf));}
template<> template<> inline void    I2C_sensor<I2C_sensor_be>::put(uint8_t buf[], int16_t data) {               put(buf,uint16_t(data));}

class I2C_sensor_le:public I2C_sensor<I2C_sensor_le> {
public:
  using I2C_sensor::read;
  using I2C_sensor::write;
  I2C_sensor_le(uint8_t Laddr,TwoWire& Lport=Wire):I2C_sensor(Laddr,Lport) {};
};

template<> template<>
inline uint8_t I2C_sensor<I2C_sensor_le>::get(uint8_t buf[]) {return buf[0];}

template<> template<>
inline void    I2C_sensor<I2C_sensor_le>::put(uint8_t buf[], uint8_t data) {buf[0]=data;}

template<> template<>
inline uint16_t I2C_sensor<I2C_sensor_le>::get(uint8_t buf[]) {
  uint16_t msb, lsb;
  msb = buf[1];
  lsb = buf[0];
  return msb<<8 | lsb;
}

template<> template<>
inline void I2C_sensor<I2C_sensor_le>::put(uint8_t buf[], uint16_t data) {
  uint8_t msb=(uint8_t)((data>>8) & 0xff);
  uint8_t lsb=(uint8_t)((data>>0) & 0xff);
  buf[1]=msb;
  buf[0]=lsb;
}

template<class T, int bank_reg, int bank_shift, int bank_bits, typename bank_reg_type=uint8_t>
class I2C_banked_sensor: public T {
public:
  using T::read;
  using T::write;
  I2C_banked_sensor(uint8_t Laddr,TwoWire& Lport=Wire):T(Laddr,Lport) {};
  bank_reg_type last_bank=0;
  // Set the current register bank. The current bank number is always
  // the same 2 bits in the same register in all banks. Note that we keep
  // track on the host of which bank we are currently in, and only write
  // if we are actually changing bank, so this can be very efficient.
  void set_bank(bank_reg_type bank) {
    if(bank==last_bank) return;
    last_bank=bank;
    write(bank_reg,bank_reg_type((last_bank & ((1<<bank_bits)-1)) << bank_shift));
  }
  uint8_t get_bank() {
    last_bank=(read(bank_reg)>>bank_shift) & ((1<<bank_bits)-1);
    return last_bank;
  }
  // Read from a specified bank
  void read (bank_reg_type bank, uint8_t reg_addr, uint8_t buf[], size_t len) {set_bank(bank);       read    (reg_addr,buf,len);}
  template<typename U>
  U    read (bank_reg_type bank, uint8_t reg_addr)                            {set_bank(bank);return read <U>(reg_addr        );}

  // Write to a specified bank
  void write(bank_reg_type bank, uint8_t reg_addr, uint8_t buf[], size_t len) {set_bank(bank);       write   (reg_addr,buf,len);}
  template<typename U>
  void write(bank_reg_type bank, uint8_t reg_addr, U data)                    {set_bank(bank);       write<U>(reg_addr,data   );}        
};

#endif
