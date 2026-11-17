# MadILI9341

Buffered SPI adapter for ILI9341, using Adafruit ILI9341. MadDisplay owns UI,
navigation and motion; this package owns hardware initialization and transfers.

```cpp
#include <MadDisplay/MadDisplay.hpp>
#include <MadILI9341/MadILI9341.hpp>

mad::ILI9341 adapter(csPin, dcPin, resetPin); // reset is optional
mad::Display<mad::ILI9341> display(adapter);
// setup: display.begin(rotation, spiFrequency); register/open your pages.
// loop:  display.render(micros());
```

`begin(rotation=0, spiFrequency=0, differential=true)` accepts rotations 0–3:
240×320, 320×240, 240×320 upside-down, 320×240 upside-down. Frequency 0 uses the
vendor driver's default. A custom SPI bus can be passed to
`ILI9341(SPIClass&, dc, cs, reset)` on targets supported by Adafruit. The driver
uses blocking burst writes; no asynchronous DMA buffer lifetime is assumed.

The RGB565 framebuffer needs 153,600 bytes. Exact differential updates need
another 153,600 bytes plus 1,280 bytes of row ranges. When previous-frame memory
cannot be allocated, initialization keeps working with full-frame transfers;
check `differentialUpdates()`. `begin(..., false)` explicitly disables history.
Initialization returns false on insufficient frame memory or invalid rotation.
SPI has no device acknowledgment: a true result does not verify physical wiring.

For RAM/PSRAM allocated by the application, call
`setFrameBuffers(frame, pixelCapacity, previous, previousPixelCapacity)` before
`begin()`. Each supplied buffer must have room for 76,800 `uint16_t` values.
Previous-frame storage is optional. Buffers cannot overlap and must outlive the
adapter; it never frees externally supplied storage. Ordinary heap storage is
allocated only during `begin()`, with no per-frame allocations.

`present()` compares final pixels, including erasure. Unchanged frames issue no
SPI transaction. Small changes transfer only changed spans of rows; sufficiently
large changes use one full-frame address window. Comparisons are exact, without
hash collision artifacts. `lastTransferPixels()` and `lastTransferWindows()`
report payload/window counts of the last presentation. `invalidate()` forces a
full refresh after direct driver access. `setRotation()` clears/invalidate the
frame so geometry and transfers stay synchronized.

The `examples/color_ui` sketch is configured for ESP32 in `platformio.ini`.
Use suitable RAM/PSRAM for the chosen buffer mode and set pins for your wiring.

API reference: [Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341),
[Adafruit SPITFT transaction API](https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_SPITFT.h).
Host tests emulate controller RAM and compare the resulting image after full,
partial, unchanged and rotated frames. ESP32 firmware is compiled against real
Adafruit libraries. Electrical timing and frame rate still need hardware tests.

## Installation

Repository: https://github.com/Astonikum/ili9341-lib

PlatformIO (`platformio.ini`):

```ini
lib_deps =
  https://github.com/Astonikum/ili9341-lib.git#v0.2.0
```

PlatformIO resolves the core, adapter support and vendor dependencies automatically.

Arduino IDE: download **Code → Download ZIP**, then use **Sketch → Include Library → Add .ZIP Library**.
Install these GitHub ZIP libraries first:

- [MadDisplay](https://github.com/Astonikum/display-lib)
- [MadDisplayAdapters](https://github.com/Astonikum/adapter-common-lib)

Install from Arduino Library Manager: Adafruit ILI9341.

These packages are distributed through GitHub; they are not yet indexed in the Arduino or PlatformIO registries. Arduino IDE does not install a library directly from a Git URL.

Requires C++14 on the host. Embedded examples select their board and its Arduino toolchain in `platformio.ini`. RAM requirements are documented above; a matching controller does not guarantee a board has enough memory.
