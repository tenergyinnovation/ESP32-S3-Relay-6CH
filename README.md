# ESP32-S3-Relay-6CH Library

Complete Arduino library for **Waveshare ESP32-S3-Relay-6CH** industrial relay control board.

## 📋 Features

✅ **6 Independent Relay Channels**
- 10A @ 250VAC or 30VDC per channel
- Optocoupler isolated (each channel)
- Soft control via GPIO pins

✅ **Wireless Connectivity**
- WiFi 2.4GHz (AP + STA modes)
- Bluetooth 5.0 / BLE support

✅ **Communication Interfaces**
- Isolated RS485 (UART2)
- Support for Modbus RTU and AT commands

✅ **Status Indicators**
- WS2812B RGB LED (GPIO38)
- Passive Buzzer (GPIO21)
- Power and RX/TX indicator LEDs

✅ **Expandable**
- 40-pin Pico HAT compatible header
- Support for CAN, RS232, LoRa, RTC (external modules)

## 🔧 Hardware Specifications

| Parameter | Value |
|-----------|-------|
| **Processor** | ESP32-S3 Dual-Core @ 240MHz |
| **Flash Memory** | 16 MB (QSPI) |
| **PSRAM** | 8 MB (DDR2) |
| **Relay Channels** | 6 (10A @ 250VAC / 30VDC) |
| **RS485 Baud Rate** | 300 - 115,200 bps |
| **Power Input** | 7V - 36V DC |
| **Dimensions** | 145 × 90 × 40 mm |
| **Enclosure** | Rail-mount DIN case |

## 📌 Pin Configuration

### Relay Channels
| Relay | GPIO |
|-------|------|
| Relay 1 | GPIO1 |
| Relay 2 | GPIO2 |
| Relay 3 | GPIO42 |
| Relay 4 | GPIO41 |
| Relay 5 | GPIO40 |
| Relay 6 | GPIO39 |

### Communication & Indicators
| Function | GPIO | Interface |
|----------|------|-----------|
| **RS485 TX** | GPIO4 | UART2 |
| **RS485 RX** | GPIO5 | UART2 |
| **RGB LED** | GPIO38 | SPI (NeoPixel) |
| **Buzzer** | GPIO21 | LEDC PWM |

## 🚀 Quick Start

### Installation

**Option 1: Arduino IDE (Library Manager)**
1. Open Arduino IDE
2. Sketch → Include Library → Manage Libraries
3. Search: "ESP32-S3-Relay-6CH"
4. Install the latest version

**Option 2: PlatformIO**
```bash
platformio lib install "ESP32-S3-Relay-6CH"
```

**Option 3: Manual Installation**
1. Download this repository
2. Copy to: `~/Documents/Arduino/libraries/ESP32-S3-Relay-6CH/`
3. Restart Arduino IDE

### Basic Example

```cpp
#include "ESP32S3_Relay6CH.h"

ESP32S3_Relay6CH board;

void setup() {
    Serial.begin(115200);
    board.begin();
}

void loop() {
    board.relayOn(1);      // Turn on relay 1
    delay(1000);
    board.relayOff(1);     // Turn off relay 1
    board.buzzer_beep(3);  // Beep 3 times
    delay(1000);
}
```

## 📚 API Reference

### Initialization
```cpp
ESP32S3_Relay6CH board;
board.begin();
```

### Relay Control
```cpp
board.relayOn(1);              // Turn ON relay 1
board.relayOff(1);             // Turn OFF relay 1
board.toggleRelay(1);          // Toggle relay 1
board.getRelayState(1);        // Read state (true/false)
board.allRelaysOn();           // All relays ON
board.allRelaysOff();          // All relays OFF
```

### RGB LED
```cpp
board.setRGBColor(255, 0, 0);  // Red
board.TickRedLED(2.0);         // Red blink for 2 seconds
board.LEDOff();                // Turn off
```

### Buzzer
```cpp
board.buzzer_beep(3);          // Beep 3 times
board.buzzerBeep(5, 100, 150); // 5 beeps, custom timing
```

### RS485
```cpp
board.setupRS485(9600);        // Initialize
board.sendRS485(data, len);    // Send
board.readRS485(buf, size);    // Receive
```

## 💡 Examples

### Beginner Examples
1. **01_BasicRelayControl** - Sequential relay control with patterns
   - Turn relays on/off in sequence
   - Visual feedback via LED and buzzer
   - Perfect for learning basic relay control

2. **02_LEDAndBuzzer** - RGB LED and Buzzer demonstrations
   - All 7 colors demonstration
   - Different buzz patterns
   - LED animations

### Intermediate Examples
3. **03_RS485Communication** - RS485 serial interface demo
   - Send/receive data over RS485
   - Stream mode and AT command mode
   - External device communication

4. **04_InteractiveControl** - Full command interface via Serial Monitor
   - Simple commands for relay, LED, buzzer control
   - Status reporting
   - Help menu with command examples

### Advanced Examples (Based on Waveshare Demo Analysis)
5. **05_MultiSourceControl** - Unified relay control from multiple sources
   - Single command handler for local + external relays
   - Inspired by Waveshare MAIN_ALL architecture
   - Extensible design for future WiFi/Bluetooth/MQTT integration
   - Command examples: `RELAY 1 ON`, `EXT 2 OFF`, `LED red 3`, `BUZZER 5`

6. **06_RS485Control** - External relay control via RS485
   - Local (GPIO) + External (RS485) relay control
   - Connection testing and diagnostics
   - Full-duplex RS485 communication
   - Status querying and raw data transmission

## 🏗️ Architecture & Design

### Why OOP Over Procedural Code?

This library uses **Object-Oriented Programming (OOP)** with separate classes for each subsystem (Relay, RS485, RGB, Buzzer), compared to the procedural approach in Waveshare's reference demo.

**Advantages of our OOP approach:**
- ✅ **Better Encapsulation** - Each subsystem is self-contained
- ✅ **State Management** - Efficient internal state tracking
- ✅ **Error Handling** - Comprehensive validation and feedback
- ✅ **Code Reusability** - Classes can be used independently
- ✅ **Maintainability** - Clear structure, easy to modify
- ✅ **Extensibility** - Simple to add new features

### Architectural Decisions

1. **No RTC Support** - The board has no dedicated RTC chip. External I2C pins conflict with RS485 TX/RX. Use NTP for network time or external RTC module if needed.

2. **Modular Relay Control** - Relays grouped in a single efficient class with state caching.

3. **Unified RS485 Interface** - Clean API for both AT commands and stream mode communication.

4. **LED State Machine** - RGB animations handled via non-blocking state machine in `update()`.

5. **LEDC PWM for Buzzer** - Hardware PWM for efficient, accurate tone generation.

See [WAVESHARE_DEMO_ANALYSIS.md](WAVESHARE_DEMO_ANALYSIS.md) for detailed comparison with official Waveshare reference code.

## 📖 Resources

- [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-Relay-6CH)
- [Product Page](https://www.waveshare.com/product/arduino/industrial-controller/esp32-s3-relay-6ch.htm)

## 📝 License

MIT License

## 👨‍💻 Author

Tenergy Innovation Co., Ltd.  
Email: uten.boonliam@tenergyinnovation.co.th

---

**Version**: 1.0.0 | **Last Updated**: 29 June 2026
