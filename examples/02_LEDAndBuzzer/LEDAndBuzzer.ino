/*
 * Example: RGB LED and Buzzer Control
 * Description: Demonstrates RGB LED color control and buzzer operation
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 */

#include "ESP32S3_Relay6CH.h"

ESP32S3_Relay6CH board;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32-S3-Relay-6CH LED & Buzzer Demo ===");
    
    // Initialize board
    ESP32S3_Relay6CH::Status status = board.begin();
    
    if (status != ESP32S3_Relay6CH::STATUS_OK) {
        Serial.println("❌ Board initialization failed!");
        while(1) delay(1000);
    }
    
    Serial.println("✓ Board initialized successfully");
}

void loop() {
    // Test different LED colors
    Serial.println("\n--- Testing LED Colors ---");
    
    Serial.println("🔴 Red LED");
    board.TickRedLED(1.0);
    delay(1500);
    
    Serial.println("🟢 Green LED");
    board.TickGreenLED(1.0);
    delay(1500);
    
    Serial.println("🔵 Blue LED");
    board.TickBlueLED(1.0);
    delay(1500);
    
    Serial.println("🟡 Yellow LED");
    board.TickYellowLED(1.0);
    delay(1500);
    
    Serial.println("🟣 Purple LED");
    board.TickPurpleLED(1.0);
    delay(1500);
    
    Serial.println("🟠 Orange LED");
    board.TickOrangeLED(1.0);
    delay(1500);
    
    Serial.println("⚪ White LED");
    board.TickWhiteLED(1.0);
    delay(1500);
    
    // Turn off LED
    Serial.println("⚫ LED Off");
    board.LEDOff();
    delay(1000);
    
    // Test buzzer
    Serial.println("\n--- Testing Buzzer ---");
    
    Serial.println("🔊 Buzzer: 1 beep");
    board.buzzer_beep(1);
    delay(500);
    
    Serial.println("🔊 Buzzer: 3 beeps");
    board.buzzer_beep(3);
    delay(1000);
    
    Serial.println("🔊 Buzzer: 5 beeps");
    board.buzzer_beep(5);
    delay(2000);
    
    // Combined test: relay + LED + buzzer
    Serial.println("\n--- Combined Test: Relay + LED + Buzzer ---");
    
    for (uint8_t i = 1; i <= 3; i++) {
        Serial.print("Cycle ");
        Serial.println(i);
        
        // Turn on relay 1, set LED to green, buzzer
        board.relayOn(1);
        board.setRGBColor(0, 255, 0);  // Green
        board.buzzer_beep(1);
        delay(1000);
        
        // Turn on relay 2, set LED to blue, buzzer
        board.relayOn(2);
        board.setRGBColor(0, 0, 255);  // Blue
        board.buzzer_beep(2);
        delay(1000);
        
        // Turn off all relays, LED off
        board.allRelaysOff();
        board.LEDOff();
        delay(500);
    }
    
    delay(2000);
}
