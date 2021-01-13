#include "Arduino.h"

// Graphics Libraries 
#include "Adafruit_GFX.h"
#include "Adafruit_NeoPixel.h"
#include "Adafruit_NeoMatrix.h"
#include "Picopixel.h"

// Networking Libraries
#include "ESP8266WiFiMulti.h"
#include "ESP8266mDNS.h"

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
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

typedef void (*void_function_pointer)();

// Holds do_handle values
bool do_scroll_en = false;
bool do_time_en = false;
uint32_t do_at_millis;
static void_function_pointer do_handler;

// Wifi Conneting status
bool connecting = false;

// Current text on screen
int16_t text_xpos = 0;
bool text_scroll = false;
uint16_t text_color = 0;
String text_string = "";