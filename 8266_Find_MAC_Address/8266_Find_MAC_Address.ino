#include <ESP8266WiFi.h>

void setup() {
    Serial.begin(115200); // Initialize Serial Monitor
    WiFi.mode(WIFI_STA);  // Set ESP8266 to Station Mode
    Serial.println("ESP8266 MAC Address Finder");

    // Get and print the MAC address
    String macAddress = WiFi.macAddress();
    Serial.print("MAC Address: ");
    Serial.println(macAddress);
}

void loop() {
    // Nothing to do here
}
