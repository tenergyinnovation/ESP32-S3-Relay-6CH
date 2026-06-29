/*
 * Example: Interactive Serial Control
 * Description: Full interactive control via Serial Monitor
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 * 
 * Commands:
 *   RELAY <n> <ON/OFF>  - Control relay (1-6)
 *   LED <color> <time>  - LED control
 *   BUZZER <times>      - Beep buzzer
 *   STATUS              - Show all status
 *   HELP                - Show all commands
 */

#include "ESP32S3_Relay6CH.h"

ESP32S3_Relay6CH board;

void printHelp() {
    Serial.println("\n╔════════════════════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-Relay-6CH Interactive Control               ║");
    Serial.println("╚════════════════════════════════════════════════════════╝");
    Serial.println("\n📋 Available Commands:\n");
    
    Serial.println("1️⃣  RELAY <channel> <state>");
    Serial.println("    channel: 1-6");
    Serial.println("    state: ON, OFF, or ? (read status)");
    Serial.println("    Example: RELAY 1 ON");
    
    Serial.println("\n2️⃣  LED <color> <seconds>");
    Serial.println("    color: red, green, blue, yellow, purple, orange, white");
    Serial.println("    seconds: 0.1-10, or -1 to turn off");
    Serial.println("    Example: LED red 2");
    
    Serial.println("\n3️⃣  BUZZER <times>");
    Serial.println("    times: 1-10");
    Serial.println("    Example: BUZZER 3");
    
    Serial.println("\n4️⃣  STATUS");
    Serial.println("    Show all relay states and board info");
    
    Serial.println("\n7️⃣  HELP");
    Serial.println("    Show this help message\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Initialize board
    ESP32S3_Relay6CH::Status status = board.begin();
    
    if (status != ESP32S3_Relay6CH::STATUS_OK) {
        Serial.println("❌ Board initialization failed!");
        while(1) delay(1000);
    }
    
    Serial.println("✅ Board initialized successfully\n");
    printHelp();
}

void processRelayCommand(String params) {
    // Parse: RELAY <channel> <state>
    int space1 = params.indexOf(' ');
    if (space1 == -1) {
        Serial.println("❌ Usage: RELAY <channel> <state>");
        return;
    }
    
    String channel_str = params.substring(0, space1);
    String state_str = params.substring(space1 + 1);
    state_str.toUpperCase();
    
    uint8_t channel = channel_str.toInt();
    
    if (channel < 1 || channel > 6) {
        Serial.println("❌ Channel must be 1-6");
        return;
    }
    
    if (state_str == "ON") {
        board.relayOn(channel);
        Serial.print("✓ Relay ");
        Serial.print(channel);
        Serial.println(" ON");
    } else if (state_str == "OFF") {
        board.relayOff(channel);
        Serial.print("✓ Relay ");
        Serial.print(channel);
        Serial.println(" OFF");
    } else if (state_str == "?") {
        bool state = board.getRelayState(channel);
        Serial.print("📍 Relay ");
        Serial.print(channel);
        Serial.print(": ");
        Serial.println(state ? "ON" : "OFF");
    } else {
        Serial.println("❌ State must be ON, OFF, or ?");
    }
}

void processLEDCommand(String params) {
    // Parse: LED <color> <seconds>
    int space1 = params.indexOf(' ');
    if (space1 == -1) {
        Serial.println("❌ Usage: LED <color> <seconds>");
        return;
    }
    
    String color = params.substring(0, space1);
    String time_str = params.substring(space1 + 1);
    color.toLowerCase();
    
    float seconds = time_str.toFloat();
    
    if (seconds < 0) {
        board.LEDOff();
        Serial.println("✓ LED turned OFF");
        return;
    }
    
    if (color == "red") {
        board.TickRedLED(seconds);
        Serial.println("✓ LED: Red");
    } else if (color == "green") {
        board.TickGreenLED(seconds);
        Serial.println("✓ LED: Green");
    } else if (color == "blue") {
        board.TickBlueLED(seconds);
        Serial.println("✓ LED: Blue");
    } else if (color == "yellow") {
        board.TickYellowLED(seconds);
        Serial.println("✓ LED: Yellow");
    } else if (color == "purple") {
        board.TickPurpleLED(seconds);
        Serial.println("✓ LED: Purple");
    } else if (color == "orange") {
        board.TickOrangeLED(seconds);
        Serial.println("✓ LED: Orange");
    } else if (color == "white") {
        board.TickWhiteLED(seconds);
        Serial.println("✓ LED: White");
    } else {
        Serial.println("❌ Unknown color");
    }
}

void processBuzzerCommand(String params) {
    int times = params.toInt();
    
    if (times < 1 || times > 10) {
        Serial.println("❌ Beeps must be 1-10");
        return;
    }
    
    board.buzzer_beep(times);
    Serial.print("✓ Buzzer: ");
    Serial.print(times);
    Serial.println(" beep(s)");
}


void processCommand(String input) {
    input.trim();
    if (input.length() == 0) return;
    
    input.toUpperCase();
    int space = input.indexOf(' ');
    
    String command = (space == -1) ? input : input.substring(0, space);
    String params = (space == -1) ? "" : input.substring(space + 1);
    
    if (command == "RELAY") {
        processRelayCommand(params);
    } else if (command == "LED") {
        processLEDCommand(params);
    } else if (command == "BUZZER") {
        processBuzzerCommand(params);
    } else if (command == "STATUS") {
        Serial.println(board.getBoardInfo());
    } else if (command == "HELP") {
        printHelp();
    } else {
        Serial.println("❌ Unknown command. Type HELP for list of commands.");
    }
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        Serial.print("\n> ");
        Serial.println(input);
        processCommand(input);
        Serial.print("\n> ");
    }
    
    board.loop();
    delay(10);
}
