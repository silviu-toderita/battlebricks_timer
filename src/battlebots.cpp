#include "battlebots.h"

// Matrix Display Objects
Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

// Networking objects
ESP8266WiFiMulti wifimulti;
Web_Interface webinterface;

// Interrupts Object
Soft_ISR soft_isr;

// Object for transient preferences
Persistent_Storage prefs("pref");

// Button objects
Button btn_black(PIN_BTN_BLACK, true);
Button btn_blue(PIN_BTN_BLUE, true);
Button btn_red(PIN_BTN_RED, true);
Button btn_green(PIN_BTN_GREEN, true);

// Handle drawing players ready lights
void players_handle(){
    if(three_players){
        display_1.drawRect(8,0,16,2,green_ready ? GREEN : green_dim);
        display_2.drawLine(4,0,11,0,green_ready ? GREEN : green_dim);
        if(show_aux_lights){
            display_1.drawRect(0,0,6,2,blue_ready ? BLUE : blue_dim);
            display_2.drawLine(13,0,15,0,blue_ready ? BLUE : blue_dim);
            display_1.drawRect(26,0,6,2,red_ready ? RED : red_dim);
            display_2.drawLine(0,0,2,0,red_ready ? RED : red_dim);
        }
    }else if(show_aux_lights){
        display_1.drawRect(0,0,14,2,blue_ready ? BLUE : blue_dim);
        display_2.drawLine(9,0,15,0,blue_ready ? BLUE : blue_dim);

        display_1.drawRect(18,0,14,2,red_ready ? RED : red_dim);
        display_2.drawLine(0,0,6,0,red_ready ? RED : red_dim);
    }

    if(blue_ready) digitalWrite(PIN_LED_BLUE, HIGH);
    else digitalWrite(PIN_LED_BLUE, LOW);

    if(red_ready) digitalWrite(PIN_LED_RED, HIGH);
    else digitalWrite(PIN_LED_RED, LOW);
}

void brightness_handle(){
    if(show_brightness){
        display_1.drawBitmap(1,2,bmp_brightness_l,16,13,WHITE);
        display_2.drawBitmap(0,0,bmp_brightness_s,8,8,WHITE);
    }
}

// Handle drawing text
void text_handle(){
    if(text_scroll){
        text_xpos--;

        int16_t text_length = text_string.length() * 8;
        if(text_xpos < -text_length){
            text_xpos = 32;
            soft_isr.trigger();
            return;
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

// Set new static text (centered)
void text_static(String text, uint16_t color){
    text_scroll = false;
    text_xpos = 15 - (text.length() * 3);
    text_color = color;
    text_string = text;
}

// Set new static text with custom X position
void text_static(String text, uint16_t color, uint8_t xpos){
    text_scroll = false;
    text_xpos = xpos;
    text_color = color;
    text_string = text;
}

// Set new dynamic (scrolling) text
void text_dynamic(String text, uint16_t color){
    text_xpos = 32;
    text_color = color;
    text_string = text;
    text_scroll = true;
}

// Parse color from string
uint16_t parse_color(String input){
    if(input == "Blue") return BLUE;
    if(input == "Red") return RED;
    if(input == "Green") return GREEN;
    if(input == "Cyan") return CYAN;
    if(input == "Magenta") return MAGENTA;
    if(input == "Yellow") return YELLOW;
    return WHITE;
}

// Format time correctly for a timer
String format_time(uint8_t time, bool colon){
    String minute = String(time / 60);
    String second = String(time % 60);
    if(second.length() == 1) second = "0" + second;
    if(colon) return minute + ":" + second;
    return minute + " " + second;
}

void ready();
void countdown_a();

void resume(){
    ready();
}

void pause(){
    state = PAUSED;
    soft_isr.remove();
    text_dynamic("PAUSED", YELLOW);
}

void countdown_b(){
    text_static(format_time(time_remaining, false), color_timer);
    soft_isr.set_timer(countdown_a,500);
}

void countdown_a(){
    time_remaining--;
    text_static(format_time(time_remaining, true), color_timer);
    soft_isr.set_timer(countdown_b,500);
}

void countdown_start(){
    if(state == COUNTDOWN) time_remaining = total_time - go_time + 1;
    else state = COUNTDOWN;
    countdown_a();
}

// GO!
void pre_countdown_go(){
    if(state == PRE) state = COUNTDOWN;
    if(go_time > 0){
        text_static("GO!", GREEN);
        soft_isr.set_timer(countdown_start,go_time*1000);
    }else{
        countdown_start();
    }
}

// 1...
void pre_countdown_1(){
    text_static("1",color_pre);
    soft_isr.set_timer(pre_countdown_go,1000);

}

// 2...
void pre_countdown_2(){
    text_static("2",color_pre);
    soft_isr.set_timer(pre_countdown_1,1000);

}

// 3...
void pre_countdown_3(){
    text_static("3",color_pre);
    soft_isr.set_timer(pre_countdown_2,1000);
}

// Get ready message
void ready(){
    if(state == STANDBY) state = PRE;
    if(pre_time > 0){
        text_dynamic("GET READY...",color_pre);
        soft_isr.set_timer(pre_countdown_3,pre_time*1000);
    }else{
        pre_countdown_3();
    }
}

// Standby timer display
void standby(){
    show_brightness = false;
    text_static(format_time(total_time, true), color_timer);
}

// Num players message
void num_players(){
    state = STANDBY;

    if(three_players) text_dynamic("3 PLAYERS", GREEN);
    else text_dynamic("2 PLAYERS", BLUE);
    soft_isr.set_trigger(standby);
    
}

// Intro message
void msg_intro(){
    text_dynamic(webinterface.load_setting("msg_intro"), parse_color(webinterface.load_setting("color_intro")));
    soft_isr.set_trigger(num_players);
}

void reset(){
    state = STANDBY;
    soft_isr.remove();
    red_ready = false;
    blue_ready = false;
    green_ready = false;
    standby();
}

// Check if enough players are ready to start the game
void check_players_ready(){
    if(three_players){
        if(blue_ready & green_ready & red_ready){
            if(show_ready){
                soft_isr.set_trigger(ready);
            }else{
                ready();
            }
        }
    }else{
        if(blue_ready & red_ready){
            if(show_ready){
                soft_isr.set_trigger(ready);
            }else{
                ready();
            }
        }
    }
}

// Handler for black button press
void black_btn_press(){
    switch(state){
        case STARTUP:
            num_players();
            break;

        case STANDBY:
            show_brightness = false;
            standby();
            break;
        
        case PRE:
            reset();
            break;

        case COUNTDOWN:
            pause();
            break;

        case PAUSED:
            resume();
            break;

        default: break;
    }
}

// Handler for green button press
void green_btn_press(){
    switch(state){
        case STARTUP: break;

        case STANDBY:
            if(btn_black.get()){
                if(total_time + interval_time > max_time){
                    total_time = min_time;
                }else{
                    total_time = total_time + interval_time;
                }
                prefs.set("total_time", String(total_time));
                standby();
            }else if(three_players){
                if(green_ready) green_ready = false;
                else {
                    green_ready = true;
                    if(show_ready){
                        text_dynamic("GREEN STANDBY", GREEN);
                        soft_isr.set_trigger(standby);
                    }
                    check_players_ready();
                }
            }
            break;
        
        default:
            break;
    }
}

// Handler for blue button press
void blue_btn_press(){
    switch(state){
        case STARTUP: break;

        case STANDBY:
            if(btn_black.get()){
                if(brightness == 8){
                    brightness = 1;
                }else{
                    brightness++;
                }
                display_1.setBrightness(brightness*10+10);
                display_2.setBrightness(brightness*10);
                prefs.set("brightness",String(brightness));
                text_static(String(brightness),WHITE,20);
                show_brightness = true;
                soft_isr.set_timer(standby,1000);
            }else{
                if(blue_ready) blue_ready = false;
                else {
                    blue_ready = true;
                    if(show_ready){
                        text_dynamic("BLUE STANDBY", BLUE);
                        soft_isr.set_trigger(standby);
                    }
                    check_players_ready();
                }
            }
            break;
        
        default:
            break;
    }
}

// Handler for red button press
void red_btn_press(){
    switch(state){
        case STARTUP: break;

        case STANDBY:
            if(btn_black.get()){
                if(three_players){
                        three_players = false;
                        green_ready = false;
                        prefs.set("num_players", "2");
                    }else{
                        three_players = true;
                        prefs.set("num_players", "3");
                    }
                num_players();
            }else{
                if(red_ready) red_ready = false;
                else {
                    red_ready = true;
                    if(show_ready){
                        text_dynamic("RED STANDBY", RED);
                        soft_isr.set_trigger(standby);
                    }
                    check_players_ready();
                }
            }
            break;
        
        default:
            break;
    }
}

// Load all settings 
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

    String show_aux_lights_string = webinterface.load_setting("show_aux_lights");
    show_aux_lights = show_aux_lights_string == "trueselected" || show_aux_lights_string == "true";
    String show_ready_string = webinterface.load_setting("show_ready");
    show_ready = show_ready_string == "trueselected" || show_ready_string == "true";
    String show_dim_lights_string = webinterface.load_setting("show_dim_lights");
    if(show_dim_lights_string == "falseselected" || show_dim_lights_string == "false"){
        blue_dim = 0x0000;
        red_dim = 0x0000;
        green_dim = 0x0000;
    }

    String pre_time_string = webinterface.load_setting("pre_time");
    if(pre_time_string == "Off") pre_time = 0;
    else pre_time = pre_time_string.substring(0,1).toInt();

    String go_time_string = webinterface.load_setting("go_time");
    if(go_time_string == "Off") go_time = 0;
    else go_time = go_time_string.substring(0,1).toInt();

    color_timer = parse_color(webinterface.load_setting("color_timer"));
    color_pre = parse_color(webinterface.load_setting("color_pre"));

}

// Wi-Fi Loop for settings update
void wifi_loop(){
    display_1.clear();
    display_2.clear();
    display_1.setBrightness(32);
    display_2.setBrightness(24);
    display_1.drawBitmap(8,3,bmp_wifi_l,16,9,CYAN);
    display_2.drawBitmap(4,0,bmp_wifi_s,8,8,CYAN);
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

    display_1.setBrightness(brightness * 10 + 10);
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

    display_1.clear();
    display_2.clear();
    text_handle();
    if(!text_scroll){
        brightness_handle();
        if(!show_brightness) players_handle();
    }
    display_1.show();
    display_2.show();

}