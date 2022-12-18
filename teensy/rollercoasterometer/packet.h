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

#include "bits.h"

template<int N=65535+8>
class ubloxPacket:public Print {
public:
  uint8_t buf[N];
  size_t ptr;
  Print* ouf;
  using Print::write;
  using Print::print;
  using Print::println;
  ubloxPacket() {}
  void begin(Print* Louf) {ouf=Louf;}
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
  template<typename T> size_t write_le(T data, const char* name=nullptr, const char* desc=nullptr) {
    if(ptr+sizeof(T)<=N) {
      put_field_le(buf,ptr,data);
      ptr+=sizeof(T);
      return sizeof(T);
    } else {
      return 0;
    }
  }
  size_t print(const char* data, const char* name=nullptr, const char* desc=nullptr) {
    return Print::print(data);
  }
  void start(uint8_t cls, uint8_t id, const char* name=nullptr, const char* desc=nullptr) {
    clear();
    write(uint8_t(0xb5));
    write(uint8_t(0x62));
    write(cls);
    write(id);
    ptr+=2; //skip spot for size
  }
  size_t finish() {
    uint16_t len=ptr-6; //Only count the payload
    put_field_le(buf,4,len);
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
    size_t result=ouf->write(buf,ptr);
    clear();
    return result;
  }
};



#endif
