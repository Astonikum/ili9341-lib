#pragma once
#include <stdint.h>
#include <cassert>
#include <cstring>
struct SPIClass {};
class Adafruit_ILI9341 {
 public:
  Adafruit_ILI9341(int16_t,int16_t,int16_t) {}
  Adafruit_ILI9341(SPIClass*,int16_t,int16_t,int16_t) {}
  struct Trace {
    uint16_t pixels[240*320]{};
    uint16_t width = 240,height = 320,x = 0,y = 0,w = 0,h = 0;
    uint32_t position = 0, sent = 0, starts = 0, ends = 0, windows = 0, frequency = 0;
    uint8_t rotation = 0;
    bool transaction = false;
  };
  static Trace& trace() { static Trace value; return value; }
  void begin(uint32_t frequency = 0) { trace().frequency = frequency; }
  void setRotation(uint8_t value) {
    trace().rotation = value; trace().width = value%2 ? 320 : 240; trace().height = value%2 ? 240 : 320;
  }
  void startWrite() { auto& t = trace(); assert(!t.transaction); t.transaction = true; ++t.starts; }
  void endWrite() { auto& t = trace(); assert(t.transaction); t.transaction = false; ++t.ends; }
  virtual void setAddrWindow(uint16_t x,uint16_t y,uint16_t w,uint16_t h) {
    auto& t = trace(); assert(t.transaction && w && h && uint32_t(x)+w <= t.width && uint32_t(y)+h <= t.height);
    t.x = x; t.y = y; t.w = w; t.h = h; t.position = 0; ++t.windows;
  }
  void writePixels(uint16_t* pixels,uint32_t length,bool block = true,bool bigEndian = false) {
    auto& t = trace(); assert(t.transaction && block && !bigEndian && t.position+length <= uint32_t(t.w)*t.h);
    for (uint32_t i = 0; i < length; ++i,++t.position)
      t.pixels[(t.y+t.position/t.w)*t.width+t.x+t.position%t.w] = pixels[i];
    t.sent += length;
  }
 protected:
  void writeCommand(uint8_t command) {
    command_ = command; word_ = 0;
    if (command == 0x2c) Adafruit_ILI9341::setAddrWindow(x1_,y1_,x2_-x1_+1,y2_-y1_+1);
  }
  void SPI_WRITE16(uint16_t value) {
    if (command_ == 0x2a) (word_++ ? x2_ : x1_) = value;
    else if (command_ == 0x2b) (word_++ ? y2_ : y1_) = value;
    else assert(false);
  }
 private:
  uint16_t x1_ = 0,x2_ = 0,y1_ = 0,y2_ = 0;
  uint8_t command_ = 0,word_ = 0;

};
