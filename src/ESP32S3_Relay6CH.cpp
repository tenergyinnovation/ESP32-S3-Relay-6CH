/***********************************************************************
 * File         :     ESP32S3_Relay6CH.cpp
 * Description  :     Unified library implementation for ESP32-S3-Relay-6CH
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     29 Jun 2026
 * Revision     :     1.0.0
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 ***********************************************************************/

#include "ESP32S3_Relay6CH.h"

// Relay pins definition
constexpr uint8_t ESP32S3_Relay::RELAY_PINS[6];

// ============================================================
// SECTION 1: RELAY CONTROLLER IMPLEMENTATION
// ============================================================

ESP32S3_Relay::ESP32S3_Relay() : _relayState(0) {}

bool ESP32S3_Relay::begin() {
    _initializeGPIO();
    return true;
}

void ESP32S3_Relay::_initializeGPIO() {
    for (int i = 0; i < CHANNELS; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], LOW);  // All relays off initially
    }
}

bool ESP32S3_Relay::_validateChannel(uint8_t channel) {
    return (channel <= CHANNELS);
}

uint8_t ESP32S3_Relay::_normalizeChannel(uint8_t channel) {
    // Accept both 1-6 (user format) and 0-5 (array index format)
    if (channel >= 1 && channel <= CHANNELS) {
        return channel - 1;  // Convert 1-6 to 0-5
    } else if (channel < CHANNELS) {
        return channel;  // Already 0-5
    }
    return 0;  // Invalid: default to 0
}

bool ESP32S3_Relay::set(uint8_t channel, RelayState state) {
    if (!_validateChannel(channel)) return false;
    
    channel = _normalizeChannel(channel);
    
    switch(state) {
        case RELAY_ON:
            digitalWrite(RELAY_PINS[channel], HIGH);
            _relayState |= (1 << channel);
            return true;
        case RELAY_OFF:
            digitalWrite(RELAY_PINS[channel], LOW);
            _relayState &= ~(1 << channel);
            return true;
        case RELAY_TOGGLE:
            // Toggle the relay state directly
            if ((_relayState >> channel) & 1) {
                // Currently ON, turn OFF
                digitalWrite(RELAY_PINS[channel], LOW);
                _relayState &= ~(1 << channel);
            } else {
                // Currently OFF, turn ON
                digitalWrite(RELAY_PINS[channel], HIGH);
                _relayState |= (1 << channel);
            }
            return true;
        default:
            return false;
    }
}

bool ESP32S3_Relay::on(uint8_t channel) {
    return set(channel, RELAY_ON);
}

bool ESP32S3_Relay::off(uint8_t channel) {
    return set(channel, RELAY_OFF);
}

bool ESP32S3_Relay::toggle(uint8_t channel) {
    // Delegate to set() to avoid double-normalization
    return set(channel, RELAY_TOGGLE);
}

ESP32S3_Relay::RelayState ESP32S3_Relay::getState(uint8_t channel) {
    if (!_validateChannel(channel)) return RELAY_OFF;
    channel = _normalizeChannel(channel);
    return ((_relayState >> channel) & 1) ? RELAY_ON : RELAY_OFF;
}

uint8_t ESP32S3_Relay::getMask() {
    return _relayState;
}

uint8_t ESP32S3_Relay::getRawState() {
    uint8_t state = 0;
    for (int i = 0; i < CHANNELS; i++) {
        if (digitalRead(RELAY_PINS[i]) == HIGH) {
            state |= (1 << i);
        }
    }
    return state;
}

bool ESP32S3_Relay::setMask(uint8_t mask) {
    for (int i = 0; i < CHANNELS; i++) {
        if (mask & (1 << i)) {
            digitalWrite(RELAY_PINS[i], HIGH);
        } else {
            digitalWrite(RELAY_PINS[i], LOW);
        }
    }
    _relayState = mask;
    return true;
}

bool ESP32S3_Relay::allOn() {
    return setMask(0x3F);  // 0b111111 = all 6 channels on
}

bool ESP32S3_Relay::allOff() {
    return setMask(0x00);
}

String ESP32S3_Relay::getStatusString() {
    String status = "Relay Status: ";
    for (int i = 1; i <= CHANNELS; i++) {
        status += "R" + String(i) + "=";
        status += (getState(i) == RELAY_ON) ? "ON " : "OFF ";
    }
    return status;
}

bool ESP32S3_Relay::isOperational() {
    return true;  // All relays are operational
}

// ============================================================
// SECTION 2: RS485 COMMUNICATION IMPLEMENTATION
// ============================================================

ESP32S3_RS485::ESP32S3_RS485() 
    : _baudrate(9600), _commMode(MODE_AT_COMMAND), _serial(nullptr),
      _rxIndex(0), _lastRxTime(0), _bytesSent(0), _bytesReceived(0) {
    memset(_rxBuffer, 0, BUFFER_SIZE);
}

bool ESP32S3_RS485::begin(unsigned long baudrate, CommMode mode) {
    _baudrate = baudrate;
    _commMode = mode;
    _serial = &Serial2;
    _initializeUART();
    return true;
}

void ESP32S3_RS485::_initializeUART() {
    if (_serial) {
        _serial->begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    }
}

void ESP32S3_RS485::end() {
    if (_serial) {
        _serial->end();
    }
}

size_t ESP32S3_RS485::write(const uint8_t* data, size_t length) {
    if (!_serial) return 0;
    size_t written = _serial->write(data, length);
    _bytesSent += written;
    return written;
}

size_t ESP32S3_RS485::writeString(const char* str) {
    if (!_serial) return 0;
    size_t written = _serial->write(str);
    _bytesSent += written;
    return written;
}

size_t ESP32S3_RS485::read(uint8_t* buffer, size_t maxLength) {
    if (!_serial) return 0;
    
    size_t bytesRead = 0;
    while (_serial->available() && bytesRead < maxLength) {
        buffer[bytesRead++] = _serial->read();
        _bytesReceived++;
    }
    return bytesRead;
}

size_t ESP32S3_RS485::available() {
    if (!_serial) return 0;
    return _serial->available();
}

int ESP32S3_RS485::peek() {
    if (!_serial) return -1;
    return _serial->peek();
}

void ESP32S3_RS485::flush() {
    if (_serial) {
        _serial->flush();
    }
}

void ESP32S3_RS485::clearRxBuffer() {
    memset(_rxBuffer, 0, BUFFER_SIZE);
    _rxIndex = 0;
}

bool ESP32S3_RS485::changeBaudrate(unsigned long baudrate) {
    _baudrate = baudrate;
    if (_serial) {
        _serial->updateBaudRate(baudrate);
        return true;
    }
    return false;
}

String ESP32S3_RS485::sendATCommand(const char* command, uint16_t timeout) {
    if (!_serial) return "ERROR";
    
    // Clear RX buffer first
    while (_serial->available()) _serial->read();
    
    // Send command
    _serial->print(command);
    _serial->print("\r\n");
    
    // Wait for response
    unsigned long startTime = millis();
    String response = "";
    
    while (millis() - startTime < timeout) {
        if (_serial->available()) {
            char c = _serial->read();
            response += c;
            
            // Check for typical AT command responses
            if (response.endsWith("\r\n")) {
                return response;
            }
        }
    }
    
    return response.length() > 0 ? response : "TIMEOUT";
}

void ESP32S3_RS485::resetStats() {
    _bytesSent = 0;
    _bytesReceived = 0;
}

void ESP32S3_RS485::update() {
    _processRxBuffer();
}

void ESP32S3_RS485::_processRxBuffer() {
    if (!_serial) return;
    
    while (_serial->available() && _rxIndex < BUFFER_SIZE) {
        _rxBuffer[_rxIndex++] = _serial->read();
        _lastRxTime = millis();
    }
}

// ============================================================
// SECTION 3: RGB LED IMPLEMENTATION
// ============================================================

ESP32S3_RGB::ESP32S3_RGB() 
    : _currentColor(0), _targetColor(0), _brightness(255),
      _lastUpdateTime(0), _blinkOnTime(500), _blinkOffTime(500),
      _blinkCount(0), _isBlinking(false), _blinkStartTime(0), _neoPixel(nullptr) {}

bool ESP32S3_RGB::begin() {
    _neoPixel = new Adafruit_NeoPixel(NUM_LEDS, RGB_PIN, NEO_GRB + NEO_KHZ800);
    if (_neoPixel) {
        _neoPixel->begin();
        _neoPixel->clear();
        _neoPixel->show();
        return true;
    }
    return false;
}

void ESP32S3_RGB::end() {
    if (_neoPixel) {
        _neoPixel->clear();
        _neoPixel->show();
        delete _neoPixel;
        _neoPixel = nullptr;
    }
}

void ESP32S3_RGB::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    setColor((uint32_t)((red << 16) | (green << 8) | blue));
}

void ESP32S3_RGB::setColor(uint32_t color) {
    _currentColor = color;
    _isBlinking = false;
    _updateLED();
}

void ESP32S3_RGB::setStatusColor(StatusColor status) {
    setColor((uint32_t)status);
}

void ESP32S3_RGB::blink(uint32_t color, uint16_t onTime, uint16_t offTime, uint16_t count) {
    _currentColor = color;
    _blinkOnTime = onTime;
    _blinkOffTime = offTime;
    _blinkCount = count;
    _isBlinking = true;
    _blinkStartTime = millis();
}

void ESP32S3_RGB::pulse(uint32_t color, uint16_t period) {
    setColor(color);
}

void ESP32S3_RGB::stop() {
    _isBlinking = false;
    _currentColor = COLOR_OFF;
    _updateLED();
}

void ESP32S3_RGB::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void ESP32S3_RGB::tickRedLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_RED, halfTime, halfTime);
}

void ESP32S3_RGB::tickGreenLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_GREEN, halfTime, halfTime);
}

void ESP32S3_RGB::tickBlueLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_BLUE, halfTime, halfTime);
}

void ESP32S3_RGB::tickYellowLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_YELLOW, halfTime, halfTime);
}

void ESP32S3_RGB::tickPurpleLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_MAGENTA, halfTime, halfTime);
}

void ESP32S3_RGB::tickOrangeLED(float seconds) {
    // Split total time equally between ON and OFF (fallback to magenta)
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_MAGENTA, halfTime, halfTime);
}

void ESP32S3_RGB::tickWhiteLED(float seconds) {
    // Split total time equally between ON and OFF
    uint16_t halfTime = (uint16_t)(seconds * 1000 / 2);
    blink(COLOR_WHITE, halfTime, halfTime);
}

void ESP32S3_RGB::update() {
    if (_isBlinking) {
        unsigned long elapsed = millis() - _blinkStartTime;
        unsigned long period = _blinkOnTime + _blinkOffTime;
        
        if (_blinkCount > 0 && (elapsed / period) >= _blinkCount) {
            _isBlinking = false;
            _currentColor = COLOR_OFF;
        } else {
            uint32_t phase = elapsed % period;
            if (phase < _blinkOnTime) {
                // Light on
                _updateLED();
            } else {
                // Light off
                if (_neoPixel) {
                    _neoPixel->setPixelColor(0, 0);
                    _neoPixel->show();
                }
            }
        }
    }
}

void ESP32S3_RGB::_updateLED() {
    if (!_neoPixel) return;
    
    uint8_t r = (_currentColor >> 16) & 0xFF;
    uint8_t g = (_currentColor >> 8) & 0xFF;
    uint8_t b = _currentColor & 0xFF;
    
    _applyBrightness(r, g, b);
    
    uint32_t grb = _rgbToGrb((r << 16) | (g << 8) | b);
    _neoPixel->setPixelColor(0, grb);
    _neoPixel->show();
}

void ESP32S3_RGB::_applyBrightness(uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (r * _brightness) / 255;
    g = (g * _brightness) / 255;
    b = (b * _brightness) / 255;
}

uint32_t ESP32S3_RGB::_rgbToGrb(uint32_t rgbColor) {
    uint8_t r = (rgbColor >> 16) & 0xFF;
    uint8_t g = (rgbColor >> 8) & 0xFF;
    uint8_t b = rgbColor & 0xFF;
    return (g << 16) | (r << 8) | b;
}

// ============================================================
// SECTION 4: BUZZER IMPLEMENTATION
// ============================================================

ESP32S3_Buzzer::ESP32S3_Buzzer()
    : _frequency(DEFAULT_FREQ), _currentVolume(255), _isPlaying(false),
      _playStartTime(0), _playDuration(0) {}

bool ESP32S3_Buzzer::begin(uint16_t frequency) {
    _frequency = frequency;
    _initializeGPIO();
    return true;
}

void ESP32S3_Buzzer::end() {
    off();
}

void ESP32S3_Buzzer::_initializeGPIO() {
    // Configure LEDC PWM for buzzer
    ledcSetup(PWM_CHANNEL, _frequency, PWM_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);  // Start with buzzer off
}

void ESP32S3_Buzzer::on() {
    ledcWrite(PWM_CHANNEL, _currentVolume);
}

void ESP32S3_Buzzer::off() {
    ledcWrite(PWM_CHANNEL, 0);
    _isPlaying = false;
}

void ESP32S3_Buzzer::setVolume(uint8_t volume) {
    _currentVolume = volume;
}

void ESP32S3_Buzzer::beep(uint16_t duration, uint8_t volume) {
    _currentVolume = volume;
    _playDuration = duration;
    _playStartTime = millis();
    _isPlaying = true;
    on();
}

void ESP32S3_Buzzer::playPattern(BuzzerPattern pattern) {
    switch(pattern) {
        case PATTERN_SHORT:
            beep(100, _currentVolume);
            break;
        case PATTERN_LONG:
            beep(500, _currentVolume);
            break;
        case PATTERN_DOUBLE:
            beep(100, _currentVolume);
            // Second beep handled in update()
            break;
        case PATTERN_TRIPLE:
            beep(100, _currentVolume);
            // Remaining beeps handled in update()
            break;
        case PATTERN_ALARM:
            beep(200, _currentVolume);
            break;
        default:
            break;
    }
}

void ESP32S3_Buzzer::setFrequency(uint16_t frequency) {
    _frequency = frequency;
    ledcChangeFrequency(PWM_CHANNEL, frequency, PWM_RESOLUTION);
}

void ESP32S3_Buzzer::stop() {
    off();
}

void ESP32S3_Buzzer::update() {
    _updatePlayback();
}

void ESP32S3_Buzzer::_updatePlayback() {
    if (_isPlaying && (millis() - _playStartTime >= _playDuration)) {
        off();
    }
}

// ============================================================
// SECTION 5: MAIN BOARD CONTROLLER IMPLEMENTATION
// ============================================================

ESP32S3_Relay6CH::ESP32S3_Relay6CH()
    : initialized(false), rs485_enabled(false), _status(STATUS_OK),
      _ledBlinking(false), _ledBlinkColor(0), _ledBlinkStartTime(0), _ledBlinkInterval(0) {}

ESP32S3_Relay6CH::Status ESP32S3_Relay6CH::begin() {
    // Initialize relay controller
    if (!_relay.begin()) {
        _status = STATUS_RELAY_ERROR;
        return _status;
    }

    // Initialize RGB LED
    if (!_rgb.begin()) {
        _status = STATUS_RGB_ERROR;
        return _status;
    }

    // Initialize Buzzer
    if (!_buzzer.begin()) {
        _status = STATUS_BUZZER_ERROR;
        return _status;
    }

    initialized = true;
    _status = STATUS_OK;
    return _status;
}

void ESP32S3_Relay6CH::relayOn(uint8_t channel) {
    _relay.on(channel);
}

void ESP32S3_Relay6CH::relayOff(uint8_t channel) {
    _relay.off(channel);
}

void ESP32S3_Relay6CH::toggleRelay(uint8_t channel) {
    _relay.toggle(channel);
}

void ESP32S3_Relay6CH::setRelayState(uint8_t channel, bool state) {
    if (state) {
        relayOn(channel);
    } else {
        relayOff(channel);
    }
}

bool ESP32S3_Relay6CH::getRelayState(uint8_t channel) {
    return _relay.getState(channel) == ESP32S3_Relay::RELAY_ON;
}

void ESP32S3_Relay6CH::allRelaysOn() {
    _relay.allOn();
}

void ESP32S3_Relay6CH::allRelaysOff() {
    _relay.allOff();
}

void ESP32S3_Relay6CH::setRGBColor(uint8_t red, uint8_t green, uint8_t blue) {
    _rgb.setColor(red, green, blue);
}

void ESP32S3_Relay6CH::setLEDColor(uint32_t color) {
    _rgb.setColor(color);
}

void ESP32S3_Relay6CH::LEDOff() {
    _rgb.stop();
}

void ESP32S3_Relay6CH::TickRedLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickRedLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickGreenLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickGreenLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickBlueLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickBlueLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickYellowLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickYellowLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickPurpleLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickPurpleLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickOrangeLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickOrangeLED(seconds);
    }
}

void ESP32S3_Relay6CH::TickWhiteLED(float seconds) {
    if (seconds < 0) {
        LEDOff();
    } else {
        _rgb.tickWhiteLED(seconds);
    }
}

void ESP32S3_Relay6CH::buzzer_beep(int times) {
    // Direct PWM control for audible buzzer feedback
    // Beep pattern: 100ms ON, 100ms OFF
    const uint16_t BEEP_DURATION = 100;        // milliseconds
    const uint16_t SILENCE_DURATION = 100;    // milliseconds
    const uint8_t BEEP_VOLUME = 200;           // 0-255 (200 = 78% duty cycle)
    
    for (int i = 0; i < times; i++) {
        // Turn buzzer ON (use delay() instead of delayMicroseconds)
        ledcWrite(ESP32S3_Buzzer::PWM_CHANNEL, BEEP_VOLUME);
        delay(BEEP_DURATION);  // Use millisecond delay for stable PWM signal
        
        // Turn buzzer OFF
        ledcWrite(ESP32S3_Buzzer::PWM_CHANNEL, 0);
        delay(SILENCE_DURATION);  // Silence between beeps
    }
}

void ESP32S3_Relay6CH::buzzerBeep(uint8_t times, uint16_t duration, uint16_t interval) {
    for (int i = 0; i < times; i++) {
        _buzzer.beep(duration, 255);
        delay(duration + interval);
    }
}

/***********************************************************************
 * FUNCTION:    buzzer_test
 * DESCRIPTION: Diagnostic function to test buzzer hardware directly.
 *              Generates a continuous tone for 2 seconds.
 * PARAMETERS:  None
 * RETURNED:    None
 * NOTE:        Use this to test if buzzer hardware is working correctly.
 ***********************************************************************/
void ESP32S3_Relay6CH::buzzer_test() {
    // Generate continuous tone for 2 seconds
    ledcWrite(ESP32S3_Buzzer::PWM_CHANNEL, 200);  // Set duty cycle
    delay(2000);  // Keep tone for 2 seconds
    ledcWrite(ESP32S3_Buzzer::PWM_CHANNEL, 0);    // Turn off
}

bool ESP32S3_Relay6CH::setupRS485(uint32_t baudRate) {
    if (_rs485.begin(baudRate)) {
        rs485_enabled = true;
        return true;
    }
    return false;
}

size_t ESP32S3_Relay6CH::sendRS485(const uint8_t* data, size_t length) {
    return _rs485.write(data, length);
}

size_t ESP32S3_Relay6CH::readRS485(uint8_t* buffer, size_t maxLength) {
    return _rs485.read(buffer, maxLength);
}

size_t ESP32S3_Relay6CH::availableRS485() {
    return _rs485.available();
}



String ESP32S3_Relay6CH::getBoardInfo() {
    String info = "\n=== ESP32-S3-Relay-6CH Board Info ===\n";
    info += "Board: " + String(BOARD_NAME) + "\n";
    info += "Library Version: " + String(LIBRARY_VERSION) + "\n";
    info += "Relay Channels: " + String(RELAY_CHANNELS) + "\n";
    info += "Status: " + String(_status == STATUS_OK ? "OK" : "ERROR") + "\n";
    info += "Relay Status: " + _relay.getStatusString() + "\n";
    return info;
}

String ESP32S3_Relay6CH::getRelayStatus() {
    return _relay.getStatusString();
}

void ESP32S3_Relay6CH::loop() {
    _rgb.update();
    _buzzer.update();
    _rs485.update();
}

void ESP32S3_Relay6CH::configurePins() {
    // All GPIO configuration is done in subsystem begin() methods
}

void ESP32S3_Relay6CH::initLED() {
    _rgb.begin();
}

void ESP32S3_Relay6CH::initBuzzer() {
    _buzzer.begin();
}
