/**
 * BATTLEBRICKS TIMER
 * https://github.com/silviu-toderita/battlebricks_timer
 * 
 * Battlebricks Timer is an ESP8266-based countdown timer for "Battlebricks" (Lego Battlebots) 
 * competitions. 
 * 
 * Features: 
 *  -Audience-facing 32x16 LED matrix display (WS2812)
 *  -Player-facing 16x8 LED matrix display (WS2812)
 *  -Player ready lights for 2 players
 *  -Auxiliary player ready lights on LED displays supports up to 3 players
 *  -Buzzer for audio feedback
 *  -Wi-Fi and web settings page for full customization of all parameters
 *  -In-game settings can be adjusted on the fly (total time, number of players, screen brightness)
 * 
 * Setup:
 *  Full details at https://github.com/silviu-toderita/battlebricks_timer
 * 
 **/
#include "Arduino.h"

#include "graphics.h"

#include "Persistent_Storage.h"
#include "Button.h"
#include "ESP8266WiFi.h"
#include "ESP8266WiFiMulti.h"
#include "Web_Interface.h"
#include "Buzzer.h"

// Input/Output Pins
#define PIN_BTN_RED     14
#define PIN_BTN_BLACK   5
#define PIN_BTN_BLUE    4
#define PIN_BTN_GREEN   0
#define PIN_BUZZER      15

// States
#define STARTUP     0
#define STANDBY     1
#define PRE         2
#define COUNTDOWN   3
#define PAUSED      4
#define GAME_OVER   5

uint8_t state = STARTUP;

// Modes
#define TWO_PLAYER      0
#define THREE_PLAYER    1
#define RUMBLE          2

// In-game settings
uint16_t total_time;
uint8_t mode;

// Settings from settings file
String msg_intro;
String color_intro;
String color_pre;
String color_timer;
String msg_rumble;
String msg_get_ready;
String msg_game_over;
bool show_ready;
bool buzzer_on;

uint16_t min_time;
uint16_t max_time;
uint8_t interval_time;
uint8_t pre_time;
uint8_t go_time;
uint8_t game_over_time;
bool auto_reset;

// Game Status
int16_t time_remaining;
bool blue_ready = false;
bool red_ready = false;
bool green_ready = false;

