#ifndef packet_h
#define packet_h

template<typename T> inline int typecode();
template<> inline int typecode<uint8_t >(){return  1;};
template<> inline int typecode<int16_t >(){return  2;};
template<> inline int typecode<int32_t >(){return  3;};
template<> inline int typecode<float   >(){return  4;};
template<> inline int typecode<double  >(){return  5;};
template<> inline int typecode<char*   >(){return  7;}; //Intended for English strings
template<> inline int typecode<uint8_t*>(){return 10;}; //Intended for binary blobs
template<> inline int typecode<uint16_t>(){return 12;};
template<> inline int typecode<uint32_t>(){return 13;};
template<> inline int typecode<uint64_t>(){return 14;};

//These only work for unsigned integer types
template<typename T> inline void poke_le(uint8_t buf[], size_t i, T data) {
  for(size_t j=0;j<sizeof(T);j++) {
    buf[i+j]=uint8_t(data>>j*8) & 0xFF;
  }
}
template<typename T> inline T    peek_le(uint8_t buf[], size_t i) {
  T result=0;
  for(size_t j=0;j<sizeof(T);j++) {
    result|=T(buf[i+j])<<(8*j);
  }
}

template<> inline void    poke_le(uint8_t buf[], size_t i, int16_t value) {poke_le(buf,i,uint16_t(value));}
template<> inline void    poke_le(uint8_t buf[], size_t i, int32_t value) {poke_le(buf,i,uint32_t(value));}
template<> inline void    poke_le(uint8_t buf[], size_t i, int64_t value) {poke_le(buf,i,uint64_t(value));}
template<> inline int16_t peek_le(uint8_t buf[], size_t i)                {return int16_t(peek_le<uint16_t>(buf,i));}
template<> inline int32_t peek_le(uint8_t buf[], size_t i)                {return int32_t(peek_le<uint32_t>(buf,i));}
template<> inline int64_t peek_le(uint8_t buf[], size_t i)                {return int64_t(peek_le<uint64_t>(buf,i));}



class ubloxPacket:public Print {
public:
  const static size_t N=65535+8;
  uint8_t buf[N];
  size_t ptr;
  Print& ouf;
  using Print::write;
  using Print::print;
  using Print::println;
  ubloxPacket(Print& Louf):ouf(Louf) {}
  void clear() {ptr=0;}
  size_t write(uint8_t c) {
    if(ptr<N) {
      buf[ptr]=c;
      ptr++;
      return 1;
    } else {
      return 0;
    }
  }
  template<typename T> size_t write_le(T data) {
    if(ptr+sizeof(T)<=N) {
      poke_le(buf,ptr,data);
      ptr+=sizeof(T);
      return sizeof(T);
    } else {
      return 0;
    }
  }
  void start(uint8_t cls, uint8_t id) {
    clear();
    write(uint8_t(0xb5));
    write(uint8_t(0x62));
    write(cls);
    write(id);
    ptr+=2; //skip spot for size
  }
  size_t finish() {
    uint16_t len=ptr-6; //Only count the payload
    poke_le(buf,4,len);
    uint8_t CK_A=0, CK_B=0;
    for(size_t i=2;i<ptr;i++) {
      CK_A+=buf[i];
      CK_B+=CK_A;
    }
    write(CK_A);
    write(CK_B);
    return drain();
  }
  size_t drain() {
    size_t result=ouf.write(buf,ptr);
    clear();
    return result;
  }
};



#endif
