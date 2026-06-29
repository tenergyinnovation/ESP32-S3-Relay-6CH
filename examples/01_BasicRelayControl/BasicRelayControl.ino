/*
 * Example: Basic Relay Control
 * Description: Demonstrates basic relay on/off control on ESP32-S3-Relay-6CH
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 */

#include "ESP32S3_Relay6CH.h"

// Create board instance
ESP32S3_Relay6CH board;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32-S3-Relay-6CH Basic Control ===");
    
    // Initialize board
    ESP32S3_Relay6CH::Status status = board.begin();
    
    if (status != ESP32S3_Relay6CH::STATUS_OK) {
        Serial.println("❌ Board initialization failed!");
        while(1) delay(1000);
    }
    
    Serial.println("✓ Board initialized successfully");
    Serial.println(board.getBoardInfo());
    
    // Turn on RGB LED (green) to indicate ready
    board.setRGBColor(0, 255, 0);
    delay(500);
    board.LEDOff();
}

void loop() {
    // Sequential relay control: Turn on relays 1-6 sequentially
    Serial.println("\n--- Sequential Relay Control ---");
    
    for (uint8_t relay = 1; relay <= 6; relay++) {
        Serial.print("🔌 Turning ON Relay ");
        Serial.println(relay);
        
        board.relayOn(relay);
        board.setRGBColor(255, 0, 0);  // Red = relay on
        delay(1000);
        
        board.relayOff(relay);
        board.LEDOff();
        delay(500);
    }
    
    // Turn all relays on
    Serial.println("\n✓ Turning ALL relays ON");
    board.allRelaysOn();
    board.setRGBColor(0, 255, 0);  // Green = all on
    board.buzzerBeep(3, 100, 100);
    delay(2000);
    
    // Turn all relays off
    Serial.println("✓ Turning ALL relays OFF");
    board.allRelaysOff();
    board.LEDOff();
    delay(1000);
    
    // Toggle relays
    Serial.println("\n--- Toggle Relay Test ---");
    for (uint8_t relay = 1; relay <= 6; relay++) {
        Serial.print("🔄 Toggling Relay ");
        Serial.println(relay);
        
        board.toggleRelay(relay);
        board.TickYellowLED(0.5);
        delay(800);
    }
    
    Serial.println("\n--- Relay Status ---");
    Serial.println(board.getRelayStatus());
    
    delay(3000);
}
