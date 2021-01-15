#include "battlebots.h"

// Matrix Display Objects
Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

// Networking objects
ESP8266WiFiMulti wifimulti;
Web_Interface webinterface;

// Interrupts Library
Soft_ISR soft_isr;

// Stores transient preferences
Persistent_Storage prefs("pref");

void text_handle(){
    if(text_scroll){
        text_xpos--;
    }

    int16_t text_length = text_string.length() * 8;
    if(text_xpos < -text_length){
        text_xpos = 32;
        soft_isr.trigger();
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

uint16_t parse_color(String input){
    if(input == "Blue") return BLUE;
    if(input == "Red") return RED;
    if(input == "Green") return GREEN;
    if(input == "Cyan") return CYAN;
    if(input == "Magenta") return MAGENTA;
    if(input == "Yellow") return YELLOW;
    return WHITE;
}

void ready(){
    text("1:30", parse_color(webinterface.load_setting("color_timer")), false);
}

void num_players(){
    state = READY;

    if(three_players) text("3 PLAYERS", GREEN, true);
    else text("2 PLAYERS", BLUE, true);
    soft_isr.set_trigger(ready);
    
}

void msg_intro(){
    text(webinterface.load_setting("msg_intro"), parse_color(webinterface.load_setting("color_intro")), true);
    soft_isr.set_trigger(num_players);
}

void wifi_loop(){
    display_1.clear();
    display_2.clear();
    display_1.setBrightness(32);
    display_2.setBrightness(24);
    display_1.fillRect(14, 11, 4, 2, CYAN);
    display_2.fillRect(7, 6, 2, 2, CYAN);
    display_1.show();
    display_2.show();
    delay(500);
    display_1.fillRect(14, 7, 4, 2, CYAN);
    display_1.fillRect(12, 8, 2, 2, CYAN);
    display_1.fillRect(18, 8, 2, 2, CYAN);
    display_2.fillRect(7, 3, 2, 2, CYAN);
    display_2.fillRect(5, 4, 2, 2, CYAN);
    display_2.fillRect(9, 4, 2, 2, CYAN);
    display_1.show();
    display_2.show();
    delay(500);
    display_1.fillRect(12, 3, 8, 2, CYAN);
    display_1.fillRect(10, 4, 2, 2, CYAN);
    display_1.fillRect(20, 4, 2, 2, CYAN);
    display_2.fillRect(6, 0, 4, 2, CYAN);
    display_2.fillRect(4, 1, 2, 2, CYAN);
    display_2.fillRect(10, 1, 2, 2, CYAN);
    display_1.show();
    display_2.show();

    WiFi.mode(WIFI_AP);

    String SSID = webinterface.load_setting("hotspot_SSID");
    if(SSID == ""){
        WiFi.softAP("battlebots","12345678");
    }else{
        String password = webinterface.load_setting("hotspot_password");
        if(password.length() < 8){
            WiFi.softAP(SSID);
        }else{
            WiFi.softAP(SSID, password);
        }
    }

    //Load local_URL and filter out prefix and suffix
    String local_URL = webinterface.load_setting("local_URL");
    if(local_URL.endsWith(".local")) local_URL = local_URL.substring(0,local_URL.length()-6);
    if(local_URL.startsWith("http://")) local_URL = local_URL.substring(7);
    if(local_URL.startsWith("https://")) local_URL = local_URL.substring(8);
    if(local_URL == "") local_URL = "battlebots";
    MDNS.begin(local_URL);

    uint64_t button_pressed_time;
    bool button_pressed = false;

    while(true){
        MDNS.update();
        webinterface.handle();
        yield();

        if(button_pressed){
            button_pressed = !digitalRead(PIN_BTN_BLACK);
            if(millis() >= button_pressed_time + 3000){
                break;
            }
        }else if(!digitalRead(PIN_BTN_BLACK)){
            button_pressed = true;
            button_pressed_time = millis();
        }
    }
}

void start(){

}


// #############################################################################
//   SETUP
// #############################################################################
void setup(){
    // Serial Monitor
    Serial.begin(115200);
    Serial.println("");
    
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

    display_1.setBrightness(32);
    display_2.setBrightness(32);

    webinterface.begin();

    if(!digitalRead(PIN_BTN_BLACK)){
        wifi_loop();
    }

    WiFi.mode(WIFI_OFF);

    three_players = (prefs.get("num_players") == "3");

    if(webinterface.load_setting("msg_intro") == ""){
        num_players();
    }else{
        msg_intro();
    }
    
}   

// #############################################################################
//   LOOP
// #############################################################################
void loop(){

    btn_black_down = !digitalRead(PIN_BTN_BLACK);
    btn_red_down = !digitalRead(PIN_BTN_RED);
    btn_blue_down = !digitalRead(PIN_BTN_BLUE);
    btn_green_down = !digitalRead(PIN_BTN_GREEN);

    soft_isr.handle();

    switch(state){
        case STARTUP:
            break;
        case READY:
            break;
        default:
            break;
    }

    // UpdateMatrix Displays
    display_1.clear();
    display_2.clear();

    text_handle();

    display_1.show();
    display_2.show();
}