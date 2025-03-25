#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h>  // Include the Servo library
#include <math.h>   // Include for trigonometric functions

Servo myServo;  // Create a Servo object

bool verbose = false;       // Set to true to enable serial output, false to disable
bool invertServo = false;  // Set to true to invert servo rotation

const int servoPin = D4;  // Pin to control the servo

int initial_positionLeft = 0;     // Left reel starts at position 0
int initial_positionRight = 0;  // Right reel starts at position 364 (opposite end)

// Structure to receive data
typedef struct struct_message {
  uint8_t senderID;  // Unique ID for the sender
  int position;
  float angle;
} struct_message;

struct_message incomingDataLeft;   // Data received from Left reel
struct_message incomingDataRight;  // Data received from Right reel

// Define position conversion factor
const float strip_center = 182.0;                                   // Defines the center of the strip in position units
const float strip_center_meters = 7.0;                              // Defines the center of the strip in meters
const float conversionFactor = strip_center / strip_center_meters;  // Encoder positions per meter
const int engardePosition = round(5.0 * conversionFactor);          // Left fencer en garde at 5m
const int positionDelta = round(0.25 * conversionFactor);           // ±0.25m tolerance

// Servo and camera parameters
float adjacentLeft = 0.0;                        // Distance from the strip for Left reel
float adjacentRight = 0.0;                       // Distance from the strip for Right reel
float desired_angle = 0.0;                       // Servo angle
const int offset = round(0 * conversionFactor);  // Set offset to 0.75 for one reel, not needed for two reels so set to zero
float current_angle = 90.0;                      // Start with the servo centered
const float alpha = 0.05;                        // Easing factor (0.0 - 1.0), lower values = smoother

// Define position range for en garde detection
const int lowerBound = engardePosition - positionDelta;
const int upperBound = engardePosition + positionDelta;

void moveServo() {

  // If only one reel has valid data, use it for both
  if (adjacentLeft == 0 && adjacentRight != 0) {
    adjacentLeft = adjacentRight;
  } else if (adjacentRight == 0 && adjacentLeft != 0) {
    adjacentRight = adjacentLeft;
  }

  // Check if still no valid data
  if (adjacentLeft == 0 && adjacentRight == 0) {
    if (verbose) Serial.println("Warning: No valid data from either reel.");
    return;
  }

  // Calculate average position and desired angle
  float adjusted_positionLeft = incomingDataLeft.position + offset;
  float adjusted_positionRight = 364 - (incomingDataRight.position - offset);  // Flipped position for Right reel

  // Calculate desired angle for both reels
  float desired_angleLeft = 0.0;
  if (adjusted_positionLeft != 0) {
    desired_angleLeft = 90 - atan((strip_center - adjusted_positionLeft) / adjacentLeft) * (180.0 / M_PI);
  }

  float desired_angleRight = 0.0;
  if (adjusted_positionRight != 0) {
    desired_angleRight = 90 - atan((strip_center - adjusted_positionRight) / adjacentRight) * (180.0 / M_PI);
  }

  // Calculate average position and desired angle
  desired_angle = (desired_angleLeft + desired_angleRight) / 2.0;

  // Mirror the servo motion if invertServo is true
  if (invertServo) {
    desired_angle = 180 - desired_angle;  // Invert the desired angle
  }

  // Move the servo to the desired angle
  if (desired_angle >= 0 && desired_angle <= 180) {  // Ensure valid servo range
    current_angle = (alpha * desired_angle) + ((1 - alpha) * current_angle);
    myServo.write(current_angle);
    if (verbose) {
      Serial.print("Moving servo to smoothed angle: ");
      Serial.println(current_angle);
    }
  } else {
    if (verbose) {
      Serial.println("Warning: Desired angle out of range (0-180).");
    }
  }
}

void onDataRecv(uint8_t *mac, uint8_t *incomingDataBytes, uint8_t len) {
  struct_message incomingData;
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  if (incomingData.senderID == 1) {  // Left reel
    incomingDataLeft = incomingData;

    if (verbose) {
      Serial.println("Data received from Left reel:");
      Serial.print("Position: ");
      Serial.println(incomingDataLeft.position);
      Serial.print("Angle: ");
      Serial.println(incomingDataLeft.angle);
    }

    if (incomingDataLeft.angle != 0) {
      adjacentLeft = round(7.0 * conversionFactor) / tan(incomingDataLeft.angle * M_PI / 180.0);
    } else {
      adjacentLeft = 0.0;
    }
  }

  if (incomingData.senderID == 2) {  // Right reel
    incomingDataRight = incomingData;

    if (verbose) {
      Serial.println("Data received from Right reel:");
      Serial.print("Position: ");
      Serial.println(incomingDataRight.position);
      Serial.print("Angle: ");
      Serial.println(incomingDataRight.angle);
    }

    if (incomingDataRight.angle != 0) {
      adjacentRight = round(7.0 * conversionFactor) / tan(incomingDataRight.angle * M_PI / 180.0);
    } else {
      adjacentRight = 0.0;
    }
  }
}

void setup() {
  Serial.begin(115200);  // Initialize Serial Monitor

  incomingDataLeft.position = initial_positionLeft;
  incomingDataRight.position = initial_positionRight;

  // Initialize servo and set to a known middle position
  myServo.attach(servoPin, 500, 2500);
  myServo.write(90);  // Set servo to middle (90 degrees)
  delay(500);         // Give the servo time to reach position

  if (verbose) {
    Serial.println("Servo initialized to 90 degrees!");
  }

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

  if (verbose) {
    Serial.println("ESP-NOW Receiver Ready!");
  }
}

void loop() {
  // Call moveServo() regularly to update the servo
  moveServo();
  delay(10);  // Delay for a short period to prevent flooding
}
