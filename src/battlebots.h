#include "Arduino.h"

// Silviu's Libraries
#include "Soft_ISR.h"
#include "Persistent_Storage.h"
#include "Button.h"

// Graphics Libraries 
#include "Adafruit_GFX.h"
#include "Adafruit_NeoPixel.h"
#include "Adafruit_NeoMatrix.h"
#include "Picopixel.h"

// Networking Libraries
#include "ESP8266WiFi.h"
#include "ESP8266WiFiMulti.h"
#include "ESP8266mDNS.h"
#include "Web_Interface.h"

// Pin Definitions
#define PIN_DISPLAY1 2
#define PIN_DISPLAY2 10

#define PIN_LED_RED 13
#define PIN_LED_BLUE 12

#define PIN_BUZZER 15

#define PIN_BTN_RED 14
#define PIN_BTN_BLACK 5
#define PIN_BTN_BLUE 4
#define PIN_BTN_GREEN 0

// Color Definitions
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0 
#define WHITE       0xFFFF

uint16_t blue_dim = 0x0011;
uint16_t red_dim = 0x8000;
uint16_t green_dim = 0x0300;

// States
#define STARTUP     0
#define READY       1
#define RUNNING     2
#define OVER        3

uint8_t state = STARTUP;

// Current text on screen
int16_t text_xpos = 0;
bool text_scroll = false;
uint16_t text_color = 0;
String text_string = "";

// Current graphics on screen
bool show_brightness = false;

// Game prefs
uint16_t total_time = 90;
uint16_t min_time = 30;
uint16_t max_time = 180;
uint8_t interval_time = 15;

// Global prefs
uint8_t brightness = 2;
bool show_aux_lights = true;
bool show_ready = true;
uint8_t pre_time = 5;
uint8_t go_time = 2;

// Players
bool three_players = false;
bool blue_ready = false;
bool red_ready = false;
bool green_ready = false;

// Graphics
const PROGMEM uint8_t bmp_wifi_l[] = {  0x0F, 0xF0, 0x3F, 0xFC, 0x30, 0x0C, 0x03, 0xC0, 
                                        0x0F, 0xF0, 0x0C, 0x30, 0x00, 0x00, 0x03, 0xC0, 
                                        0x03, 0xC0}; 

const PROGMEM uint8_t bmp_wifi_s[] = {  0x3C, 0xFF, 0xC3, 0x18, 0x7E, 0x66, 0x18, 0x18}; 

const PROGMEM uint8_t bmp_brightness_l[] = {0x01, 0x00, 0x21, 0x08, 0x10, 0x10, 0x03, 0x80, 
                                            0x04, 0x40, 0x08, 0x20, 0x68, 0x2C, 0x08, 0x20, 
                                            0x04, 0x40, 0x03, 0x80, 0x10, 0x10, 0x21, 0x08,
                                            0x01, 0x00}; 

const PROGMEM uint8_t bmp_brightness_s[] = {0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99}; 