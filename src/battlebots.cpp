#include "battlebots.h"

// Matrix Display Objects
Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

// Networking objects
ESP8266WiFiMulti wifimulti;


void do_time(uint32_t do_in_millis, void_function_pointer callback){
    do_time_en = true;
    do_scroll_en = false;
    do_at_millis = millis() + do_in_millis;
    do_handler = callback;
}

void do_scroll(void_function_pointer callback){
    do_scroll_en = true;
    do_time_en = false;
    do_handler = callback;
}

void do_handle(){
    if(do_time_en && millis() >= do_at_millis){
        do_time_en = false;
        do_handler();
    }
}

void text_handle(){
    if(text_scroll){
        text_xpos--;
    }

    int16_t text_length = text_string.length() * 8;
    if(text_xpos < -text_length){
        text_xpos = 32;

        if(do_scroll_en){
            do_scroll_en = false;
            do_handler();
        }
    }

    display_1.setCursor(text_xpos, 12);
    display_2.setCursor(text_xpos/2, 6);

    display_1.setFont(&Picopixel);
    display_2.setFont(&Picopixel);
    display_1.setTextSize(2);
    display_2.setTextSize(1);
    display_1.setTextWrap(false);
    display_2.setTextWrap(false);

    display_1.setTextColor(text_color);
    display_2.setTextColor(text_color);
    display_1.print(text_string);
    display_2.print(text_string);
    
}

void text(String text, uint16_t color, bool scroll){
    text_xpos = scroll? 32 : (15 - (text.length() * 3));
    text_color = color;
    text_string = text;
    text_scroll = scroll;
}

void ready(){
    text("1:30", RED, false);
}

void msg_hotspot(){
    connecting = false;
    WiFi.mode(WIFI_AP);
    WiFi.softAP("battlebots_timer","12345678");
    text("HOTSPOT MODE", CYAN, true);
    do_scroll(ready);
    MDNS.begin("battlebots");
}

void msg_connecting(){
    text("WIFI CONNECTING...", YELLOW, true);
    do_time(10000, msg_hotspot);
    connecting = true;
}

void msg_intro(){
    text("LEGO BATTLEBOTS", WHITE, true);
    do_scroll(msg_connecting);
}

// #############################################################################
//   SETUP
// #############################################################################
void setup(){
    // Serial Monitor
    Serial.begin(115200);
    
    // Initialize GPIO
    pinMode(PIN_BTN_BLACK, INPUT_PULLUP);
    pinMode(PIN_BTN_BLUE, INPUT_PULLUP);
    pinMode(PIN_BTN_GREEN, INPUT_PULLUP);
    pinMode(PIN_BTN_RED, INPUT_PULLUP);

    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LED_BLUE, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(PIN_BUZZER, LOW);

    // Initialize Matrix displays
    display_1.begin();
    display_2.begin();

    display_1.setBrightness(64);
    display_2.setBrightness(64);

    WiFi.mode(WIFI_STA);
    wifimulti.addAP("Azaviu", "sebastian");

    msg_intro();
    
}   

// #############################################################################
//   LOOP
// #############################################################################
void loop(){
    display_1.clear();
    display_2.clear();

    if(connecting){
        if(wifimulti.run() == WL_CONNECTED){
            connecting = false;
            text("WIFI CONNECTED: " + WiFi.SSID(), BLUE, true);
            do_scroll(ready);
            MDNS.begin("battlebots");
        }
    }else{
        MDNS.update();
    }

    text_handle();
    do_handle();

    display_1.show();
    display_2.show();
}