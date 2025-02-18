#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <math.h> // Include for trigonometric functions

// Pin definitions for encoder and buttons
const int pinA = D5; // Encoder pin A
const int pinB = D6; // Encoder pin B
const int buttonPin1 = D7; // Button 1 pin (reset position to 0)
const int buttonPin2 = D0; // Button 2 pin (save position to hypotenuse)

// Variables to track state and position
volatile int lastA = 0; // Previous state of pin A
volatile int lastB = 0; // Previous state of pin B
volatile int position = 0; // Position counter

// For debugging transitions
volatile int transitionA = -1; // Stores transition state for A
volatile int transitionB = -1; // Stores transition state for B
volatile bool hasTransition = false; // Indicates a new transition

// Hypotenuse and angle
int hypotenuse = 0; // Stores the saved position
float angle = 0.0;  // Stores the calculated angle

// OLED display setup
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Debounce variables for button presses
unsigned long lastButtonPress1 = 0;
unsigned long lastButtonPress2 = 0;
const unsigned long debounceDelay = 200; // 200ms debounce time

// ESPNOW variables
uint8_t receiverMac[] = {0xA4, 0xCF, 0x12, 0xF3, 0x04, 0x42}; // MAC address of the receiving device

// Structure to hold data to send
typedef struct struct_message {
    int position;
    float angle;
} struct_message;

struct_message outgoingData; // Data to send

void ICACHE_RAM_ATTR handleInterrupt() {
    // Read current state
    int currentA = digitalRead(pinA);
    int currentB = digitalRead(pinB);

    // Determine direction of rotation
    if (lastA == 0 && lastB == 0) {
        if (currentA == 0 && currentB == 1) {
            position++; // Clockwise
        } else if (currentA == 1 && currentB == 0) {
            position--; // Counterclockwise
        }
    } else if (lastA == 0 && lastB == 1) {
        if (currentA == 1 && currentB == 1) {
            position++; // Clockwise
        } else if (currentA == 0 && currentB == 0) {
            position--; // Counterclockwise
        }
    } else if (lastA == 1 && lastB == 1) {
        if (currentA == 1 && currentB == 0) {
            position++; // Clockwise
        } else if (currentA == 0 && currentB == 1) {
            position--; // Counterclockwise
        }
    } else if (lastA == 1 && lastB == 0) {
        if (currentA == 0 && currentB == 0) {
            position++; // Clockwise
        } else if (currentA == 1 && currentB == 1) {
            position--; // Counterclockwise
        }
    }

    // Update transition states
    transitionA = currentA;
    transitionB = currentB;
    hasTransition = true; // Mark new transition available

    // Update last state
    lastA = currentA;
    lastB = currentB;
}

void onDataSent(uint8_t *macAddr, uint8_t sendStatus) {
    Serial.print("Send status: ");
    Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Initialize pins
    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(buttonPin1, INPUT_PULLUP); // Button 1 (reset to 0)
    pinMode(buttonPin2, INPUT_PULLUP); // Button 2 (save position to hypotenuse)

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true); // Stop if initialization fails
    }
    display.display();  // Clear buffer
    delay(2000);  // Pause for 2 seconds to display initial screen

    // Read initial state of encoder
    lastA = digitalRead(pinA);
    lastB = digitalRead(pinB);

    // Attach interrupts
    attachInterrupt(digitalPinToInterrupt(pinA), handleInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinB), handleInterrupt, CHANGE);

    // Initialize ESPNOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer(receiverMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    esp_now_register_send_cb(onDataSent);

    Serial.println("Rotary Encoder Test with ESPNOW");
}

void loop() {
    static int lastPrintedPosition = 0;

    // Check if Button 1 was pressed (debounced) -> reset position to 0
    if (digitalRead(buttonPin1) == LOW) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastButtonPress1 > debounceDelay) {
            position = 0;  // Reset the position to 0
            lastButtonPress1 = currentMillis;  // Update last button press time
            Serial.println("Position reset to 0");
        }
    }

    // Check if Button 2 was pressed (debounced) -> save position to hypotenuse and calculate angle
    if (digitalRead(buttonPin2) == LOW) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastButtonPress2 > debounceDelay) {
            hypotenuse = position;  // Save the current position as hypotenuse
            lastButtonPress2 = currentMillis;  // Update last button press time

            // Calculate angle, with error handling for zero hypotenuse
            if (hypotenuse != 0) {
                angle = asin(100.0 / hypotenuse) * (180.0 / M_PI); // Convert to degrees
                Serial.print("Hypotenuse: ");
                Serial.println(hypotenuse);
                Serial.print("Angle: ");
                Serial.println(angle);
            } else {
                Serial.println("Error: Hypotenuse is zero. Cannot calculate angle.");
            }
        }
    }

    // Check for a new transition
    if (hasTransition) {
        noInterrupts(); // Temporarily disable interrupts
        int a = transitionA;
        int b = transitionB;
        hasTransition = false; // Reset flag
        interrupts(); // Re-enable interrupts
    }

    // Send data using ESPNOW
    outgoingData.position = position;
    outgoingData.angle = angle;
    esp_now_send(receiverMac, (uint8_t *)&outgoingData, sizeof(outgoingData));

    // Update OLED display with position, hypotenuse, and angle
    if (position != lastPrintedPosition) {
        display.clearDisplay();  // Clear display
        display.setTextSize(1);  // Set text size
        display.setTextColor(SSD1306_WHITE);  // Set text color
        display.setCursor(0, 0);  // Set cursor position
        display.print("Position: ");
        display.println(position);  // Display position
        display.print("Hypotenuse: ");
        display.println(hypotenuse);  // Display hypotenuse
        display.print("Angle: ");
        if (hypotenuse != 0) {
            display.println(angle);  // Display angle
        } else {
            display.println("ERR"); // Display error if hypotenuse is zero
        }
        display.display();  // Update display
        lastPrintedPosition = position;
    }

    // Small delay for readability
    delay(100);
}
