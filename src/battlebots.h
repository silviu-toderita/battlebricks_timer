#include "Arduino.h"

#include "graphics.h"

// Silviu's Libraries
#include "Persistent_Storage.h"
#include "Button.h"

// Networking Libraries
#include "ESP8266WiFi.h"
#include "ESP8266WiFiMulti.h"
#include "Web_Interface.h"

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

// Game prefs - prefs file
uint16_t total_time;
bool three_players;

// Game prefs - settings file
String msg_intro;
String color_intro;
String color_pre;
String color_timer;
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

// In-game attributes
int16_t time_remaining;
bool blue_ready = false;
bool red_ready = false;
bool green_ready = false;

