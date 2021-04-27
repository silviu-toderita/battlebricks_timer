# Battlebricks Competition Timer

[![Battlebricks Timer Video](https://raw.githubusercontent.com/silviu-toderita/battlebricks_timer/main/docs/video_thumb.png)](https://youtu.be/ey022NEUoDs "Battlebricks Timer Overview")

Competition timer for a game of all-out remote-controlled LEGO battlebots destruction. Based on an ESP8266 WiFi-enabled Microcontroller, it features an LED matrix displays that faces the audience, and a smaller LED matrix display that faces the players. Both displays show a mix of text and graphics depending on the status of the game. 

## Features
- Adjust game time, number of players and brightness easily between rounds
- Supports 2 players, 3 players, and a free-for-all "rumble mode"
- Externad and on-screen light bars show player status
- Fully configurable over wifi on mobile-friendly settings page
- Built-in buzzer for audio feedback

## Progress
As of Apr 2021, the firmware is complete and has been tested on a prototype with all hardware components. Still to do:
- Build a case from clear acrylic sheets and LEGO to display and protect the Battlebricks Timer during competitions
- Finish and publish a parts list and schematic
- Create a setup guide so you can build your own Battlebricks Timer

Here are some photos of the current prototype:

<img src="https://raw.githubusercontent.com/silviu-toderita/battlebricks_timer/main/docs/proto_1.JPG" width="500"><img src="https://raw.githubusercontent.com/silviu-toderita/battlebricks_timer/main/docs/proto_2.JPG" width="500">


## Dependencies
- [adafruit/Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) 1.7.0
- [adafruit/Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) 1.10.4
- [adafruit/Adafruit NeoMatrix](https://github.com/adafruit/Adafruit_NeoMatrix) 1.2.0
- [adafruit/Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) 1.7.1
- [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson) 6.17.2
