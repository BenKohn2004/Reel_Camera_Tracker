#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h> // Include the Servo library
#include <math.h>  // Include for trigonometric functions

Servo myServo; // Create a Servo object

const int servoPin = D4; // Pin to control the servo

// Structure to receive data
typedef struct struct_message {
    int position;  // Position of the fencer
    float angle;   // Angle from the strip
} struct_message;

struct_message incomingData; // Data received

// Servo and camera parameters
float adjacent = 0.0; // Distance from the strip
float desired_angle = 0.0; // Servo angle
const int offset = 12; // Offset to approximate center of two fencers

void onDataRecv(uint8_t *mac, uint8_t *incomingDataBytes, uint8_t len) {
    // Copy received data into the structure
    memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

    Serial.println("Data received:");
    Serial.print("Position: ");
    Serial.println(incomingData.position);
    Serial.print("Angle: ");
    Serial.println(incomingData.angle);

    // Calculate adjacent (distance from the strip)
    if (incomingData.angle != 0) { // Prevent division by zero
        adjacent = 100.0 / tan(incomingData.angle * M_PI / 180.0); // Convert angle to radians
    } else {
        adjacent = 0.0;
        Serial.println("Warning: Angle is zero, cannot calculate adjacent.");
    }

    // Adjust position to approximate center of two fencers
    int adjusted_position = incomingData.position + offset;

    // Determine desired angle
    if (adjusted_position >= 64 && adjusted_position <= 88) {
        // If fencers are near the center, point the camera at the center
        desired_angle = 90.0; // Center the camera
        Serial.println("Fencers near the center. Setting desired_angle to 90 degrees.");
    } else if (adjacent != 0) { // Prevent division by zero
        desired_angle = atan((100.0 - adjusted_position) / adjacent) * (180.0 / M_PI) + 90; // Convert radians to degrees
    } else {
        desired_angle = 0.0;
        Serial.println("Warning: Adjacent is zero, cannot calculate desired angle.");
    }

    // Move the servo to the desired angle
    Serial.print("Desired angle: ");
    Serial.println(desired_angle);
    if (desired_angle >= 0 && desired_angle <= 180) { // Ensure valid servo range
        myServo.write(desired_angle);
        Serial.print("Moving servo to angle: ");
        Serial.println(desired_angle);
    } else {
        Serial.println("Warning: Desired angle out of range (0-180).");
    }
}

void setup() {
    Serial.begin(115200); // Initialize Serial Monitor

    // Initialize servo and set to a known middle position
    myServo.attach(servoPin, 500, 2500);
    myServo.write(90);  // Set servo to middle (90 degrees)
    delay(500); // Give the servo time to reach position

    Serial.println("Servo initialized to 90 degrees!");

    // Initialize WiFi in Station mode
    WiFi.mode(WIFI_STA);

    // Initialize ESPNOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register callback for when data is received
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataRecv);

    Serial.println("ESP-NOW Receiver Ready!");
}


void loop() {
    // The main loop does nothing as all actions occur in the callback
    delay(10);
}
