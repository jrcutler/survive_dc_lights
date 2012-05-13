/* Libraries */
#include <EEPROM.h>
#include <LPD8806.h> // patched for 8-bit color I/O
#include <SPI.h>
/* Values */
#include "CIELAB.h"
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

uint32_t palette[] = { safety_orange, blue };
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

// Create a 24-bit CIE lightness-adjusted color value from 8-bit R, G, B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = CIE_L(r);
  c <<= 8;
  c |= CIE_L(g);
  c <<= 8;
  c |= CIE_L(b);
  return c;
}

// 24-bit RGB color wheel mapped to [0, 767]
uint32_t RGB_ColorWheel(uint16_t pos)
{
  uint8_t r, g, b;
  uint32_t c;
  switch (pos >> 8)
  {
    case 0:
      r = 255 - pos; // red down
      g = pos;       // green up
      b = 0;         // blue off
      break;
    case 1:
      g = 255 - pos; // green down
      b = pos;       // blue up
      r = 0;         // red off
      break;
    case 2:
      b = 255 - pos; // blue down
      r = pos;       // red up
      g = 0;         // green off
      break;
  }
  return Color(r, g, b);
}

void fill_up(LPD8806 &lpd, uint32_t color, uint16_t wait)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, color);
    lpd.show();
    delay(wait);
  }
}

void fill_down(LPD8806 &lpd, uint32_t color, uint16_t wait)
{
  uint16_t i = lpd.numPixels() - 1;
  do
  {
    lpd.setPixelColor(i, color);
    lpd.show();
    delay(wait);
  }
  while (i-- != 0);
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

void shift_up(LPD8806 &lpd)
{
  uint32_t previous = lpd.getPixelColor(lpd.numPixels()-1);
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    uint32_t current = lpd.getPixelColor(i);
    lpd.setPixelColor(i, previous);
    previous = current;
  }
}

void shift_down(LPD8806 &lpd)
{
  uint32_t previous = lpd.getPixelColor(0);
  uint16_t i = lpd.numPixels() - 1;
  do
  {
    uint32_t current = lpd.getPixelColor(i);
    lpd.setPixelColor(i, previous);
    previous = current;
  }
  while (i-- != 0);
}

void cycle_up(LPD8806 &lpd, uint16_t wait)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    shift_up(lpd);
    lpd.show();
    delay(wait);
  }
}

void cycle_down(LPD8806 &lpd, uint16_t wait)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    shift_down(lpd);
    lpd.show();
    delay(wait);
  }
}

/*
 * Pattern Functions
 */

void null_init(LPD8806 &)
{
}

void rainbow_repeat(LPD8806 &lpd)
{
  for (uint16_t i = 0; i < 768; ++i)
  {
    for (uint16_t pixel = 0; pixel < lpd.numPixels(); ++pixel)
    {
      uint16_t color = (pixel * 768 / lpd.numPixels() + i) % 768;
      lpd.setPixelColor(pixel, RGB_ColorWheel(color));
    }
    lpd.show();
  }
}

void fill_init(LPD8806 &lpd)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, palette[palette_count-1]);
  }
}

void fill_up_repeat(LPD8806 &lpd)
{
  // cycle through colors at roughly 2 Hz
  unsigned ms = 500/lights.numPixels();
  for (size_t i = 0; i < palette_count; ++i)
  {
    fill_up(lpd, palette[i], ms);
  }
}

void fill_down_repeat(LPD8806 &lpd)
{
  // cycle through colors at roughly 2 Hz
  unsigned ms = 500/lights.numPixels();
  for (size_t i = 0; i < palette_count; ++i)
  {
    fill_down(lpd, palette[i], ms);
  }
}

void fade_repeat(LPD8806 &lpd)
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

// spread the palette across all pixels
void palette_init(LPD8806 &lpd)
{
  for (uint16_t i = 0; i < lpd.numPixels(); ++i)
  {
    lpd.setPixelColor(i, palette[i*palette_count/lpd.numPixels()]);
  }
}

void cycle_up_repeat(LPD8806 &lpd)
{
  // cycle through pixels at approximately 1 Hz
  unsigned ms = 1000/lights.numPixels();
  cycle_up(lpd, ms);
}

void cycle_down_repeat(LPD8806 &lpd)
{
  // cycle through pixels at approximately 1 Hz
  unsigned ms = 1000/lights.numPixels();
  cycle_down(lpd, ms);
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
  { null_init, rainbow_repeat },
  { fill_init, fade_repeat },
  { fill_init, fill_up_repeat },
  { fill_init, fill_down_repeat },
  { palette_init, cycle_up_repeat },
  { palette_init, cycle_down_repeat },
};
const size_t pattern_count = sizeof(pattern)/sizeof(pattern[0]);
size_t pattern_index = 0;

/*
 * Arduino Sketch Functions
 */

void setup()
{
#ifndef DEBUG
  // read pattern_index from EEPROM
  pattern_index = EEPROM.read(0);
#endif
  if (pattern_index >= pattern_count)
  {
    pattern_index = 0;
  }
#ifndef DEBUG
  // write next pattern_index to EEPROM (after short delay)
  delay(250);
  EEPROM.write(0, pattern_index + 1);
#endif

  // color correct palette for human perception
  for (size_t i = 0; i < palette_count; ++i)
  {
    palette[i] = CIE_L(palette[i]);
  }

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

