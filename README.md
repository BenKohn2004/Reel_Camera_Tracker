# Reel_Camera_Tracker

Project Overview
The goal of this project is to automatically track fencers and is specifically designed to complement the Super Fencing System. A brief example of the system can be found on [YouTube](Youtube Link).

This is accomplished using optical quadrature rotary encoding and a Wemos Arduino to determine how much of a Favero fencing reel has been reeled out. This information is transmitted to another Wemos Arduino that controls a servo attached to a camera, which tracks the midpoint between two fencers.

Reel Encoders
Reel encoding is handled using a Wemos ESP8266 Arduino connected to two TCRT5000 IR sensors, all mounted on a custom PCB ([Gerber File]). Reflective tape and masking tape are applied in 1/16 segments to the drum of the Favero reel, aligned with the reel’s rivets for simplified placement.

The IR sensors are mounted using a 3D printed Favero Optical Mount. The mount is attached using 10mm M3 screws, similar to those found in body cords. A 7/64" drill bit is used to create the hole in the bottom of the Favero reel for mounting. The mount’s front indentation is designed to align with a rivet on the reel.

The Wemos, SSD1306 OLED display, a 10kΩ resistor, and two buttons are combined on the PCB. The board is panelized (though roughly) and may need to be snapped apart. The SSD1306 display is optional and used to show the current position and angle sent to the camera servo. A matching 3D printed holder for the PCB is found in the same OnShape file and is designed to hold 4mm x 2mm magnets in its base with a drop of superglue.

Each reel (Left and Right) requires its own Wemos board. Power is supplied via USB, but a direct 5V connection may be more reliable long term. The Arduino code is the same for both reels except for this line:

cpp
Copy
Edit
outgoingData.senderID = 1;  // ID of 1 is the Left Reel, ID of 2 is the Right Reel
The Left reel should use ID 1, and the Right reel should use ID 2. If using multiple systems, be sure to coordinate these IDs with the servo-side code in lines such as:

cpp
Copy
Edit
if (incomingData.senderID == 1) {  // Left reel
if (incomingData.senderID == 2) {  // Right reel
Since the sender ID is an 8-bit unsigned integer, each reel must have a numeric ID. Only one Arduino sketch is needed for both reels; just update the senderID appropriately.

IR Sensor Calibration
To calibrate each IR sensor:

Turn the potentiometer on the sensor fully counterclockwise (least sensitive).

Point the sensor at a white tape segment and slowly rotate the potentiometer clockwise until the onboard LED lights up.

Confirm the LED turns off when pointed at black tape.

Slowly rotate the reel and ensure the sensor detects transitions between white and black accurately.

You should see this pattern: (0,0); (0,1); (1,1); (1,0).

You may need to adjust tape placement or tighten the sensors to get clean readings.

Servo and Camera
The Wemos controlling the camera servo delivers Power, Ground, and Data (on pin D4) to the servo. I used a DS3218 high-torque servo and a UVC webcam with manual zoom and focus.

The servo is housed in a custom OnShape DS3218 Enclosure.

I'm not offering detailed advice on how to mount the servo and camera—I used zip ties and spare parts, and I wouldn’t recommend my setup as a model of craftsmanship.

Webcam & AirPad Connection
The webcam was connected to an iPad Air (4th Gen), as detailed in this Reddit post.
