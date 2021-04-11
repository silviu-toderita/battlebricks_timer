#include "battlebots.h"
#include "graphics.h"

// Matrix Displays
Adafruit_NeoMatrix display_1 = Adafruit_NeoMatrix(16, 16, 2, 1, PIN_DISPLAY1, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix display_2 = Adafruit_NeoMatrix(8, 8, 2, 1, PIN_DISPLAY2, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG + NEO_TILE_RIGHT + NEO_GRB + NEO_KHZ800);

// Networking
ESP8266WiFiMulti wifimulti;
Web_Interface webinterface;

// Interrupts
Soft_ISR display_isr;
Soft_ISR buzzer_isr;

// In-game preferences
Persistent_Storage prefs("pref");

// Buttons
Button btn_black(PIN_BTN_BLACK, true);
Button btn_blue(PIN_BTN_BLUE, true);
Button btn_red(PIN_BTN_RED, true);
Button btn_green(PIN_BTN_GREEN, true);

void stop_beep() {
    digitalWrite(PIN_BUZZER, LOW);
}

void beep(uint16_t time) {
    digitalWrite(PIN_BUZZER, HIGH);
    buzzer_isr.set_timer(stop_beep, time);
}

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
            display_isr.trigger();
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
    if(input == "White") return WHITE;
    if(input == "Green") return GREEN;
    if(input == "Cyan") return CYAN;
    if(input == "Magenta") return MAGENTA;
    if(input == "Yellow") return YELLOW;
    return RED;
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
void standby();
void countdown_a();

void reset(){
    state = STANDBY;
    display_isr.remove();
    red_ready = false;
    blue_ready = false;
    green_ready = false;
    standby();
}

void post_game_over(){
    if(auto_reset) {
        reset();
    } else{
        text_static("0:00", color_timer);
    }
}

void game_over(){
    state = GAME_OVER;
    if(game_over_time > 0) {
        text_dynamic(msg_game_over,RED);
        display_isr.set_timer(post_game_over,game_over_time*1000);
    }else{
        post_game_over();
    }
}

void pause(){
    state = PAUSED;
    display_isr.remove();
    time_remaining = time_remaining - go_time + 1;
    if(time_remaining < 0) time_remaining = 0;
    text_dynamic("PAUSED", YELLOW);
}

void countdown_b(){
    text_static(format_time(time_remaining, false), color_timer);
    if(time_remaining <= 1) {
        display_isr.set_timer(game_over,500);
    } else {
        display_isr.set_timer(countdown_a,500);
    }
    
}

void countdown_a(){
    time_remaining--;
    text_static(format_time(time_remaining, true), color_timer);
    display_isr.set_timer(countdown_b,500);
}

// GO!
void pre_countdown_go(){
    state = COUNTDOWN;
    if(go_time > 0){
        text_static("GO!", GREEN);
        display_isr.set_timer(countdown_a,go_time*1000);
    }else{
        countdown_a();
    }
}

// 1...
void pre_countdown_1(){
    text_static("1",color_pre);
    display_isr.set_timer(pre_countdown_go,1000);

}

// 2...
void pre_countdown_2(){
    text_static("2",color_pre);
    display_isr.set_timer(pre_countdown_1,1000);

}

// 3...
void pre_countdown_3(){
    text_static("3",color_pre);
    display_isr.set_timer(pre_countdown_2,1000);
}

// Get ready message
void pre_countdown_msg(){
    state = PRE;
    if(pre_time > 0){
        text_dynamic(msg_get_ready,color_pre);
        display_isr.set_timer(pre_countdown_3,pre_time*1000);
    }else{
        pre_countdown_3();
    }
}

// All players are ready, set the time
void ready(){
    time_remaining = total_time - go_time + 1;
    pre_countdown_msg();
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
    display_isr.set_trigger(standby);
    
}

// Intro message
void intro(){
    text_dynamic(msg_intro, color_intro);
    display_isr.set_trigger(num_players);
}

// Check if enough players are ready to start the game
void check_players_ready(){
    if(three_players){
        if(blue_ready & green_ready & red_ready){
            state = PRE;
            if(show_ready){
                display_isr.set_trigger(ready);
            }else{
                ready();
            }
        }
    }else{
        if(blue_ready & red_ready){
            state = PRE;
            if(show_ready){
                display_isr.set_trigger(ready);
            }else{
                ready();
            }
        }
    }
}

// Handler for black button press
void black_btn_press(){
    uint32_t start_time = millis();
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
            while(btn_black.get()){
                if(millis() > start_time + 2000){
                    reset();
                    break;
                }
            }
            pre_countdown_msg();
            break;
        
        case GAME_OVER:
            reset();
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
                if(green_ready){
                    green_ready = false;
                    players_handle();
                } else {
                    green_ready = true;
                    players_handle();
                    if(show_ready){
                        text_dynamic("GREEN READY", GREEN);
                        display_isr.set_trigger(standby);
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
                display_isr.set_timer(standby,1000);
            }else{
                if(blue_ready){
                    blue_ready = false;
                    players_handle();
                } else {
                    blue_ready = true;
                    players_handle();
                    if(show_ready){
                        text_dynamic("BLUE READY", BLUE);
                        display_isr.set_trigger(standby);
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
                red_ready = false;
                blue_ready = false;
                green_ready = false;
                num_players();
            }else{
                if(red_ready){
                    red_ready = false;
                    players_handle();
                } else {
                    red_ready = true;
                    players_handle();
                    if(show_ready){
                        text_dynamic("RED READY", RED);
                        display_isr.set_trigger(standby);
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

    // General Settings
    msg_intro = webinterface.load_setting("msg_intro");

    color_intro = parse_color(webinterface.load_setting("color_intro"));

    color_pre = parse_color(webinterface.load_setting("color_pre"));

    color_timer = parse_color(webinterface.load_setting("color_timer"));

    show_aux_lights = webinterface.load_setting("show_aux_lights") == "true";

    if(webinterface.load_setting("show_dim_lights") == "false"){
        blue_dim = 0x0000;
        red_dim = 0x0000;
        green_dim = 0x0000;
    }

    show_ready = webinterface.load_setting("show_ready") == "true";

    msg_get_ready = webinterface.load_setting("msg_get_ready");

    msg_game_over = webinterface.load_setting("msg_game_over");


    // Advanced Settings
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

    String pre_time_string = webinterface.load_setting("pre_time");
    if(pre_time_string == "Off") pre_time = 0;
    else if(pre_time_string == "") pre_time = 5;
    else pre_time = pre_time_string.substring(0,1).toInt();

    String go_time_string = webinterface.load_setting("go_time");
    if(go_time_string == "Off") go_time = 0;
    else if(go_time_string == "") go_time = 2;
    else go_time = go_time_string.substring(0,1).toInt();

    String game_over_time_string = webinterface.load_setting("game_over_time");
    if(game_over_time_string == "Off") game_over_time = 0;
    else if(game_over_time_string == "") game_over_time = 5;
    else game_over_time = game_over_time_string.substring(0,1).toInt();

    auto_reset = webinterface.load_setting("auto_reset") == "true";


    // Prefs File
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

    three_players = (prefs.get("num_players") == "3");

}

// Wi-Fi Loop for settings update
void wifi_loop(){
    // Display a static wifi symbol on the displays
    display_1.clear();
    display_2.clear();
    display_1.setBrightness(32);
    display_2.setBrightness(24);
    display_1.drawBitmap(8,3,bmp_wifi_l,16,9,CYAN);
    display_2.drawBitmap(4,0,bmp_wifi_s,8,8,CYAN);
    display_1.show();
    display_2.show();

    // Start a hotspot
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(1,2,3,4),IPAddress(192,168,4,1),IPAddress(255,255,255,0));
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

    // Wifi Loop
    uint64_t button_pressed_time;
    bool button_pressed = false;
    bool restart = false;

    while(true){
        webinterface.handle();
        yield();
        
        if(button_pressed){
            button_pressed = !digitalRead(PIN_BTN_BLACK);
            // If black button is pressed for 3 seconds and let go, restart
            if(millis() >= button_pressed_time + 3000){
                restart = true;
            }
            // If black button is pressed for 10 seconds, factory reset
            if(millis() >= button_pressed_time + 10000){
                SPIFFS.remove("/settings.txt");
                ESP.restart();
            }
        }else if(!digitalRead(PIN_BTN_BLACK)){
            button_pressed = true;
            button_pressed_time = millis();
        }else if(digitalRead(PIN_BTN_BLACK)){
            if(restart) break;
        }
    }
}

// #############################################################################
//   SETUP
// #############################################################################
void setup(){
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

    // Initialize web interface
    webinterface.begin();

    // If black button held during start up, go to the wifi_loop
    if(!digitalRead(PIN_BTN_BLACK)){
        wifi_loop();
    }

    // Disable wifi and load settings
    WiFi.mode(WIFI_OFF);
    load_settings();

    // Set display brightness
    display_1.setBrightness(brightness * 10 + 10);
    display_2.setBrightness(brightness * 10);

    beep(500);
    
    // Begin the intro sequence
    if(msg_intro == ""){
        num_players();
    }else{
        intro();
    }
    
}   

// #############################################################################
//   LOOP
// #############################################################################
void loop(){
    // Handle any time-based interrupts
    display_isr.handle();
    buzzer_isr.handle();

    // Handle all button inputs
    btn_black.handle();
    btn_blue.handle();
    btn_red.handle();
    btn_green.handle();

    // Clear Displays
    display_1.clear();
    display_2.clear();

    // Draw text
    text_handle();
    if(!text_scroll){
        brightness_handle();
        
    }

    // Draw player ready bars
    if(!show_brightness && state != STARTUP) players_handle();

    // Refresh display
    display_1.show();
    display_2.show();

}