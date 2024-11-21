#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Make sure to not power too much LED lights at the same time directly from the 5V of the Arduino!
// This code only lights up one LED light at the time to avoid frying the Arduino board

#define PIXEL_PIN 6     // Digital IO pin connected to the LED strip
#define PIXEL_COUNT 40   // Number of NeoPixels
#define WAIT 200        // milliseconds per LED light before going to the next
#define BRIGHTNESS 100  // Set BRIGHTNESS (0 - 255)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

int mode = 0;    // Start mode in mode 0

void setup() {
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
  strip.setBrightness(BRIGHTNESS); // Set the brightness
}

void loop() {
  if(++mode > 2) mode = 0; // Advance to next mode

  switch(mode) {           // Start the new animation...
    case 0:
      colorWipe(strip.Color(255,   0,   0));    // Red
      break;
    case 1:
      colorWipe(strip.Color(  0, 255,   0));    // Green
      break;
    case 2:
      colorWipe(strip.Color(  0,   0, 255));    // Blue
      break;
  }
}

void colorWipe(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor((i + PIXEL_COUNT -1) % PIXEL_COUNT, (0,0,0)); // Turn off previous LED light
    strip.setPixelColor(i, color);         // Turn on current LED light
    strip.show();                          //  Update strip to match
    delay(WAIT);                           //  Pause for a moment
  }
}
