#include "battlebots.h"

// Networking
ESP8266WiFiMulti wifimulti;
Web_Interface webinterface;

// Interrupts
Soft_ISR state_isr;
Soft_ISR buzzer_isr;

// In-game preferences
Persistent_Storage prefs("pref");

// Buttons
Button btn_black(PIN_BTN_BLACK, true);
Button btn_blue(PIN_BTN_BLUE, true);
Button btn_red(PIN_BTN_RED, true);
Button btn_green(PIN_BTN_GREEN, true);

// LED Matrix Displays
Graphics graphics;

void stop_beep() {
    digitalWrite(PIN_BUZZER, LOW);
}

void beep(uint16_t time) {
    if(buzzer_on) {
        digitalWrite(PIN_BUZZER, HIGH);
        buzzer_isr.set_timer(stop_beep, time);
    }
}

void short_beep() {
    if(buzzer_on) {
        digitalWrite(PIN_BUZZER, HIGH);
        buzzer_isr.set_timer(stop_beep, 100);
    }
    
}

void pause_beep() {
    digitalWrite(PIN_BUZZER, LOW);
    buzzer_isr.set_timer(short_beep, 100);
}

void double_beep() {
    digitalWrite(PIN_BUZZER, HIGH);
    buzzer_isr.set_timer(pause_beep, 100);
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
    state_isr.remove();
    red_ready = false;
    blue_ready = false;
    green_ready = false;
    graphics.set_red_ready(false);
    graphics.set_blue_ready(false);
    graphics.set_green_ready(false);
    standby();
}

void post_game_over(){
    if(auto_reset) {
        reset();
    } else{
        graphics.text_static("0:00", color_timer);
    }
}

void game_over(){
    state = GAME_OVER;
    if(game_over_time > 0) {
        beep(game_over_time*1000);
        graphics.text_dynamic(msg_game_over,"Red");
        state_isr.set_timer(post_game_over,game_over_time*1000);
    }else{
        beep(2000);
        post_game_over();
    }
}

void pause(){
    state = PAUSED;
    state_isr.remove();
    time_remaining = time_remaining - go_time + 1;
    if(time_remaining < 0) time_remaining = 0;
    graphics.text_dynamic("PAUSED", "Yellow");
}

void countdown_b(){
    graphics.text_static(format_time(time_remaining, false), color_timer);
    if(time_remaining <= 1) {
        state_isr.set_timer(game_over,500);
    } else {
        state_isr.set_timer(countdown_a,500);
    }
    
}

void countdown_a(){
    time_remaining--;
    graphics.text_static(format_time(time_remaining, true), color_timer);
    state_isr.set_timer(countdown_b,500);
}

// GO!
void pre_countdown_go(){
    state = COUNTDOWN;
    if(go_time > 0){
        beep(go_time*1000);
        graphics.text_static("GO!", "Green");
        state_isr.set_timer(countdown_a,go_time*1000);
    }else{
        beep(1000);
        countdown_a();
    }
}

// 1...
void pre_countdown_1(){
    beep(250);
    graphics.text_static("1",color_pre);
    state_isr.set_timer(pre_countdown_go,1000);

}

// 2...
void pre_countdown_2(){
    beep(250);
    graphics.text_static("2",color_pre);
    state_isr.set_timer(pre_countdown_1,1000);

}

// 3...
void pre_countdown_3(){
    beep(250);
    graphics.text_static("3",color_pre);
    state_isr.set_timer(pre_countdown_2,1000);
}

// Get ready message
void pre_countdown_msg(){
    state = PRE;
    if(pre_time > 0){
        graphics.text_dynamic(msg_get_ready,color_pre);
        state_isr.set_timer(pre_countdown_3,pre_time*1000);
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
    graphics.text_static(format_time(total_time, true), color_timer);
    graphics.set_show_player_bar();
}

// Num players message
void num_players(){
    state = STANDBY;
    if(three_players) graphics.text_dynamic("3 PLAYERS", "Green", standby);
    else graphics.text_dynamic("2 PLAYERS", "Blue", standby);
}

// Intro message
void intro(){
    graphics.text_dynamic(msg_intro, color_intro, num_players);
}

// Check if enough players are ready to start the game
void check_players_ready(){
    if(three_players){
        if(blue_ready & green_ready & red_ready){
            state = PRE;
            ready();
        }else{
            standby();
        }
    }else{
        if(blue_ready & red_ready){
            state = PRE;
            ready();
        }else{
            standby();
        }
    }
}

// Handler for black button press
void black_btn_press(){
    uint32_t start_time = millis();
    switch(state){
        case STARTUP:
            short_beep();
            num_players();
            break;

        case STANDBY:
            standby();
            break;
        
        case PRE:
            beep(1000);
            reset();
            break;

        case COUNTDOWN:
            beep(1000);
            pause();
            break;

        case PAUSED:
            short_beep();
            pre_countdown_msg();
            break;
        
        case GAME_OVER:
            beep(500);
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
                short_beep();
                if(total_time + interval_time > max_time){
                    total_time = min_time;
                }else{
                    total_time = total_time + interval_time;
                }
                prefs.set("total_time", String(total_time));
                standby();
            }else if(three_players){
                double_beep();
                if(green_ready){
                    green_ready = false;
                    graphics.set_green_ready(false);
                } else {
                    green_ready = true;
                    graphics.set_green_ready(true);
                    if(show_ready){
                        graphics.text_dynamic("GREEN READY", "Green",check_players_ready);
                    }else{
                        check_players_ready();
                    }
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
                short_beep();
                prefs.set("brightness",String(graphics.change_brightness()));
            }else{
                double_beep();
                if(blue_ready){
                    blue_ready = false;
                    graphics.set_blue_ready(false);
                } else {
                    blue_ready = true;
                    graphics.set_blue_ready(true);
                    if(show_ready){
                        graphics.text_dynamic("BLUE READY", "Blue", check_players_ready);
                    }else{
                        check_players_ready();
                    }
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
                short_beep();
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
                graphics.set_three_players(three_players);
                num_players();
            }else{
                double_beep();
                if(red_ready){
                    red_ready = false;
                    graphics.set_red_ready(false);
                } else {
                    red_ready = true;
                    graphics.set_red_ready(true);
                    if(show_ready){
                        graphics.text_dynamic("RED READY", "Red",check_players_ready);
                    }else{
                        check_players_ready();
                    }
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
    color_intro = webinterface.load_setting("color_intro");
    color_pre = webinterface.load_setting("color_pre");
    color_timer = webinterface.load_setting("color_timer");
    graphics.set_show_aux_lights(webinterface.load_setting("show_aux_lights") == "true");
    graphics.set_show_dim_lights(webinterface.load_setting("show_dim_lights") == "true");
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
    buzzer_on = webinterface.load_setting("buzzer_on") != "false";


    // Prefs File
    String total_time_string = (prefs.get("total_time"));
    if(total_time_string == ""){
        total_time = 90;
    }else{
        total_time = total_time_string.toInt();
    } 
    if(total_time > max_time) total_time = max_time;
    if(total_time < min_time) total_time = min_time;

    graphics.set_brightness(prefs.get("brightness"));

    three_players = (prefs.get("num_players") == "3");
    graphics.set_three_players(three_players);

}

// Wi-Fi Loop for settings update
void wifi_loop(){
    // Display a static wifi symbol on the displays
    graphics.show_wifi();

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
  
    // Initialize web interface
    webinterface.begin();

    // Initialize LED displays
    graphics.begin();

    // If black button held during start up, go to the wifi_loop
    if(!digitalRead(PIN_BTN_BLACK)){
        wifi_loop();
    }

    // Disable wifi and load settings
    WiFi.mode(WIFI_OFF);
    load_settings();

    // Startup beep
    beep(500);
    
    // Intro sequence
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
    state_isr.handle();
    buzzer_isr.handle();

    // Handle all button inputs
    btn_black.handle();
    btn_blue.handle();
    btn_red.handle();
    btn_green.handle();

    // Handle LED displays
    graphics.handle();
}