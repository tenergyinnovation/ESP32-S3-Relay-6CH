/*
 * Example: RS485 Communication
 * Description: Demonstrates RS485 serial communication (Modbus RTU, AT Commands, etc.)
 * Board: Waveshare ESP32-S3-Relay-6CH
 * Author: Tenergy Innovation Co., Ltd.
 * Date: 29 Jun 2026
 * 
 * Note: RS485 interface
 * - TX: GPIO4 (UART2)
 * - RX: GPIO5 (UART2)
 */

#include "ESP32S3_Relay6CH.h"

ESP32S3_Relay6CH board;

// Buffer for receiving data
uint8_t rxBuffer[256];
size_t bytesRead = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== ESP32-S3-Relay-6CH RS485 Communication ===");
    
    // Initialize board
    ESP32S3_Relay6CH::Status status = board.begin();
    
    if (status != ESP32S3_Relay6CH::STATUS_OK) {
        Serial.println("❌ Board initialization failed!");
        while(1) delay(1000);
    }
    
    Serial.println("✓ Board initialized successfully");
    
    // Setup RS485 interface (9600 baud by default)
    if (board.setupRS485(9600)) {
        Serial.println("✓ RS485 interface initialized at 9600 baud");
    } else {
        Serial.println("❌ RS485 initialization failed!");
    }
    
    // Display pinout info
    Serial.println("\n--- RS485 Pinout ---");
    Serial.println("TX Pin: GPIO4");
    Serial.println("RX Pin: GPIO5");
    Serial.println("Speed: 9600 baud");
    
    // Indicate readiness with LED
    board.setRGBColor(0, 255, 0);  // Green = ready
    delay(500);
    board.LEDOff();
}

void loop() {
    // Process serial input from user (via USB serial)
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            Serial.print("📤 Sending to RS485: ");
            Serial.println(command);
            
            // Send command via RS485
            board.sendRS485((uint8_t*)command.c_str(), command.length());
            board.sendRS485((uint8_t*)"\r\n", 2);  // Add line ending
            
            board.setRGBColor(255, 255, 0);  // Yellow = sending
            delay(100);
            board.LEDOff();
        }
    }
    
    // Check for incoming RS485 data
    if (board.availableRS485() > 0) {
        bytesRead = board.readRS485(rxBuffer, sizeof(rxBuffer) - 1);
        rxBuffer[bytesRead] = '\0';  // Null-terminate
        
        Serial.print("📥 Received from RS485 (");
        Serial.print(bytesRead);
        Serial.print(" bytes): ");
        
        // Display as text if printable
        bool isPrintable = true;
        for (size_t i = 0; i < bytesRead; i++) {
            if (rxBuffer[i] < 32 && rxBuffer[i] != '\r' && rxBuffer[i] != '\n') {
                isPrintable = false;
                break;
            }
        }
        
        if (isPrintable) {
            Serial.println((char*)rxBuffer);
        } else {
            // Display as hex
            Serial.print("[HEX] ");
            for (size_t i = 0; i < bytesRead; i++) {
                Serial.print(rxBuffer[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
        
        board.setRGBColor(0, 0, 255);  // Blue = receiving
        delay(100);
        board.LEDOff();
    }
    
    board.loop();  // Update board (LED, buzzer, etc.)
    delay(10);
}

/*
 * Example RS485 Commands to send:
 * 
 * For LoRa DTU modules (SX1262):
 * - AT              (test connection)
 * - AT+VER          (get version)
 * - AT+MODE=1       (stream mode)
 * - AT+BAUD=9600    (set baud rate)
 * - AT+SF=7         (spreading factor)
 * - AT+PWR=22       (transmit power)
 * 
 * For general Modbus RTU:
 * - Send Modbus RTU frames via RS485
 * - Typical frame: [SlaveID] [Function] [Address] [Value] [CRC_LO] [CRC_HI]
 */
