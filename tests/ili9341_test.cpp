#include <MadDisplay/MadDisplay.hpp>
#include <MadILI9341/MadILI9341.hpp>
#include <cassert>
#include <vector>
using namespace mad;
static void assertFrame(const ILI9341& display) {
  const auto& trace = Adafruit_ILI9341::trace();
  for (int y = 0; y < display.height(); ++y) for (int x = 0; x < display.width(); ++x)
    assert(trace.pixels[y*display.width()+x] == rgb565(display.pixel(x,y)));
  assert(trace.starts == trace.ends && !trace.transaction);
}
int main() {
  ILI9341 adapter(5,2,4); Display<ILI9341> display(adapter);
  adapter.present(); assert(adapter.lastTransferPixels() == 0);
  assert(!display.begin(4)); assert(display.begin(0,24000000));
  assert(adapter.differentialUpdates() && display.size().width == 240);
  assert(Adafruit_ILI9341::trace().frequency == 24000000);
  adapter.clear(Color::Blue); adapter.present(); assert(adapter.lastTransferPixels() == 76800); assertFrame(adapter);
  const auto starts = Adafruit_ILI9341::trace().starts;
  adapter.present(); assert(adapter.lastTransferPixels() == 0 && Adafruit_ILI9341::trace().starts == starts);
  adapter.pixel(11,7,Color::Red); adapter.present(); assert(adapter.lastTransferPixels() == 1 && adapter.lastTransferWindows() == 1); assertFrame(adapter);
  adapter.pixel(11,7,Color::Blue); adapter.present(); assert(adapter.lastTransferPixels() == 1); assertFrame(adapter); // erasure is transmitted too
  adapter.fillRect({20,30,10,5},Color::Green); adapter.present(); assert(adapter.lastTransferPixels() == 50 && adapter.lastTransferWindows() == 5); assertFrame(adapter);
  adapter.clear(Color::Yellow); adapter.present(); assert(adapter.lastTransferPixels() == 76800 && adapter.lastTransferWindows() == 1); assertFrame(adapter);
  for (uint8_t rotation = 0; rotation < 4; ++rotation) {
    assert(adapter.setRotation(rotation)); assert(display.size().width == (rotation%2 ? 320 : 240));
    adapter.pixel(adapter.width()-1,adapter.height()-1,Color::Magenta); adapter.present(); assertFrame(adapter);
  }
  adapter.invalidate(); adapter.present(); assert(adapter.lastTransferPixels() == 76800);
  assert(!adapter.setRotation(4));
  {
    std::vector<uint16_t> frame(76800),previous(76800);
    ILI9341 external(5,2);
    assert(!external.setFrameBuffers(frame.data(),100));
    assert(!external.setFrameBuffers(frame.data(),76800,frame.data(),76800));
    assert(external.setFrameBuffers(frame.data(),76800,previous.data(),76800));
    assert(external.begin(1)); external.clear(Color::Cyan); external.present(); assertFrame(external);
    assert(!external.setFrameBuffers(frame.data(),76800));
    external.present(); assert(external.lastTransferPixels() == 0);
  }
  {
    std::vector<uint16_t> frame(76800); ILI9341 lowMemory(5,2);
    assert(lowMemory.setFrameBuffers(frame.data(),frame.size())); assert(lowMemory.begin(0,0,false));
    assert(!lowMemory.differentialUpdates()); lowMemory.present(); lowMemory.present(); assert(lowMemory.lastTransferPixels() == 76800);
  }
}
