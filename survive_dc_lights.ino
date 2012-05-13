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

const uint32_t palette[] = { safety_orange, blue };
const size_t palette_count = sizeof(palette)/sizeof(palette[0]);

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
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, color);
    lpd.show();
    delay(ms);
  }
}

/*
 * Pattern Functions
 */

void fill_1Hz_init(LPD8806 &lpd)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, palette[palette_count-1]);
  }
}

void fill_1Hz_repeat(LPD8806 &lpd)
{
  // alternate colors at roughly 1 Hz
  unsigned ms = 1000/lights.numPixels()/palette_count;
  for (size_t i = 0; i < palette_count; ++i)
  {
    fill(lpd, palette[i], ms);
  }
}

/*
 * Pattern Definitions
 */

typedef struct
{
  void (*init)(LPD8806 &lpd);  
  void (*repeat)(LPD8806 &lpd);  
} pattern_t;

const pattern_t pattern[] =
{
  { fill_1Hz_init, fill_1Hz_repeat },
};
const size_t pattern_count = sizeof(pattern)/sizeof(pattern[0]);
size_t pattern_index = 0;

/*
 * Arduino Sketch Functions
 */

void setup()
{
  // initialize lights
  lights.begin();
  pattern[pattern_index].init(lights);
  // display initial state
  lights.show();
}

void loop()
{
  pattern[pattern_index].repeat(lights);
}

