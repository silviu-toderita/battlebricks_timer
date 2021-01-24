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

// Button objects
Button btn_black(PIN_BTN_BLACK, true);
Button btn_blue(PIN_BTN_BLUE, true);
Button btn_red(PIN_BTN_RED, true);
Button btn_green(PIN_BTN_GREEN, true);

void text_handle_scroll(){
    if(text_scroll){
        text_xpos--;

        int16_t text_length = text_string.length() * 8;
        if(text_xpos < -text_length){
            text_xpos = 32;
            soft_isr.trigger();
            return;
        }

        display_1.clear();
        display_2.clear();

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

        display_1.show();
        display_2.show();
    }
    
}

void text_static(String text, uint16_t color){
    Serial.println("Printing Static Text: " + text);
    text_scroll = false;

    display_1.clear();
    display_2.clear();

    display_1.setCursor((15 - (text.length() * 3)), 12);
    display_2.setCursor((15 - (text.length() * 3))/2, 6);

    display_1.setFont(&Picopixel);
    display_2.setFont(&Picopixel);
    display_1.setTextSize(2);
    display_2.setTextSize(1);
    display_1.setTextWrap(false);
    display_2.setTextWrap(false);

    display_1.setTextColor(color);
    display_2.setTextColor(color);
    display_1.print(text);
    display_2.print(text);

    display_1.show();
    display_2.show();

}

void text_dynamic(String text, uint16_t color){
    Serial.println("Printing Dynamic Text: " + text);
    text_xpos = 32;
    text_color = color;
    text_string = text;
    text_scroll = true;
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

String format_time(uint8_t time){
    String minute = String(time / 60);
    String second = String(time % 60);
    if(second.length() == 1) second = "0" + second;
    return minute + ":" + second;
}

void ready(){
    text_static(format_time(total_time), parse_color(webinterface.load_setting("color_timer")));
}

void show_brightness(){
    display_1.setBrightness(brightness*10);
    display_2.setBrightness(brightness*10);
    display_1.clear();
    display_2.clear();
    display_1.drawCircle(8, 8, 3, WHITE);
    display_1.drawLine(8, 3, 8, 2, WHITE);
    display_1.drawLine(8, 13, 8, 14, WHITE);
    display_1.drawLine(3, 8, 2, 8, WHITE);
    display_1.drawLine(13, 8, 14, 8, WHITE);
    display_1.drawLine(12, 12, 13, 13, WHITE);
    display_1.drawLine(4, 4, 3, 3, WHITE);
    display_1.drawLine(12, 4, 13, 3, WHITE);
    display_1.drawLine(4, 12, 3, 13, WHITE);

    display_2.drawLine(1, 1, 0, 0, WHITE);
    display_2.drawLine(1, 6, 0, 7, WHITE);
    display_2.drawLine(6, 1, 7, 0, WHITE);
    display_2.drawLine(6, 6, 7, 7, WHITE);
    display_2.drawRect(3, 0, 2, 3, WHITE);
    display_2.drawRect(3, 5, 2, 3, WHITE);
    display_2.drawRect(0, 3, 3, 2, WHITE);
    display_2.drawRect(5, 3, 3, 2, WHITE);
    display_2.drawRect(2, 2, 4, 4, WHITE);
    display_2.drawPixel(0, 0, WHITE);
    display_2.drawPixel(0, 7, WHITE);
    display_2.drawPixel(7, 0, WHITE);
    display_2.drawPixel(7, 7, WHITE);

    display_1.setCursor(21,12);
    display_2.setCursor(11,6);

    display_1.setFont(&Picopixel);
    display_2.setFont(&Picopixel);
    display_1.setTextSize(2);
    display_2.setTextSize(1);
    display_1.setTextWrap(false);
    display_2.setTextWrap(false);

    display_1.setTextColor(WHITE);
    display_2.setTextColor(WHITE);
    display_1.print(String(brightness));
    display_2.print(String(brightness));

    display_1.show();
    display_2.show();

    soft_isr.set_timer(ready,1000);

}

void num_players(){
    state = READY;

    if(three_players) text_dynamic("3 PLAYERS", GREEN);
    else text_dynamic("2 PLAYERS", BLUE);
    soft_isr.set_trigger(ready);
    
}

void msg_intro(){
    text_dynamic(webinterface.load_setting("msg_intro"), parse_color(webinterface.load_setting("color_intro")));
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

void load_settings(){
    String min_time_string = webinterface.load_setting("min_time");
    if(min_time_string == "0:15"){
        min_time = 15;
    }else if(min_time_string == "0:45"){
        min_time = 45;
    }else if(min_time_string == "1:00"){
        min_time = 60;
    }else if(min_time_string == "1:30"){
        min_time = 90;
    }else{
        min_time = 30;
    }

    String max_time_string = webinterface.load_setting("max_time");
    if(max_time_string == "2:00"){
        max_time = 120;
    }else if(max_time_string == "4:00"){
        max_time = 240;
    }else if(max_time_string == "5:00"){
        max_time = 300;
    }else{
        max_time = 180;
    }

    String interval_time_string = webinterface.load_setting("interval_time");
    if(interval_time_string == "0:01"){
        interval_time = 1;
    }else if(interval_time_string == "0:02"){
        interval_time = 2;
    }else if(interval_time_string == "0:05"){
        interval_time = 5;
    }else if(interval_time_string == "0:10"){
        interval_time = 10;
    }else if(interval_time_string == "0:30"){
        interval_time = 30;
    }else{
        interval_time = 15;
    }

    three_players = (prefs.get("num_players") == "3");

    String total_time_string = (prefs.get("total_time"));
    if(total_time_string == ""){
        total_time = 90;
    }else{
        total_time = total_time_string.toInt();
    } 
    if(total_time > max_time) total_time = max_time;
    if(total_time < min_time) total_time = min_time;

    String brightness_string = prefs.get("brightness");
    if(brightness_string == ""){
        brightness = 2;
    }else{
        brightness = brightness_string.toInt();
        if(brightness > 8) brightness = 8;
        if(brightness < 1) brightness = 1;
    }
}

void black_btn_press(){
    Serial.println("Black button pressed!");
    switch(state){
        case STARTUP:
            num_players();
            break;

        case READY:
            ready();
            break;
        
        default: break;
    }
}

void green_btn_press(){
    switch(state){
        case STARTUP: break;

        case READY:
            if(btn_black.get()){
                if(total_time + interval_time > max_time){
                    total_time = min_time;
                }else{
                    total_time = total_time + interval_time;
                }
                prefs.set("num_players", String(total_time));
                ready();
            }else{

            }
            break;
        
        default:
            break;
    }
}

void blue_btn_press(){
    switch(state){
        case STARTUP: break;

        case READY:
            if(btn_black.get()){
                if(brightness == 8){
                    brightness = 1;
                }else{
                    brightness++;
                }
                prefs.set("brightness",String(brightness));
                show_brightness();
            }else{

            }
            break;
        
        default:
            break;
    }
}

void red_btn_press(){
    switch(state){
        case STARTUP: break;

        case READY:
            if(btn_black.get()){
                if(three_players){
                        three_players = false;
                        prefs.set("num_players", "2");
                    }else{
                        three_players = true;
                        prefs.set("num_players", "3");
                    }
                num_players();
            }else{

            }
            break;
        
        default:
            break;
    }
}

// #############################################################################
//   SETUP
// #############################################################################
void setup(){
    // Serial Monitor
    Serial.begin(115200);
    Serial.println("");
    
    // Initialize GPIO
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LED_BLUE, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(PIN_BUZZER, LOW);

    // Set button callbacks
    btn_black.set_posedge_cb(black_btn_press);
    btn_blue.set_posedge_cb(blue_btn_press);
    btn_green.set_posedge_cb(green_btn_press);
    btn_red.set_posedge_cb(red_btn_press);
  
    // Initialize Matrix displays
    display_1.begin();
    display_2.begin();

    webinterface.begin();

    if(!digitalRead(PIN_BTN_BLACK)){
        wifi_loop();
    }

    WiFi.mode(WIFI_OFF);

    load_settings();

    display_1.setBrightness(brightness * 10);
    display_2.setBrightness(brightness * 10);

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
    soft_isr.handle();
    btn_black.handle();
    btn_blue.handle();
    btn_red.handle();
    btn_green.handle();

    text_handle_scroll();

}