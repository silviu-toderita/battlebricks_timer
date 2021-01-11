#include "battlebots.h"

// Matrix Display Objects
Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

int16_t text_xpos = 0;
bool text_scroll = false;
uint16_t text_color = 0;
String text_string = "";

void handle_text(){
    if(text_scroll){
        text_xpos--;
    }

    int16_t text_length = text_string.length() * 8;
    if(text_xpos < -text_length){
        text_xpos = 32;
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
    text_xpos = scroll? 32 : (16 - (text.length() * 3));
    text_color = color;
    text_string = text;
    text_scroll = scroll;
}

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

    text("GO!", YELLOW, false);

}   

void loop(){
    display_1.clear();
    display_2.clear();

    handle_text();

    display_1.show();
    display_2.show();
}