#include <Adafruit_NeoPixel.h>

// Define the LED strip pin and the number of LEDs
#define LED_PIN    6  // Pin where the LED strip is connected
#define NUM_LEDS   30 // Number of LEDs on the strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();         // Initialize the LED strip
  strip.show();          // Turn off all LEDs initially
}

void loop() {
  // Test individual colors
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue

  // Perform a rainbow cycle
  rainbowCycle(20);

  // Blink all LEDs
  blinkTest(strip.Color(255, 255, 255), 500); // White blink
}

// Function to wipe a color across the LEDs
void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);  // Set color for each pixel
    strip.show();                  // Display the color
    delay(wait);                   // Wait before setting the next pixel
  }
}

// Function for a rainbow cycle effect
void rainbowCycle(int wait) {
  for (int j = 0; j < 256 * 5; j++) { // 5 cycles of all colors
    for (int i = 0; i < strip.numPixels(); i++) {
      int pixelIndex = (i * 256 / strip.numPixels() + j) & 255;
      strip.setPixelColor(i, Wheel(pixelIndex));
    }
    strip.show();
    delay(wait);
  }
}

// Helper function to generate rainbow colors
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// Function to blink the LEDs
void blinkTest(uint32_t color, int wait) {
  for (int i = 0; i < 3; i++) { // Blink 3 times
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, color); // Turn on LEDs
    }
    strip.show();
    delay(wait);

    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, 0); // Turn off LEDs
    }
    strip.show();
    delay(wait);
  }
}
