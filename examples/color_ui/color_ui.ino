#include <MadDisplay.h>
#include <MadILI9341.h>

// Choose pins to match your SPI wiring. This example targets an ESP32.
mad::ILI9341 adapter(5,2,4);
mad::Display<mad::ILI9341> display(adapter);
mad::VerticalLayout root;
mad::Page page(root,mad::rgb(8,12,16));
mad::Label title("MAD DISPLAY RGB",mad::LabelStyle(mad::Color::Cyan,2));
mad::Button button("COLOR + MOTION",mad::ButtonStyle(mad::Color::Yellow,mad::Color::Black,2,2));
void paint(mad::Canvas& canvas,mad::Size size,uint32_t time,void*) {
  for (int16_t y = 0; y < size.height; ++y) {
    const uint8_t phase = static_cast<uint8_t>(time/16+y*3);
    canvas.fillSpan(0,y,size.width,mad::rgb(phase,255-phase,160));
  }
}
mad::CanvasComponent gradient({300,100},paint);
mad::Motion entrance(gradient,mad::Entrance::Reveal,700);
void setup() {
  if (!display.begin(1)) while (true) {}
  display.setFps(60); root.setGap(8); root.add(title); root.add(button); root.add(entrance);
  display.openPage(display.registerPage(page));
}
void loop() { display.render(micros()); }
