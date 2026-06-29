/*
 * Example: RS485 External Relay Control (Advanced)
 * Description: Control external relay modules via RS485 communication
 *              Based on Waveshare demo patterns but with improved structure
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 * 
 * Features:
 *   - Local relay control
 *   - External relay control via RS485
 *   - Data sending/receiving over RS485
 *   - Stream mode and AT command mode
 *   - Unified interface for both local and remote relays
 *
 * Wiring:
 *   RS485 TX (GPIO4) -> Module A (GPIO5 RX/DI pin)
 *   RS485 RX (GPIO5) -> Module A (GPIO4 TX/DO pin)
 *   GND -> Module GND
 *   Module Power: 5V (from external power supply)
 *
 * Protocol:
 *   Send format: "CH<n> ON\r\n" or "CH<n> OFF\r\n"
 *   Response: "OK\r\n" or "ERR\r\n"
 */

#include "ESP32S3_Relay6CH.h"

// ============================================================
// CONFIGURATION
// ============================================================

#define BAUD_RATE_RS485      9600
#define RS485_TIMEOUT_MS     2000
#define MAX_EXTERNAL_RELAYS  8

#define STREAM_MODE     1  // Continuous data mode
#define AT_MODE         0  // AT command mode

// ============================================================
// GLOBAL VARIABLES
// ============================================================

ESP32S3_Relay6CH board;

// Relay state tracking
bool localRelayState[6] = {false, false, false, false, false, false};
bool externalRelayState[MAX_EXTERNAL_RELAYS] = {false, false, false, false, false, false, false, false};

uint8_t rs485ReceiveBuffer[256];
bool isRS485Connected = false;

// ============================================================
// RS485 COMMUNICATION FUNCTIONS
// ============================================================

/**
 * Send command to external relay via RS485
 * Command format: "CH<n> ON" or "CH<n> OFF"
 */
bool sendExternalRelayCommand(uint8_t channel, bool state) {
    if (channel < 1 || channel > MAX_EXTERNAL_RELAYS) {
        Serial.printf("❌ Invalid external channel %d\n", channel);
        return false;
    }
    
    // Build command string
    char command[32];
    snprintf(command, sizeof(command), "CH%d %s\r\n", channel, state ? "ON" : "OFF");
    
    // Send via RS485
    size_t sent = board.sendRS485((const uint8_t*)command, strlen(command));
    
    if (sent > 0) {
        Serial.printf("📤 Sent to RS485: %s", command);
        externalRelayState[channel - 1] = state;
        return true;
    } else {
        Serial.printf("❌ Failed to send RS485 command\n");
        return false;
    }
}

/**
 * Query external relay status via RS485
 */
bool queryExternalRelayStatus(uint8_t channel) {
    if (channel < 1 || channel > MAX_EXTERNAL_RELAYS) return false;
    
    // Build query command
    char command[32];
    snprintf(command, sizeof(command), "CH%d ?\r\n", channel);
    
    // Send query
    size_t sent = board.sendRS485((const uint8_t*)command, strlen(command));
    
    if (sent > 0) {
        Serial.printf("❓ Status query sent for CH%d\n", channel);
        
        // Wait for response with timeout
        unsigned long startTime = millis();
        while (millis() - startTime < RS485_TIMEOUT_MS) {
            size_t available = board.availableRS485();
            if (available > 0) {
                size_t read = board.readRS485(rs485ReceiveBuffer, sizeof(rs485ReceiveBuffer) - 1);
                if (read > 0) {
                    rs485ReceiveBuffer[read] = '\0';
                    Serial.printf("📥 Response: %s\n", (char*)rs485ReceiveBuffer);
                    return true;
                }
            }
            delay(10);
        }
        Serial.println("⏱️  Response timeout");
        return false;
    }
    return false;
}

/**
 * Initialize RS485 communication
 */
bool initRS485() {
    Serial.println("\n🔌 Initializing RS485...");
    
    if (!board.setupRS485(BAUD_RATE_RS485)) {
        Serial.println("❌ RS485 initialization failed!");
        return false;
    }
    
    Serial.println("✅ RS485 initialized successfully");
    Serial.printf("   Baudrate: %d bps\n", BAUD_RATE_RS485);
    Serial.printf("   Timeout: %d ms\n", RS485_TIMEOUT_MS);
    
    isRS485Connected = true;
    return true;
}

/**
 * Test RS485 connection by attempting AT command
 */
bool testRS485Connection() {
    Serial.println("\n🧪 Testing RS485 connection...");
    
    // Try to get a response
    String response = board.getRS485().sendATCommand("AT", 1000);
    
    if (response.length() > 0) {
        Serial.printf("✅ RS485 responsive: %s\n", response.c_str());
        return true;
    } else {
        Serial.println("⚠️  RS485 no response (device may be in stream mode)");
        // Not necessarily an error - device might be in stream mode
        return true;
    }
}

/**
 * Send raw data over RS485 (for debugging)
 */
void sendRawRS485Data(const char* data) {
    Serial.printf("📤 Sending raw data: %s\n", data);
    board.sendRS485((const uint8_t*)data, strlen(data));
}

/**
 * Receive and display RS485 data
 */
void handleRS485Input() {
    size_t available = board.availableRS485();
    
    if (available > 0) {
        size_t read = board.readRS485(rs485ReceiveBuffer, sizeof(rs485ReceiveBuffer) - 1);
        if (read > 0) {
            rs485ReceiveBuffer[read] = '\0';
            Serial.printf("📥 RS485 Data: %s\n", (char*)rs485ReceiveBuffer);
        }
    }
}

// ============================================================
// CONTROL FUNCTIONS
// ============================================================

/**
 * Control external relay
 */
void controlExternalRelay(uint8_t channel, const char* command) {
    String cmd = String(command);
    cmd.toUpperCase();
    
    if (cmd == "ON") {
        if (sendExternalRelayCommand(channel, true)) {
            Serial.printf("✅ EXT CH%d -> ON\n", channel);
        }
    }
    else if (cmd == "OFF") {
        if (sendExternalRelayCommand(channel, false)) {
            Serial.printf("✅ EXT CH%d -> OFF\n", channel);
        }
    }
    else if (cmd == "?") {
        queryExternalRelayStatus(channel);
    }
}

/**
 * Show all relay states (local + external)
 */
void showFullStatus() {
    Serial.println("\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║          FULL SYSTEM STATUS (Local + External)        ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    Serial.println(board.getBoardInfo().c_str());
    
    Serial.println("\n🔴 LOCAL RELAYS (Built-in - GPIO Control):");
    for (int i = 0; i < 6; i++) {
        bool state = board.getRelayState(i + 1);
        char indicator = state ? '●' : '○';
        Serial.printf("   CH%d: [%c] %s\n", i + 1, indicator, state ? "ON " : "OFF");
    }
    
    Serial.println("\n🔵 EXTERNAL RELAYS (via RS485):");
    Serial.printf("   Connection Status: %s\n", isRS485Connected ? "✅ Connected" : "❌ Disconnected");
    
    if (isRS485Connected) {
        for (int i = 0; i < MAX_EXTERNAL_RELAYS; i++) {
            char indicator = externalRelayState[i] ? '●' : '○';
            Serial.printf("   CH%d: [%c] %s\n", i + 1, indicator, externalRelayState[i] ? "ON " : "OFF");
        }
    }
    
    Serial.println();
}

/**
 * Print help message
 */
void printHelp() {
    Serial.println("\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║    ESP32-S3-Relay-6CH RS485 External Control          ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    Serial.println("📋 LOCAL RELAY CONTROL (GPIO):");
    Serial.println("   LOCAL <ch> <ON/OFF>  - Control local relay (ch: 1-6)\n");
    
    Serial.println("📡 EXTERNAL RELAY CONTROL (RS485):");
    Serial.println("   EXT <ch> <ON/OFF>    - Control external relay (ch: 1-8)");
    Serial.println("   EXT <ch> ?           - Query external relay status\n");
    
    Serial.println("🔧 RS485 OPERATIONS:");
    Serial.println("   TEST                 - Test RS485 connection");
    Serial.println("   SEND <data>          - Send raw data over RS485");
    Serial.println("   RECV                 - Read and display RS485 data\n");
    
    Serial.println("📊 DIAGNOSTICS:");
    Serial.println("   STATUS               - Show full system status");
    Serial.println("   INFO                 - Show board information");
    Serial.println("   HELP                 - Show this help message\n");
    
    Serial.println("💡 EXAMPLES:");
    Serial.println("   LOCAL 1 ON     - Turn on local relay CH1");
    Serial.println("   EXT 3 OFF      - Turn off external relay CH3");
    Serial.println("   EXT 5 ?        - Check status of external relay CH5\n");
}

// ============================================================
// SETUP & MAIN LOOP
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║    ESP32-S3-Relay-6CH RS485 External Relay Control    ║");
    Serial.println("║              Advanced Communication Example            ║");
    Serial.println("╚════════════════════════════════════════════════════════╝\n");
    
    // Initialize board
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    Serial.println("✅ Board initialized successfully!");
    
    // Initialize RS485
    if (initRS485()) {
        // Test connection
        testRS485Connection();
    } else {
        Serial.println("⚠️  Continuing without RS485 (local control only)");
    }
    
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
                String cmd = inputBuffer;
                cmd.toUpperCase();
                
                Serial.printf("\n📥 Command: %s\n", inputBuffer.c_str());
                
                if (cmd.startsWith("LOCAL")) {
                    int channel;
                    char state[10];
                    if (sscanf(inputBuffer.c_str(), "LOCAL %d %s", &channel, state) == 2) {
                        if (channel >= 1 && channel <= 6) {
                            if (strcmp(state, "ON") == 0) {
                                board.relayOn(channel);
                                localRelayState[channel - 1] = true;
                                board.buzzer_beep(1);
                            } else if (strcmp(state, "OFF") == 0) {
                                board.relayOff(channel);
                                localRelayState[channel - 1] = false;
                                board.buzzer_beep(1);
                            }
                        }
                    }
                }
                else if (cmd.startsWith("EXT")) {
                    int channel;
                    char state[10];
                    if (sscanf(inputBuffer.c_str(), "EXT %d %s", &channel, state) == 2) {
                        if (channel >= 1 && channel <= MAX_EXTERNAL_RELAYS) {
                            controlExternalRelay(channel, state);
                        }
                    }
                }
                else if (cmd == "TEST") {
                    testRS485Connection();
                }
                else if (cmd == "RECV") {
                    handleRS485Input();
                }
                else if (cmd.startsWith("SEND")) {
                    String data = inputBuffer.substring(5);
                    sendRawRS485Data(data.c_str());
                }
                else if (cmd == "STATUS") {
                    showFullStatus();
                }
                else if (cmd == "INFO") {
                    Serial.println(board.getBoardInfo().c_str());
                }
                else if (cmd == "HELP") {
                    printHelp();
                }
                else {
                    Serial.println("❌ Unknown command");
                }
                
                inputBuffer = "";
            }
            Serial.print("\n> ");
        }
        else if (c >= 32 && c <= 126) {
            inputBuffer += c;
            Serial.print(c);
        }
    }
    
    // Periodically check for RS485 data
    handleRS485Input();
    
    // Update board animations
    board.loop();
    
    delay(10);
}
