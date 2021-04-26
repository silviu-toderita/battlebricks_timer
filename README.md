# Battlebricks Competition Timer

Competition timer for a game of all-out remote-controlled LEGO battlebots destruction. Based on an ESP8266 WiFi-enabled Microcontroller, it features an LED matrix displays that faces the audience, and a smaller LED matrix display that faces the players. Both displays can show numbers, text and graphics. 

## Features
- Adjust game time, number of players and brightness easily between rounds
- Supports 2 players, 3 players, and a free-for-all "rumble mode"
- External player ready lights for Red and Blue players, and auxilliary on-screen lights for Red, Blue and Green players
- Fully configurable over wifi on mobile-friendly settings page
- Built-in buzzer for audio feedback

## Progress
As of Apr 2021, the firmware is complete and has been tested on a prototype with all hardware components. The next step will be building a case for the timer out of clear acrylic sheets and lego so that it can be used in Battlebricks competitions this summer. Here are some shots of the prototype:

<img src="https://raw.githubusercontent.com/silviu-toderita/battlebricks_timer/main/docs/proto_1.JPG" width="500"><img src="https://raw.githubusercontent.com/silviu-toderita/battlebricks_timer/main/docs/proto_2.JPG" width="500">


## Dependencies
- [adafruit/Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) 1.7.0
- [adafruit/Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) 1.10.4
- [adafruit/Adafruit NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix) 1.2.0
- [adafruit/Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) 1.7.1
- [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson) 6.17.2
