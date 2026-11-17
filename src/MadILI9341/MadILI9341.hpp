#pragma once
#include <Adafruit_ILI9341.h>
#include <MadDisplay/presentation.hpp>
#include <stdlib.h>
#include <string.h>

namespace mad {
// Always send a complete window: the vendor's static cache is shared between devices.
class ILI9341Transport : public Adafruit_ILI9341 {
 public:
  using Adafruit_ILI9341::Adafruit_ILI9341;
  void setAddrWindow(uint16_t x,uint16_t y,uint16_t w,uint16_t h) override {
    writeCommand(0x2a); SPI_WRITE16(x); SPI_WRITE16(x+w-1);
    writeCommand(0x2b); SPI_WRITE16(y); SPI_WRITE16(y+h-1);
    writeCommand(0x2c);
  }
};
// Full RGB565 frame. An optional previous frame enables exact differential SPI updates.
class ILI9341 final : public RGB565Surface {
 public:
  explicit ILI9341(int16_t cs,int16_t dc,int16_t reset = -1) : driver_(cs,dc,reset) {}
#if !defined(ESP8266)
  ILI9341(SPIClass& spi,int16_t dc,int16_t cs = -1,int16_t reset = -1) : driver_(&spi,dc,cs,reset) {}
#endif
  ~ILI9341() override { if (ownFrame_) free(frame_); if (ownPrevious_) free(previous_); }
  ILI9341(const ILI9341&) = delete;
  ILI9341& operator=(const ILI9341&) = delete;
  // External RAM/PSRAM can be attached before begin(). Storage must outlive the adapter.
  bool setFrameBuffers(uint16_t* frame,size_t capacity,uint16_t* previous = nullptr,size_t previousCapacity = 0) {
    if (ready_ || frame_ || !frame || capacity < kPixels || (previous && previousCapacity < kPixels)) return false;
    const uintptr_t a = reinterpret_cast<uintptr_t>(frame), b = reinterpret_cast<uintptr_t>(previous);
    if (previous && (a > b ? a-b : b-a) < kPixels*sizeof(uint16_t)) return false;
    frame_ = frame; previous_ = previous; external_ = true; return true;
  }
  bool begin(uint8_t rotation = 0,uint32_t spiFrequency = 0,bool differential = true) {
    if (rotation > 3 || SIZE_MAX < kPixels*sizeof(uint16_t)) return false;
    if (!frame_) { frame_ = static_cast<uint16_t*>(malloc(kPixels*sizeof(uint16_t))); ownFrame_ = frame_ != nullptr; }
    if (!frame_) return false;
    if (differential && !previous_ && !external_) {
      previous_ = static_cast<uint16_t*>(malloc(kPixels*sizeof(uint16_t))); ownPrevious_ = previous_ != nullptr;
    }
    differential_ = differential && previous_;
    driver_.begin(spiFrequency); ready_ = true;
    return setRotation(rotation);
  }
  bool setRotation(uint8_t rotation) {
    if (rotation > 3) return false;
    dimensions_ = rotation%2 ? Size{320,240} : Size{240,320};
    if (ready_) {
      driver_.setRotation(rotation); setStorage(dimensions_,frame_,kPixels); clear();
    }
    presenter_.invalidate(); return true;
  }
  Size size() const override { return dimensions_; }
  int16_t width() const override { return dimensions_.width; }
  int16_t height() const override { return dimensions_.height; }
  bool differentialUpdates() const { return differential_; }
  uint32_t lastTransferPixels() const { return transferred_; }
  uint16_t lastTransferWindows() const { return windows_; }
  void invalidate() { presenter_.invalidate(); }
  Adafruit_ILI9341& driver() { return driver_; }
  void present() override {
    transferred_ = 0; windows_ = 0;
    if (!ready_) return;
    presenter_.present(*this,differential_ ? previous_ : nullptr,driver_);
    windows_ = presenter_.stats().windows;
    transferred_ = (presenter_.stats().bytes-windows_*11)/2;
  }

 private:
  static constexpr uint32_t kPixels = 240UL*320;
  ILI9341Transport driver_;
  Size dimensions_{240,320};
  uint16_t* frame_ = nullptr;
  uint16_t* previous_ = nullptr;
  RGB565Presenter presenter_;
  uint32_t transferred_ = 0;
  uint16_t windows_ = 0;
  bool ownFrame_ = false, ownPrevious_ = false, external_ = false;
  bool ready_ = false, differential_ = false;
};
}  // namespace mad
