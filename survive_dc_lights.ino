/* Libraries */
#include <EEPROM.h>
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

void fill(LPD8806 &lpd, uint32_t color, uint16_t wait)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, color);
    lpd.show();
    delay(wait);
  }
}

// Lady Ada's "ordered dither" algorithm
void dither(LPD8806 &lpd, uint32_t color, uint16_t wait)
{
  // Determine highest bit needed to represent pixel index
  int hiBit = 0;
  int n = lpd.numPixels() - 1;
  for (int bit = 1; bit < 0x8000; bit <<= 1)
  {
    if(n & bit) hiBit = bit;
  }
  for (int i = 0; i < (hiBit << 1); ++i)
  {
    // Reverse the bits in i to create ordered dither:
    int reverse = 0;
    for (int bit = 1; bit <= hiBit; bit <<= 1)
    {
      reverse <<= 1;
      if (i & bit)
      {
        reverse |= 1;
      }
    }
    lpd.setPixelColor(reverse, color);
    lpd.show();
    delay(wait);
  }
}

/*
 * Pattern Functions
 */

void fill_init(LPD8806 &lpd)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, palette[palette_count-1]);
  }
}

void fill_repeat(LPD8806 &lpd)
{
  // cycle through colors at roughly 2 Hz
  unsigned ms = 500/lights.numPixels();
  for (size_t i = 0; i < palette_count; ++i)
  {
    fill(lpd, palette[i], ms);
  }
}

void dither_repeat(LPD8806 &lpd)
{
  // switch colors at roughly 1 Hz
  unsigned ms = 1000/lights.numPixels()/4;
  for (size_t i = 0; i < palette_count; ++i)
  {
    // fade in (1/4)
    dither(lpd, palette[i], ms);
    // hold (3/4)
    delay(ms*lights.numPixels()*3);
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
  { fill_init, fill_repeat },
  { fill_init, dither_repeat },
};
const size_t pattern_count = sizeof(pattern)/sizeof(pattern[0]);
size_t pattern_index = 0;

/*
 * Arduino Sketch Functions
 */

void setup()
{
  // read pattern_index from EEPROM
  pattern_index = EEPROM.read(0);
  if (pattern_index >= pattern_count)
  {
    pattern_index = 0;
  }
  // write next pattern_index to EEPROM (after short delay)
  delay(250);
  EEPROM.write(0, pattern_index + 1);

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

