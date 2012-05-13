/* Libraries */
#include <LPD8806.h> // patched for 8-bit color I/O
#include <SPI.h>
/* Values */
#include "colors.h"

/*
 * Configuration
 */

const int nLEDs = 16;

//#define USE_HARDWARE_SPI 0
#define USE_HARDWARE_SPI 1

#if ! USE_HARDWARE_SPI
const int dataPin  = 2;
const int clockPin = 3;
#endif

/*
 * Globals
 */

#if USE_HARDWARE_SPI
LPD8806 lights(nLEDs);
#else
LPD8806 lights(nLEDs, dataPin, clockPin);
#endif

/*
 * Utility Functions
 */

void fill(LPD8806 &lpd, uint32_t color, uint16_t ms)
{
  for (int i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, color);
    lpd.show();
    delay(ms);
  }
}

/*
 * Arduino Sketch Functions
 */

void setup()
{
  // initialize lights
  lights.begin();
  // display initial state
  lights.show();
}

void loop()
{
  // alternate colors at roughly 1 Hz
  unsigned ms = 1000/lights.numPixels();
  fill(lights, blue, ms/2);
  fill(lights, safety_orange, ms/2);
}

