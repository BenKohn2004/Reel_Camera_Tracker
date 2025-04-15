## Project Overview

The goal of this project is to automatically track fencers and is specifically designed to complement the Super Fencing System. A brief example of the reel tracker can be found on [YouTube](Youtube Link).

This is accomplished using optical quadrature rotary encoding and a Wemos Arduino to determine how much of a Favero fencing reel has been reeled out. This information is transmitted to another Wemos Arduino that controls a servo attached to a camera, which tracks the midpoint between two fencers.

---

## Reel Encoders

Reel encoding is handled using a [Wemos ESP8266 Arduino](https://www.aliexpress.us/item/3256806810666156.html) connected to two [TCRT5000 IR sensors](https://www.aliexpress.us/item/3256806126379687.html), all mounted on a custom PCB [Gerber File](https://github.com/BenKohn2004/Reel_Camera_Tracker/blob/main/Gerber_Favero_Optical_Encoder_PCB.zip). Reflective [tape](https://www.amazon.com/dp/B089RNX794) and [masking tape](https://www.amazon.com/dp/B0CDGCCKXX) are applied in 1/16 segments to the drum of the Favero reel, aligned with the reel’s rivets for simplified placement.

The IR sensors are mounted using a 3D printed [Favero Optical Mount](https://cad.onshape.com/documents/fab3dbb0c6cd24d122a26ac7/w/167803772a56f7a36cd09560/e/40a31ee80700d67aef5da61b?renderMode=0&uiState=67fe731ba1c8f971c51b1d21). The mount is attached using 10mm [M3 screws](https://www.aliexpress.us/item/2251832624537980.html), similar to those found in body cords. A 7/64" drill bit is used to create the hole in the bottom of the Favero reel for mounting. The mount’s front indentation is designed to align with a rivet on the reel.

The Wemos, [SSD1306 OLED display](https://www.aliexpress.us/item/3256806315309280.html), a 10kΩ resistor, and two buttons are combined on the PCB. The board is panelized (though roughly) and may need to be snapped apart. The SSD1306 display is optional and used to show the current position and angle sent to the camera servo. A matching 3D printed holder for the PCB is found in the same OnShape file and is designed to hold 4mm x 2mm magnets in its base with a drop of superglue.

Each reel (Left and Right) requires its own Wemos board. Power is supplied via USB, but a direct 5V connection may be more reliable long term. The Arduino code is the same for both reels except for this line:

```cpp
outgoingData.senderID = 1;  // ID of 1 is the Left Reel, ID of 2 is the Right Reel
```

The Left reel should use ID 1, and the Right reel should use ID 2. If using multiple systems, be sure to coordinate these IDs with the servo-side code in lines such as:

```cpp
if (incomingData.senderID == 1) {  // Left reel
if (incomingData.senderID == 2) {  // Right reel
```

Since the sender ID is an 8-bit unsigned integer, each reel must have a numeric ID. Only one Arduino sketch is needed for both reels; just update the `senderID` appropriately.

---

## IR Sensor Calibration

To calibrate each IR sensor:
1. Turn the potentiometer on the sensor fully counterclockwise (least sensitive).
2. Point the sensor at a white tape segment and slowly rotate the potentiometer clockwise until the onboard LED lights up.
3. Confirm the LED turns off when pointed at black tape.
4. Slowly rotate the reel and ensure the sensor detects transitions between white and black accurately.
5. You should see this pattern: (0,0); (0,1); (1,1); (1,0).

You may need to adjust tape placement or tighten the sensors to get clean readings.

---

## Servo and Camera

The Wemos controlling the camera servo delivers Power, Ground, and Data (on pin D4) to the servo. I used a [DS3218 high-torque servo](https://www.amazon.com/Miuzei-Torque-Digital-Waterproof-Control/dp/B07HNTKSZT) and a [UVC webcam](https://www.aliexpress.us/item/3256805987213806.html) with manual zoom and focus.

The servo is housed in a custom [OnShape DS3218 Enclosure](https://cad.onshape.com/documents/1e486539fdc99d72c7aaabdc/w/271820f546826b94c9b7a559/e/6343038745df72bbe1ef6208?renderMode=0&uiState=67fe7cface13531524bbc043).

I'm not offering detailed advice on how to mount the servo and camera—I used zip ties and spare parts, and I wouldn’t recommend my setup as a model of craftsmanship.

---

## Webcam & AirPad Connection

The webcam was connected to an iPad Air (4th Gen), as detailed in this [Reddit post](https://www.reddit.com/r/Fencing/comments/1c2v94j/inexpensive_tournament_video_replay_setup/).
