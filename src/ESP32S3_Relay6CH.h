/***********************************************************************
 * File         :     ESP32S3_Relay6CH.h
 * Description  :     Unified library for ESP32-S3-Relay-6CH board
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     29 Jun 2026
 * Revision     :     1.0.1
 * Rev1.0.0     :     - Created for ESP32-S3-Relay-6CH [29-06-2026]
 * Rev1.0.1     :     - Fix TickerLED animations [30-06-2026 15:28]
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 * 
 * Board Info:
 * - 6 Relay Channels (10A @ 250VAC / 30VDC)
 * - RS485 Interface (Isolated)
 * - WiFi 2.4GHz + Bluetooth 5.0
 * - RGB LED (WS2812B NeoPixel)
 * - Passive Buzzer
 * - 40-pin Pico HAT header (expandable)
 ***********************************************************************/

#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>

/**
 * ============================================================
 * SECTION 1: RELAY CONTROLLER - 6 Channels
 * ============================================================
 * Controls 6 independent relay channels with optocoupler isolation
 * - Relay 1: GPIO1  | Relay 4: GPIO42
 * - Relay 2: GPIO2  | Relay 5: GPIO45
 * - Relay 3: GPIO41 | Relay 6: GPIO46
 */
class ESP32S3_Relay {
public:
    // GPIO pin definitions for relay channels
    static constexpr uint8_t RELAY_PINS[6] = {1, 2, 41, 42, 45, 46};
    static constexpr uint8_t CHANNELS = 6;

    // Relay state definitions
    enum RelayState {
        RELAY_OFF = 0,
        RELAY_ON = 1,
        RELAY_TOGGLE = 2
    };

    // Constructor
    ESP32S3_Relay();

    // Initialization
    bool begin();

    // Relay control operations
    /***********************************************************************
     * FUNCTION:    set
     * DESCRIPTION: Controls the state of a specific relay channel.
     * PARAMETERS:  channel - Relay channel (1-6 or 0-5)
     *              state   - RELAY_OFF, RELAY_ON, or RELAY_TOGGLE
     * RETURNED:    true if successful, false if channel invalid
     ***********************************************************************/
    bool set(uint8_t channel, RelayState state);

    bool on(uint8_t channel);
    bool off(uint8_t channel);
    bool toggle(uint8_t channel);

    // State query
    /***********************************************************************
     * FUNCTION:    getState
     * DESCRIPTION: Reads the current state of a specific relay channel.
     * PARAMETERS:  channel - Relay channel (1-6 or 0-5)
     * RETURNED:    RELAY_ON if relay is energized, RELAY_OFF if not
     ***********************************************************************/
    RelayState getState(uint8_t channel);

    uint8_t getMask();
    uint8_t getRawState();

    // Batch operations
    bool setMask(uint8_t mask);
    bool allOn();
    bool allOff();

    // Status query
    String getStatusString();
    bool isOperational();

private:
    // State cache
    uint8_t _relayState;  // Cached relay state (6-bit)

    // Helper methods
    void _initializeGPIO();
    bool _validateChannel(uint8_t channel);
    uint8_t _normalizeChannel(uint8_t channel);
};

/**
 * ============================================================
 * SECTION 2: RS485 COMMUNICATION INTERFACE
 * ============================================================
 * Isolated RS485 interface for Modbus RTU and custom protocols
 * - TX: GPIO4  (UART2)
 * - RX: GPIO5  (UART2)
 */
class ESP32S3_RS485 {
public:
    // Pin definitions for RS485 (UART2)
    static constexpr uint8_t TX_PIN = 4;
    static constexpr uint8_t RX_PIN = 5;
    static constexpr uint8_t UART_NUM = 2;
    static constexpr size_t BUFFER_SIZE = 256;

    // Communication modes
    enum CommMode {
        MODE_AT_COMMAND = 0,
        MODE_STREAM = 1
    };

    // Constructor
    ESP32S3_RS485();

    // Initialization
    bool begin(unsigned long baudrate = 9600, CommMode mode = MODE_AT_COMMAND);
    void end();

    // Data transmission
    size_t write(const uint8_t* data, size_t length);
    size_t writeString(const char* str);

    // Data reception
    size_t read(uint8_t* buffer, size_t maxLength);
    size_t available();
    int peek();

    // Buffer control
    void flush();
    void clearRxBuffer();

    // Mode and baud control
    void setCommMode(CommMode mode) { _commMode = mode; }
    CommMode getCommMode() { return _commMode; }
    bool changeBaudrate(unsigned long baudrate);
    unsigned long getBaudrate() { return _baudrate; }

    // AT command support
    String sendATCommand(const char* command, uint16_t timeout = 1000);

    // Statistics
    uint32_t getBytesSent() { return _bytesSent; }
    uint32_t getBytesReceived() { return _bytesReceived; }
    void resetStats();

    // Update routine (call in main loop)
    void update();

private:
    // Serial configuration
    unsigned long _baudrate;
    CommMode _commMode;
    HardwareSerial *_serial;

    // RX buffer
    uint8_t _rxBuffer[BUFFER_SIZE];
    size_t _rxIndex;
    unsigned long _lastRxTime;

    // Statistics
    uint32_t _bytesSent;
    uint32_t _bytesReceived;

    // Helper methods
    void _initializeUART();
    void _processRxBuffer();
};

/**
 * ============================================================
 * SECTION 3: RGB LED INDICATOR CONTROLLER
 * ============================================================
 * WS2812B NeoPixel RGB LED for status indication
 * - Pin: GPIO38
 */
class ESP32S3_RGB {
public:
    // Pin definition
    static constexpr uint8_t RGB_PIN = 38;
    static constexpr uint8_t NUM_LEDS = 1;

    // RGB status indicator colors
    enum StatusColor {
        COLOR_OFF = 0x000000,
        COLOR_RED = 0xFF0000,
        COLOR_YELLOW = 0xFFFF00,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE = 0x0000FF,
        COLOR_CYAN = 0x00FFFF,
        COLOR_MAGENTA = 0xFF00FF,
        COLOR_WHITE = 0xFFFFFF
    };

    // Constructor
    ESP32S3_RGB();

    // Initialization
    bool begin();
    void end();

    // Color control
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setColor(uint32_t color);
    void setStatusColor(StatusColor status);
    uint32_t getColor() { return _currentColor; }

    // Animation effects
    void blink(uint32_t color, uint16_t onTime, uint16_t offTime, uint16_t count = 0);
    void pulse(uint32_t color, uint16_t period = 1000);
    void stop();

    // Brightness control
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() { return _brightness; }

    // Ticker LED animations (high-level control)
    void tickRedLED(float seconds);
    void tickGreenLED(float seconds);
    void tickBlueLED(float seconds);
    void tickYellowLED(float seconds);
    void tickPurpleLED(float seconds);
    void tickOrangeLED(float seconds);
    void tickWhiteLED(float seconds);

    // Update routine (call in main loop)
    void update();

private:
    // State variables
    uint32_t _currentColor;
    uint32_t _targetColor;
    uint8_t _brightness;
    unsigned long _lastUpdateTime;
    uint16_t _blinkOnTime;
    uint16_t _blinkOffTime;
    uint16_t _blinkCount;
    bool _isBlinking;
    unsigned long _blinkStartTime;

    // NeoPixel object pointer for WS2812B LED
    Adafruit_NeoPixel *_neoPixel;

    // Ticker objects for background LED animations (one for each color)
    static Ticker tickerRed;
    static Ticker tickerGreen;
    static Ticker tickerBlue;
    static Ticker tickerYellow;
    static Ticker tickerPurple;
    static Ticker tickerOrange;
    static Ticker tickerWhite;

    // Static callback functions for Ticker interrupts
    static void _toggleRedLED();
    static void _toggleGreenLED();
    static void _toggleBlueLED();
    static void _toggleYellowLED();
    static void _togglePurpleLED();
    static void _toggleOrangeLED();
    static void _toggleWhiteLED();

    // Static pointer to current instance for callback access
    static ESP32S3_RGB *_instance;

    // Helper methods
    void _initializeGPIO();
    void _updateLED();
    void _applyBrightness(uint8_t &r, uint8_t &g, uint8_t &b);
    uint32_t _rgbToGrb(uint32_t rgbColor);
    
    // Stop all LED animations (called before starting new animation)
    void _stopAllLEDs();
    
    // Internal toggle methods
    void _toggleLED(uint32_t color);
};

/**
 * ============================================================
 * SECTION 4: BUZZER CONTROLLER
 * ============================================================
 * Passive buzzer controlled via PWM
 * - Pin: GPIO21
 * - LEDC Channel: 0
 */
class ESP32S3_Buzzer {
public:
    // Pin definition
    static constexpr uint8_t BUZZER_PIN = 21;
    static constexpr uint8_t PWM_CHANNEL = 0;
    static constexpr uint16_t DEFAULT_FREQ = 1000;
    static constexpr uint8_t PWM_RESOLUTION = 8;
    static constexpr uint8_t MAX_DUTY = 255;

    // Buzzer patterns
    enum BuzzerPattern {
        PATTERN_SHORT = 0,
        PATTERN_LONG = 1,
        PATTERN_DOUBLE = 2,
        PATTERN_TRIPLE = 3,
        PATTERN_ALARM = 4
    };

    // Constructor
    ESP32S3_Buzzer();

    // Initialization
    bool begin(uint16_t frequency = DEFAULT_FREQ);
    void end();

    // Basic control
    void on();
    void off();
    void setVolume(uint8_t volume);
    uint8_t getVolume() { return _currentVolume; }

    // Beeping operations
    /***********************************************************************
     * FUNCTION:    beep
     * DESCRIPTION: Produces a single beep with specified duration and volume.
     * PARAMETERS:  duration - Duration in milliseconds (default 100ms)
     *              volume   - Volume level 0-255 (default 255 = max)
     * RETURNED:    None
     ***********************************************************************/
    void beep(uint16_t duration = 100, uint8_t volume = 255);

    void playPattern(BuzzerPattern pattern);

    // Frequency control
    void setFrequency(uint16_t frequency);
    uint16_t getFrequency() { return _frequency; }

    // Status
    bool isPlaying() { return _isPlaying; }

    // Control
    void stop();

    // Update routine (call in main loop)
    void update();

private:
    // State variables
    uint16_t _frequency;
    uint8_t _currentVolume;
    bool _isPlaying;
    unsigned long _playStartTime;
    uint16_t _playDuration;

    // Helper methods
    void _initializeGPIO();
    void _updatePlayback();
};

/**
 * ============================================================
 * SECTION 5: MAIN BOARD CONTROLLER CLASS
 * ============================================================
 */
class ESP32S3_Relay6CH {
public:
    // Board information constants
    static constexpr const char* BOARD_NAME = "ESP32-S3-Relay-6CH";
    static constexpr uint8_t RELAY_CHANNELS = 6;
    static constexpr uint16_t MAX_BUFFER_SIZE = 256;
    static constexpr const char* LIBRARY_VERSION = "1.0.1";

    // Status codes
    enum Status : uint8_t {
        STATUS_OK = 0,
        STATUS_INIT_FAILED = 1,
        STATUS_RELAY_ERROR = 2,
        STATUS_RS485_ERROR = 3,
        STATUS_RGB_ERROR = 4,
        STATUS_BUZZER_ERROR = 5,
        STATUS_SYSTEM_ERROR = 255
    };

    // Constructor
    ESP32S3_Relay6CH();

    // Initialization
    Status begin();

    /**
     * ========== RELAY CONTROL METHODS ==========
     */
    void relayOn(uint8_t channel);
    void relayOff(uint8_t channel);
    void toggleRelay(uint8_t channel);
    void setRelayState(uint8_t channel, bool state);
    bool getRelayState(uint8_t channel);
    void allRelaysOn();
    void allRelaysOff();

    /**
     * ========== RGB LED CONTROL METHODS ==========
     */
    void setRGBColor(uint8_t red, uint8_t green, uint8_t blue);
    void setLEDColor(uint32_t color);
    void LEDOff();
    
    // Ticker LED animations (high-level interface)
    void TickRedLED(float seconds);
    void TickGreenLED(float seconds);
    void TickBlueLED(float seconds);
    void TickYellowLED(float seconds);
    void TickPurpleLED(float seconds);
    void TickOrangeLED(float seconds);
    void TickWhiteLED(float seconds);

    /**
     * ========== BUZZER CONTROL METHODS ==========
     */
    /***********************************************************************
     * FUNCTION:    buzzer_beep
     * DESCRIPTION: Produces multiple beeps with specified interval.
     * PARAMETERS:  times - Number of beeps (1-10)
     * RETURNED:    None
     ***********************************************************************/
    void buzzer_beep(int times);
    void buzzerBeep(uint8_t times = 1, uint16_t duration = 100, uint16_t interval = 100);
    
    /***********************************************************************
     * FUNCTION:    buzzer_test
     * DESCRIPTION: Diagnostic function to test buzzer hardware.
     * PARAMETERS:  None
     * RETURNED:    None
     ***********************************************************************/
    void buzzer_test();

    /**
     * ========== RS485 INTERFACE METHODS ==========
     */
    bool setupRS485(uint32_t baudRate = 9600);
    size_t sendRS485(const uint8_t* data, size_t length);
    size_t readRS485(uint8_t* buffer, size_t maxLength);
    size_t availableRS485();

    /**
     * ========== BOARD INFORMATION METHODS ==========
     */
    String getBoardInfo();
    String getRelayStatus();

    // Subsystem access (for advanced users)
    ESP32S3_Relay& getRelay() { return _relay; }
    ESP32S3_RS485& getRS485() { return _rs485; }
    ESP32S3_RGB& getRGB() { return _rgb; }
    ESP32S3_Buzzer& getBuzzer() { return _buzzer; }

    // Update routine (call in main loop)
    void loop();

    // Status query
    Status getStatus() { return _status; }
    bool isInitialized() { return initialized; }

private:
    // Subsystem controllers (internal instances)
    ESP32S3_Relay _relay;
    ESP32S3_RS485 _rs485;
    ESP32S3_RGB _rgb;
    ESP32S3_Buzzer _buzzer;

    // Status flags
    bool initialized;
    bool rs485_enabled;
    Status _status;

    // LED Animation State
    bool _ledBlinking;
    uint32_t _ledBlinkColor;
    unsigned long _ledBlinkStartTime;
    float _ledBlinkInterval;

    // Helper methods
    void configurePins();
    void initLED();
    void initBuzzer();
};
