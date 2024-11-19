// Include necessary libraries
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Pin definitions
#define FAN_OUTPUT          9   // Output control of both fans
#define MIST_OUTPUT         8   // Output control of mist
#define LOWER_LED_OUTPUT    7   // Lower LED strip circle
#define UPPER_LED_OUTPUT    6   // Upper LED strip circle
#define STAR_LED_OUTPUT     5   // Star LED strip
#define INCUBATION_INPUT    4   // Button input for incubation mode
#define FRUIT_INPUT         3   // Button input for fruiting mode

// Timing variables (in milliseconds)
unsigned long cyclesPerHour = 1UL;
unsigned long fanTime = 1UL * 60UL * 1000UL;     // 1 minute in milliseconds
unsigned long mistTime = 1UL * 60UL * 1000UL;    // 1 minute in milliseconds
unsigned long restTime = 0UL;                    // Will be calculated later

// Constants for incubation and fruiting modes
const unsigned long CYCLES_INCUBATION = 1UL;
const unsigned long INCUBATION_FAN_TIME = 30UL * 1000UL;    // 30 seconds in milliseconds
const unsigned long INCUBATION_MIST_TIME = 5UL * 1000UL;    // 5 seconds in milliseconds

const unsigned long CYCLES_FRUIT = 6UL;
const unsigned long FRUIT_FAN_TIME = 30UL * 1000UL;         // 30 seconds in milliseconds
const unsigned long FRUIT_MIST_TIME = 5UL * 1000UL;         // 5 seconds in milliseconds

const int BUTTON_READINGS_SIZE = 10; // Size of the button readings array

// Define a function pointer type for LED color functions
typedef void (*ColorFunctionPointer)(uint32_t, unsigned long);

// Function prototypes
void ShowTimeColor(uint32_t color, unsigned long time);
void ShowSparkColor(uint32_t color, unsigned long time);
void ShowShootingStarColor(uint32_t color, unsigned long time);
void ShowBreathColor(uint32_t color, unsigned long time);
void ShowDropletsColor(uint32_t color, unsigned long time);
void FadeLEDs(unsigned long stepDelay, float dimFactor);
uint32_t SetColorIntensity(uint32_t color, double intensity);
uint32_t DimColor(uint32_t color, float dimFactor);
bool mostlyHigh(int numbers[]);
void updateMode(unsigned long time);
void Cycle(unsigned long time);
void enterFanOnlyState(unsigned long time);
void enterSprayMistState(unsigned long time);
void enterRestState(unsigned long time);

// Function pointers to control the LED patterns
ColorFunctionPointer currentMistColorFunction;
ColorFunctionPointer currentFanColorFunction;
ColorFunctionPointer currentRestColorFunction;

// LED strip setup
#define UPPER_CIRCLE_PIXEL_COUNT 32 // Number of LEDs on upper circle strip
#define LOWER_CIRCLE_PIXEL_COUNT 30 // Number of LEDs on lower circle strip
#define STAR_PIXEL_COUNT          12 // Number of LEDs on star strip

Adafruit_NeoPixel upperLedStrip(UPPER_CIRCLE_PIXEL_COUNT, UPPER_LED_OUTPUT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lowerLedStrip(LOWER_CIRCLE_PIXEL_COUNT, LOWER_LED_OUTPUT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel starLedStrip(STAR_PIXEL_COUNT, STAR_LED_OUTPUT, NEO_GRB + NEO_KHZ800);

// Star remapping array
int starRemap[] = {3, 7, 8, 2, 6, 9, 1, 5, 10, 0, 4, 11};

long firstPixelHue = 0;
int readingIndex = 0;

// Arrays to store button readings
int fruitButtonReadings[BUTTON_READINGS_SIZE];
int incubationButtonReadings[BUTTON_READINGS_SIZE];

// Define possible states
enum State {
  FAN_ONLY,
  SPRAY_MIST,
  REST
};

State currentState = FAN_ONLY;
unsigned long lastStateChange = 0;

void setup() {
  Serial.begin(9600);

  // Set pin modes
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FAN_OUTPUT, OUTPUT);
  pinMode(MIST_OUTPUT, OUTPUT);
  pinMode(INCUBATION_INPUT, INPUT_PULLUP);
  pinMode(FRUIT_INPUT, INPUT_PULLUP);

  // Initialize LED strips
  upperLedStrip.begin();
  lowerLedStrip.begin();
  starLedStrip.begin();
  upperLedStrip.show();
  lowerLedStrip.show();
  starLedStrip.show();

  // Initial outputs
  digitalWrite(FAN_OUTPUT, HIGH);
  digitalWrite(MIST_OUTPUT, LOW);

  // Seed the random number generator with a noise value
  randomSeed(analogRead(0));

  // Initialize function pointers
  currentMistColorFunction = &ShowTimeColor;
  currentFanColorFunction = &ShowTimeColor;
  currentRestColorFunction = &ShowTimeColor;

  lastStateChange = millis();
}

void loop() {
  unsigned long time = millis();

  // Update mode based on button readings
  updateMode(time);

  // Handle state transitions and LED updates
  Cycle(time);

  // Update reading index for circular buffer
  readingIndex = (++readingIndex) % BUTTON_READINGS_SIZE;
}

// Function to update the mode based on button readings
void updateMode(unsigned long time) {
  // Read button states and store in readings arrays
  fruitButtonReadings[readingIndex] = digitalRead(FRUIT_INPUT);
  incubationButtonReadings[readingIndex] = digitalRead(INCUBATION_INPUT);

  // Determine which mode we're in based on button readings
  if (mostlyHigh(fruitButtonReadings)) {
    // Fruiting mode
    cyclesPerHour = CYCLES_FRUIT;
    fanTime = FRUIT_FAN_TIME;
    mistTime = FRUIT_MIST_TIME;
    currentRestColorFunction = &ShowBreathColor;
    currentMistColorFunction = &ShowDropletsColor;
    currentFanColorFunction = &ShowShootingStarColor;
  } else if (mostlyHigh(incubationButtonReadings)) {
    // Incubation mode
    cyclesPerHour = CYCLES_INCUBATION;
    fanTime = INCUBATION_FAN_TIME;
    mistTime = INCUBATION_MIST_TIME;
    currentRestColorFunction = &ShowBreathColor;
    currentMistColorFunction = &ShowDropletsColor;
    currentFanColorFunction = &ShowShootingStarColor;
  } else {
    // Rest mode
    currentRestColorFunction = &ShowBreathColor;
    if (currentState != REST) {
      currentState = REST;
      lastStateChange = time;
      digitalWrite(FAN_OUTPUT, LOW);
      digitalWrite(MIST_OUTPUT, LOW);
    }
    fanTime = 0;
    mistTime = 0;
  }
}

// Function to determine if most readings are LOW (button pressed)
bool mostlyHigh(int numbers[]) {
  int sum = 0;
  for (int i = 0; i < BUTTON_READINGS_SIZE; i++) {
    sum += numbers[i];
  }
  return sum <= BUTTON_READINGS_SIZE / 2;
}

// Function to handle state transitions and LED updates
void Cycle(unsigned long time) {
  // Calculate restTime to fill up the hour
  unsigned long totalCycleTime = fanTime + mistTime;
  restTime = (60UL * 60UL * 1000UL - totalCycleTime * cyclesPerHour) / cyclesPerHour;

  switch (currentState) {
    case FAN_ONLY:
      if (time - lastStateChange >= fanTime) {
        enterSprayMistState(time);
      } else {
        currentFanColorFunction(upperLedStrip.Color(0, 0, 255), time - lastStateChange); // Blue when fans are running
      }
      break;

    case SPRAY_MIST:
      if (time - lastStateChange >= mistTime) {
        enterRestState(time);
      } else {
        currentMistColorFunction(upperLedStrip.Color(0, 0, 255), time - lastStateChange); // Blue when misting
      }
      break;

    case REST:
      if (time - lastStateChange >= restTime) {
        enterFanOnlyState(time);
      } else {
        currentRestColorFunction(upperLedStrip.Color(50, 100, 255), time - lastStateChange); // Light blue when resting
      }
      break;
  }
}

// State transition functions
void enterFanOnlyState(unsigned long time) {
  currentState = FAN_ONLY;
  lastStateChange = time;
  digitalWrite(FAN_OUTPUT, HIGH);
  digitalWrite(MIST_OUTPUT, LOW);
}

void enterSprayMistState(unsigned long time) {
  currentState = SPRAY_MIST;
  lastStateChange = time;
  digitalWrite(FAN_OUTPUT, HIGH);
  digitalWrite(MIST_OUTPUT, HIGH);
}

void enterRestState(unsigned long time) {
  currentState = REST;
  lastStateChange = time;
  digitalWrite(FAN_OUTPUT, LOW);
  digitalWrite(MIST_OUTPUT, LOW);
}

// Helper function to set color intensity
uint32_t SetColorIntensity(uint32_t color, double intensity) {
  uint8_t r = (uint8_t)((color >> 16 & 0xFF) * intensity);
  uint8_t g = (uint8_t)((color >> 8 & 0xFF) * intensity);
  uint8_t b = (uint8_t)((color & 0xFF) * intensity);
  return upperLedStrip.Color(r, g, b);
}

// Helper function to dim a color
uint32_t DimColor(uint32_t color, float dimFactor) {
  uint8_t r = (uint8_t)((color >> 16 & 0xFF) / dimFactor);
  uint8_t g = (uint8_t)((color >> 8 & 0xFF) / dimFactor);
  uint8_t b = (uint8_t)((color & 0xFF) / dimFactor);
  return upperLedStrip.Color(r, g, b);
}

// Function to fade LEDs gradually
void FadeLEDs(unsigned long stepDelay, float dimFactor) {
  // Update all strips with dimmed colors
  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    uint32_t originalColor = upperLedStrip.getPixelColor(i);
    upperLedStrip.setPixelColor(i, DimColor(originalColor, dimFactor));

    originalColor = lowerLedStrip.getPixelColor(i);
    lowerLedStrip.setPixelColor(i, DimColor(originalColor, dimFactor));
  }

  for (int i = 0; i < starLedStrip.numPixels(); i++) {
    uint32_t originalColor = starLedStrip.getPixelColor(i);
    starLedStrip.setPixelColor(i, DimColor(originalColor, dimFactor));
  }

  // Show updated colors
  upperLedStrip.show();
  lowerLedStrip.show();
  starLedStrip.show();

  // Delay to control fade speed
  delay(stepDelay);
}

// LED pattern functions
void ShowTimeColor(uint32_t color, unsigned long time) {
  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    if (i < (static_cast<int>(time / 60 / 1000) % UPPER_CIRCLE_PIXEL_COUNT)) {
      upperLedStrip.setPixelColor(i, color);
    } else {
      upperLedStrip.setPixelColor(i, upperLedStrip.Color(0, 0, 0));
    }

    if (i < (static_cast<int>(time / 1000) % LOWER_CIRCLE_PIXEL_COUNT)) {
      lowerLedStrip.setPixelColor(i, color);
    } else {
      lowerLedStrip.setPixelColor(i, upperLedStrip.Color(0, 0, 0));
    }
  }
  for (int i = 0; i < starLedStrip.numPixels(); i++) {
    starLedStrip.setPixelColor(i, color);
  }
  upperLedStrip.show();
  lowerLedStrip.show();
  starLedStrip.show();
}

// Shooting star pattern for the star LED strip
void ShowShootingStarColor(uint32_t color, unsigned long time) {
  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    if (i == (static_cast<int>(time / 50) % UPPER_CIRCLE_PIXEL_COUNT)) {
      upperLedStrip.setPixelColor(i, color);
    }
  }
  for (int i = 0; i < lowerLedStrip.numPixels(); i++) {
    if (i == (static_cast<int>(time / 40) % LOWER_CIRCLE_PIXEL_COUNT)) {
      lowerLedStrip.setPixelColor(i, color);
    }
  }

  // Shooting star effect on the star LED strip
  const int numRows = 8;
  const int rowSizes[numRows] = {2, 2, 2, 2, 1, 1, 1, 1};
  const int row0[] = {11, 4};
  const int row1[] = {10, 5};
  const int row2[] = {6, 9};
  const int row3[] = {7, 8};
  const int row4[] = {3};
  const int row5[] = {2};
  const int row6[] = {1};
  const int row7[] = {0};
  const int* starRemapShooting[numRows] = {row0, row1, row2, row3, row4, row5, row6, row7};

  if (fanTime <= 0) {
    return;
  }

  double proc = static_cast<double>(time) / static_cast<double>(fanTime);
  double step = 1.0 / static_cast<double>(numRows - 1);
  for (int i = 0; i < numRows; i++) {
    double timeStep = static_cast<double>(i) / static_cast<double>(numRows - 1);
    if (proc > timeStep) {
      double intensity = (proc - timeStep) / step;
      intensity = (intensity > 1.0) ? 1.0 : intensity;
      for (int j = 0; j < rowSizes[i]; j++) {
        uint32_t adjustedColor = SetColorIntensity(color, intensity);
        starLedStrip.setPixelColor(starRemapShooting[i][j], adjustedColor);
      }
    }
  }
  FadeLEDs(5, 1.2);
}

void ShowSparkColor(uint32_t color, unsigned long fadeTime) {
  // Randomly set initial colors
  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    if (random(0, 200) == 0) {
      upperLedStrip.setPixelColor(i, color);
    }
    if (random(0, 200) == 0) {
      lowerLedStrip.setPixelColor(i, color);
    }
  }

  // Set random LEDs on star strip
  for (int i = 0; i < starLedStrip.numPixels(); i++) {
    if (random(0, 200) == 0) {
      starLedStrip.setPixelColor(i, color);
    }
  }
  FadeLEDs(60, 1.05);
}

void ShowBreathColor(uint32_t color, unsigned long fadeTime) {
  unsigned long cycleLength = 1000UL * 5UL;
  double currProc = static_cast<double>((fadeTime % cycleLength)) / static_cast<double>(cycleLength);
  double endProc = (4.0 * (currProc - 0.5) * (currProc - 0.5)) / 2.0 + 0.1;

  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    upperLedStrip.setPixelColor(i, SetColorIntensity(color, endProc));
    lowerLedStrip.setPixelColor(i, SetColorIntensity(color, endProc));
  }

  // Randomly set star LEDs
  for (int i = 0; i < starLedStrip.numPixels(); i++) {
    if (random(0, 200) == 0) {
      starLedStrip.setPixelColor(i, color);
    }
  }
  FadeLEDs(60, 1.05);
}

void ShowDropletsColor(uint32_t color, unsigned long fadeTime) {
  // Randomly set initial colors
  for (int i = 0; i < upperLedStrip.numPixels(); i++) {
    if (random(0, 120) == 0) {
      upperLedStrip.setPixelColor(i, color);
    }
    if (random(0, 120) == 0) {
      lowerLedStrip.setPixelColor(i, color);
    }
  }

  // Randomly set star LEDs
  for (int i = 0; i < starLedStrip.numPixels(); i++) {
    if (random(0, 200) == 0) {
      starLedStrip.setPixelColor(i, color);
    }
  }
  FadeLEDs(2, 1.2);
}