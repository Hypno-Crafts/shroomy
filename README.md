# Shroomy: Arduino Mushroom Incubator and Fruiting Controller

## [Build Guide](https://hypnocrafts.com/shroomy)

## Description
This project is an Arduino-based controller designed for managing the environmental conditions of a mushroom incubation and fruiting chamber. It automates fan and misting systems while providing visually appealing LED effects to indicate the current operation state.

The parameters in this code should be adapted based on the needs of the mushroom species you would like to grow. For this, check the customization section.

## Features
- **Three Operational Modes**:
  - **Fruiting Mode**: Designed for fruiting with increased frequency for venting and misting. Activated by pressing its dedicated button.
     - Fans: 30 seconds per cycle.
     - Mist: 5 seconds per cycle.
     - 6 cycles/hour.
  - **Incubation Mode**: Optimized for the growth phase. Reduced frequency for venting and misting. Activated by pressing its dedicated button. This mode is ignored when fruiting mode is enabled.
     - Fans: 30 seconds per cycle.
     - Mist: 5 seconds per cycle.
     - 1 cycle/hour.
  - **Rest Mode**: No venting and no misting. Activated when fruiting and incubating mode is disabled.
- **Automated Cycles**: Controls fans, misting, and resting periods dynamically.
- **Interactive LED Patterns**:
  - Breathing effects, shooting stars, sparks, and color cycles.
  - LED patterns change based on the operational state.
  - Can be disabled by switching off the dedicated button linked with the LED strips.
- **Button Inputs**: Switch between incubation/fruiting modes and enable/disable LED animations using external buttons.

---

## Software Setup
1. **Libraries**:
   - [Adafruit NeoPixel Library](https://github.com/adafruit/Adafruit_NeoPixel) for LED control.
   - Install via Arduino IDE's Library Manager.
2. **Arduino IDE**:
   - Load the provided `.ino` file into the Arduino IDE.
   - Configure the correct board and port in the IDE settings.

---

## Pin Configuration
| Pin  | Function                  |
|------|---------------------------|
| 9    | Fan control output        |
| 8    | Mist control output       |
| 7    | Lower LED strip output    |
| 6    | Upper LED strip output    |
| 5    | Star LED strip output     |
| 4    | Incubation mode button    |
| 3    | Fruiting mode button      |

(enabling/disabling LED strips is done by the physical wiring to a dedicated button, not via software)

---

## LED Effects
- **Fan Only**: Blue shooting star patterns.
- **Mist**: Droplet-like effects.
- **Rest**: Light blue breathing effect.
- **Mode Indicators**: Color changes for mode transitions.

---

## Customization
- Modify `CYCLES_INCUBATION`, `CYCLES_FRUIT`, `INCUBATION_FAN_TIME`, `FRUIT_FAN_TIME`, `INCUBATION_MIST_TIME`, and `FRUIT_MIST_TIME` constants for different timing configurations.
- Customize LED patterns by editing the corresponding functions (`ShowBreathColor`, `ShowDropletsColor`, etc.).

---

## License
This project is licensed under the MIT License. Feel free to use, modify, and share.
