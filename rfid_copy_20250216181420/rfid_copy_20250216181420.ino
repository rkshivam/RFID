// Viral Science www.viralsciencecreativity.com www.youtube.com/c/viralscience
// Smart Attendance System with Google Sheets (ESP32 Version)

#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

//-----------------------------------------
#define RST_PIN  22
#define SS_PIN   21
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//-----------------------------------------
int blockNum = 2;
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbwavA27J3TMPr7pVJwHGU-rZitJM3W9kx7w6vmbzNgVvnQfUMtcI7DHNMuyBF8nzsyI/exec?name=";  // Enter Google Script URL

//-----------------------------------------
#define WIFI_SSID "GUEST-N"  // Enter WiFi Name
#define WIFI_PASSWORD "user@2025"  // Enter WiFi Password
//-----------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");
    
    // WiFi Connectivity
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    SPI.begin();
    mfrc522.PCD_Init();
}

void loop() {
    Serial.println("Scan your Card");
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;
    
    Serial.println("Reading RFID Data...");
    ReadDataFromBlock(blockNum, readBlockData);
    
    Serial.print("Card Data: ");
    for (int j = 0; j < 16; j++) {
        Serial.write(readBlockData[j]);
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String request_url = sheet_url + String((char*)readBlockData);
        request_url.trim();
        
        Serial.println("Sending Data: " + request_url);
        http.begin(request_url);
        int httpCode = http.GET();
        
        if (httpCode > 0) {
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    
    status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Reading failed: ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    Serial.println("Block read successfully");
}
