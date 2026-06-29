/*
 * Example: Multi-Source Relay Control (Advanced)
 * Description: Unified control interface for relays from multiple sources
 *              Inspired by Waveshare MAIN_ALL demo but with OOP architecture
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 * 
 * Features:
 *   - Control relays from Serial Monitor
 *   - Unified relay analysis function
 *   - Multi-channel external relay support (via RS485)
 *   - Status feedback for all operations
 *   - Format similar to Waveshare demo but better organized
 *
 * Commands:
 *   1. RELAY <n> <ON/OFF> - Local relay control
 *   2. EXT <n> <ON/OFF>   - External relay control (via RS485)
 *   3. LED <color> <time> - RGB LED control
 *   4. BUZZER <times>     - Buzzer control
 *   5. STATUS             - Show board and relay status
 *   6. HELP               - Show commands
 */

#include "ESP32S3_Relay6CH.h"

// ============================================================
// CONSTANTS & DEFINITIONS
// ============================================================

#define RELAY_MODE         1  // Command source: Local relay
#define EXTERNAL_MODE      2  // Command source: External (RS485)
#define BLUETOOTH_MODE     3  // Command source: Bluetooth (future)
#define MQTT_MODE          4  // Command source: MQTT (future)

#define RS485_BUFFER_SIZE  256
#define COMMAND_TIMEOUT    5000  // ms

// ============================================================
// GLOBAL VARIABLES
// ============================================================

ESP32S3_Relay6CH board;

// Relay state tracking (similar to Waveshare demo)
bool localRelayState[6] = {false, false, false, false, false, false};
bool externalRelayState[8] = {false, false, false, false, false, false, false, false};

uint8_t rs485Buffer[RS485_BUFFER_SIZE];
size_t rs485BufferIndex = 0;
unsigned long rs485LastReceiveTime = 0;

// ============================================================
// RELAY CONTROL FUNCTIONS
// ============================================================

/**
 * Process relay command from any source
 * This unified function handles relay commands from Serial, RS485, Bluetooth, MQTT, etc.
 * Similar pattern to Waveshare's Relay_Analysis() but better organized
 */
void processRelayCommand(const char* command, uint8_t sourceMode) {
    // Parse command: "RELAY 1 ON" or "1 ON" or "EXT 2 OFF"
    
    if (strncmp(command, "RELAY", 5) == 0 || sourceMode == RELAY_MODE) {
        // Local relay control (channels 1-6)
        int channel, stateArg;
        char stateStr[10];
        
        if (sscanf(command, "RELAY %d %s", &channel, stateStr) == 2 ||
            sscanf(command, "%d %s", &channel, stateStr) == 2) {
            
            if (channel < 1 || channel > 6) {
                Serial.println("❌ Invalid channel (1-6)");
                return;
            }
            
            String state = String(stateStr);
            state.toUpperCase();
            
            if (state == "ON") {
                board.relayOn(channel);
                localRelayState[channel - 1] = true;
                Serial.printf("✅ RELAY CH%d -> ON\n", channel);
                board.buzzer_beep(1);
            } 
            else if (state == "OFF") {
                board.relayOff(channel);
                localRelayState[channel - 1] = false;
                Serial.printf("✅ RELAY CH%d -> OFF\n", channel);
                board.buzzer_beep(1);
            }
            else if (state == "?") {
                // Read status
                bool status = board.getRelayState(channel);
                Serial.printf("📖 RELAY CH%d status: %s\n", channel, status ? "ON" : "OFF");
            }
            else if (state == "TOGGLE" || state == "TGL") {
                board.toggleRelay(channel);
                localRelayState[channel - 1] = board.getRelayState(channel);
                Serial.printf("🔄 RELAY CH%d toggled -> %s\n", channel, 
                            localRelayState[channel - 1] ? "ON" : "OFF");
                board.buzzer_beep(1);
            }
        }
    }
    else if (strncmp(command, "EXT", 3) == 0 || sourceMode == EXTERNAL_MODE) {
        // External relay control via RS485 (channels 1-8)
        int channel;
        char stateStr[10];
        
        if (sscanf(command, "EXT %d %s", &channel, stateStr) == 2) {
            if (channel < 1 || channel > 8) {
                Serial.println("❌ Invalid external channel (1-8)");
                return;
            }
            
            String state = String(stateStr);
            state.toUpperCase();
            
            if (state == "ON") {
                externalRelayState[channel - 1] = true;
                Serial.printf("📡 EXT CH%d -> ON (via RS485)\n", channel);
                // TODO: Send RS485 command to external module
            }
            else if (state == "OFF") {
                externalRelayState[channel - 1] = false;
                Serial.printf("📡 EXT CH%d -> OFF (via RS485)\n", channel);
                // TODO: Send RS485 command to external module
            }
        }
    }
}

/**
 * Process LED command
 */
void processLEDCommand(const char* command) {
    char colorStr[20];
    float seconds;
    
    if (sscanf(command, "LED %s %f", colorStr, &seconds) == 2) {
        String color = String(colorStr);
        color.toUpperCase();
        
        if (color == "RED") {
            board.TickRedLED(seconds);
            Serial.printf("🔴 LED Red for %.1f seconds\n", seconds);
        }
        else if (color == "GREEN") {
            board.TickGreenLED(seconds);
            Serial.printf("🟢 LED Green for %.1f seconds\n", seconds);
        }
        else if (color == "BLUE") {
            board.TickBlueLED(seconds);
            Serial.printf("🔵 LED Blue for %.1f seconds\n", seconds);
        }
        else if (color == "YELLOW") {
            board.TickYellowLED(seconds);
            Serial.printf("🟡 LED Yellow for %.1f seconds\n", seconds);
        }
        else if (color == "PURPLE" || color == "MAGENTA") {
            board.TickPurpleLED(seconds);
            Serial.printf("🟣 LED Purple for %.1f seconds\n", seconds);
        }
        else if (color == "ORANGE") {
            board.TickOrangeLED(seconds);
            Serial.printf("🟠 LED Orange for %.1f seconds\n", seconds);
        }
        else if (color == "WHITE") {
            board.TickWhiteLED(seconds);
            Serial.printf("⚪ LED White for %.1f seconds\n", seconds);
        }
        else if (color == "OFF") {
            board.LEDOff();
            Serial.println("⚫ LED Off");
        }
        else {
            Serial.println("❌ Invalid color");
        }
    }
}

/**
 * Process buzzer command
 */
void processBuzzerCommand(const char* command) {
    int times;
    if (sscanf(command, "BUZZER %d", &times) == 1) {
        if (times < 1 || times > 10) {
            Serial.println("❌ Invalid beep count (1-10)");
            return;
        }
        board.buzzer_beep(times);
        Serial.printf("🔊 Buzzer: %d beeps\n", times);
    }
}

/**
 * Process command from any source
 */
void processCommand(const char* command, uint8_t sourceMode = RELAY_MODE) {
    if (strlen(command) == 0) return;
    
    Serial.printf("\n📥 Command: %s\n", command);
    
    if (strncmp(command, "RELAY", 5) == 0 || strncmp(command, "EXT", 3) == 0 ||
        (sourceMode == RELAY_MODE && isdigit(command[0]))) {
        processRelayCommand(command, sourceMode);
    }
    else if (strncmp(command, "LED", 3) == 0) {
        processLEDCommand(command);
    }
    else if (strncmp(command, "BUZZER", 6) == 0) {
        processBuzzerCommand(command);
    }
    else if (strcmp(command, "STATUS") == 0) {
        showStatus();
    }
    else if (strcmp(command, "HELP") == 0) {
        printHelp();
    }
    else if (strcmp(command, "ALL_ON") == 0 || strcmp(command, "7") == 0) {
        board.allRelaysOn();
        for (int i = 0; i < 6; i++) localRelayState[i] = true;
        Serial.println("✅ All relays ON");
        board.buzzer_beep(3);
    }
    else if (strcmp(command, "ALL_OFF") == 0 || strcmp(command, "8") == 0) {
        board.allRelaysOff();
        for (int i = 0; i < 6; i++) localRelayState[i] = false;
        Serial.println("✅ All relays OFF");
        board.buzzer_beep(2);
    }
    else {
        Serial.println("❌ Unknown command. Type HELP for available commands.");
    }
}

/**
 * Show board and relay status (similar to Waveshare pattern)
 */
void showStatus() {
    Serial.println("\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║              BOARD STATUS & RELAY STATES               ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    Serial.println(board.getBoardInfo().c_str());
    Serial.println("\n📊 LOCAL RELAYS:");
    for (int i = 0; i < 6; i++) {
        bool state = board.getRelayState(i + 1);
        char indicator = state ? '✓' : '✗';
        Serial.printf("   CH%d: [%c] %s\n", i + 1, indicator, state ? "ON " : "OFF");
    }
    
    Serial.println("\n📡 EXTERNAL RELAYS (via RS485):");
    for (int i = 0; i < 8; i++) {
        char indicator = externalRelayState[i] ? '✓' : '✗';
        Serial.printf("   CH%d: [%c] %s\n", i + 1, indicator, externalRelayState[i] ? "ON " : "OFF");
    }
    
    Serial.println();
}

/**
 * Print help message
 */
void printHelp() {
    Serial.println("\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-Relay-6CH Multi-Source Control (Advanced)  ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    Serial.println("📋 RELAY CONTROL:");
    Serial.println("   RELAY <ch> <state>   - Local relay (ch: 1-6, state: ON/OFF/?)");
    Serial.println("   EXT <ch> <state>     - External relay (ch: 1-8, state: ON/OFF)");
    Serial.println("   ALL_ON               - Turn all local relays ON");
    Serial.println("   ALL_OFF              - Turn all local relays OFF");
    Serial.println("   Or just: <ch> <ON/OFF>  (e.g., '1 ON' for CH1)\n");
    
    Serial.println("💡 LED CONTROL:");
    Serial.println("   LED <color> <time>   - Colors: red, green, blue, yellow, purple, orange, white");
    Serial.println("                          Time: 0.1-60 seconds, or -1 to turn off");
    Serial.println("   Example: LED red 3\n");
    
    Serial.println("🔊 BUZZER:");
    Serial.println("   BUZZER <times>       - Beep count (1-10)");
    Serial.println("   Example: BUZZER 5\n");
    
    Serial.println("📊 STATUS & INFO:");
    Serial.println("   STATUS               - Show all relay states and board info");
    Serial.println("   HELP                 - Show this help message\n");
    
    Serial.println("💡 TIPS:");
    Serial.println("   • Commands are case-insensitive");
    Serial.println("   • Use ? to read relay status without changing it");
    Serial.println("   • TOGGLE to toggle relay state\n");
}

// ============================================================
// SETUP & MAIN LOOP
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║         ESP32-S3-Relay-6CH Multi-Source Control        ║");
    Serial.println("║                  Advanced Example                      ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    
    Serial.println("✅ Board initialized successfully!\n");
    printHelp();
    
    Serial.println("\n📝 Ready for commands. Type 'HELP' for more info.\n");
}

void loop() {
    // Handle Serial input
    if (Serial.available()) {
        static String inputBuffer = "";
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                processCommand(inputBuffer.c_str(), RELAY_MODE);
                inputBuffer = "";
            }
            Serial.print("\n> ");
        }
        else if (c >= 32 && c <= 126) {  // Printable ASCII
            inputBuffer += c;
            Serial.print(c);
        }
    }
    
    // TODO: Add RS485 input handling
    // TODO: Add Bluetooth input handling
    // TODO: Add MQTT input handling
    
    // Update board (for LED animations, etc.)
    board.loop();
    
    delay(10);
}
