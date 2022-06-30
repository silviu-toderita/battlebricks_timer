/**
 * BATTLEBRICKS TIMER
 * https://github.com/silviu-toderita/battlebricks_timer
 * 
 * Battlebricks Timer is an ESP8266-based countdown timer for Lego Battlebricks
 * competitions. 
 * 
 * Features: 
 *  -Audience-facing 32x16 LED matrix display (WS2812)
 *  -Player-facing 16x8 LED matrix display (WS2812)
 *  -Player ready lights for 2 players
 *  -Auxiliary player ready lights on LED displays supports up to 3 players
 *  -Buzzer for audio feedback
 *  -Wi-Fi and web settings page for full customization of all parameters
 *  -In-game settings can be adjusted on the fly (total time, number of players, screen brightness)
 * 
 * Setup:
 *  Full details at https://github.com/silviu-toderita/battlebricks_timer
 * 
 **/
#include "battlebricks.h"

// Networking
ESP8266WiFiMulti wifimulti;
Web_Interface webinterface;

// Interrupts
Soft_ISR state_isr;
Soft_ISR buzzer_isr;

// In-game settings
Persistent_Storage ingame_settings("pref");

// Buttons
Button btn_black(PIN_BTN_BLACK, true);
Button btn_blue(PIN_BTN_BLUE, true);
Button btn_red(PIN_BTN_RED, true);
Button btn_green(PIN_BTN_GREEN, true);

// LED Matrix Displays
Graphics graphics;

// Buzzer
Buzzer buzzer(PIN_BUZZER, true);


/**
 * Format Time
 * @param time in seconds
 * @param colon display
 * @return time formatted for digital clock
 **/
String format_time(uint8_t time, bool colon){
    String minute = String(time / 60);
    String second = String(time % 60);
    if(second.length() == 1) second = "0" + second;
    if(colon) return minute + ":" + second;
    return minute + " " + second;
}

/**
 * TIMER SEQUENCE
 *  V V V V V V V
 **/
void ready();
void standby();
void countdown_a();

// Reset state
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

// Display 0:00 post-game-over
void post_game_over(){
    if(auto_reset) {
        reset();
    } else{
        graphics.text_static("0:00", color_timer);
    }
}

// Display game over message
void game_over(){
    state = GAME_OVER;
    if(game_over_time > 0) {
        buzzer.beep(game_over_time*1000);
        graphics.text_dynamic(msg_game_over,"Red");
        state_isr.set_timer(post_game_over,game_over_time*1000);
    }else{
        buzzer.beep(2000);
        post_game_over();
    }
}

// Display paused message
void pause(){
    state = PAUSED;
    state_isr.remove();
    time_remaining = time_remaining - go_time + 1;
    if(time_remaining < 0) time_remaining = 0;
    graphics.text_dynamic("PAUSED", "Yellow");
}

// Display time remaining without colon and decrement time by 1 second
void countdown_b(){
    graphics.text_static(format_time(time_remaining, false), color_timer);
    if(time_remaining <= 1) {
        state_isr.set_timer(game_over,500);
    } else {
        state_isr.set_timer(countdown_a,500);
    }
    
}

// Display time remaining with colon
void countdown_a(){
    time_remaining--;
    graphics.text_static(format_time(time_remaining, true), color_timer);
    state_isr.set_timer(countdown_b,500);
}

// Display go message
void pre_countdown_go(){
    state = COUNTDOWN;
    if(go_time > 0){
        buzzer.beep(go_time*1000);
        graphics.text_static("GO!", "Green");
        state_isr.set_timer(countdown_a,go_time*1000);
    }else{
        buzzer.beep(1000);
        countdown_a();
    }
}

// Display 1
void pre_countdown_1(){
    buzzer.beep(250);
    graphics.text_static("1",color_pre);
    state_isr.set_timer(pre_countdown_go,1000);

}

// Display 2
void pre_countdown_2(){
    buzzer.beep(250);
    graphics.text_static("2",color_pre);
    state_isr.set_timer(pre_countdown_1,1000);

}

// Display 3
void pre_countdown_3(){
    buzzer.beep(250);
    graphics.text_static("3",color_pre);
    state_isr.set_timer(pre_countdown_2,1000);
}

// Display get ready message
void pre_countdown_msg(){
    state = PRE;
    if(pre_time > 0){
        graphics.text_dynamic(msg_get_ready,color_pre);
        state_isr.set_timer(pre_countdown_3,pre_time*1000);
    }else{
        pre_countdown_3();
    }
}

// Set the time
void ready(){
    time_remaining = total_time - go_time + 1;
    pre_countdown_msg();
}

// Start rumble mode
void rumble(){
    graphics.text_dynamic(msg_rumble, "Red", ready);
}

// Display static clock
void standby(){
    graphics.text_static(format_time(total_time, true), color_timer);
    graphics.set_show_player_bar();
}

// Display number of players
void num_players(){
    state = STANDBY;
    switch(mode){
        case TWO_PLAYER:
            graphics.text_dynamic("2 PLAYERS", "Blue", standby);
            break;
        case THREE_PLAYER:
            graphics.text_dynamic("3 PLAYERS", "Green", standby);
            break;
        default:
            graphics.text_dynamic("RUMBLE MODE", "Red", standby);
            break;
    }
}

// Display intro message
void intro(){
    graphics.text_dynamic(msg_intro, color_intro, num_players);
}
/**
 *  ^ ^ ^ ^ ^ ^ ^ 
 * TIMER SEQUENCE
 **/


/**
 * Check if enough players are ready, and start the game
 **/
void check_players_ready(){
    if(mode == THREE_PLAYER){
        if(blue_ready & green_ready & red_ready){
            state = PRE;
            ready();
        }else{
            standby();
        }
    }else if(mode == TWO_PLAYER){
        if(blue_ready & red_ready){
            state = PRE;
            ready();
        }else{
            standby();
        }
    }
}

/**
 * Handles black button press
 **/
void black_btn_press(){
    long start_time = millis();
    bool reset_flag = false;
    switch(state){
        // Skip ahead during startup
        case STARTUP:
            buzzer.beep_short();
            num_players();
            break;
        case STANDBY:
            standby();
            break;
        // Reset game during pre-countdown
        case PRE:
            buzzer.beep(1000);
            reset();
            break;
        // Pause game
        case COUNTDOWN:
            buzzer.beep(1000);
            pause();
            break;
        // Resume game or restart if button held for 3 seconds
        case PAUSED:
            while(btn_black.get()){
                yield();
                if(millis() > start_time + 3000){
                    reset_flag = true;
                    break;
                }
                btn_black.handle();
            }
            if(reset_flag){
                buzzer.beep(250);
                reset();
            }else{
                buzzer.beep_short();
                pre_countdown_msg();
            }
            break;
        // Reset game after game over
        case GAME_OVER:
            buzzer.beep(250);
            reset();
            break;

        default: break;
    }
}

/**
 * Handles green button press (only active during STANDBY state)
 **/
void green_btn_press(){
    switch(state){
        case STANDBY:
            // Set green player ready / not ready if in three player mode
            if(!btn_black.get()){
                if(mode == THREE_PLAYER){
                    buzzer.beep_double();
                    if(mode == RUMBLE){
                        rumble();
                    }else if(green_ready){
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
            // If black button being pressed, alt function is change time
            }else{
                buzzer.beep_short();
                if(total_time + interval_time > max_time){
                    total_time = min_time;
                }else{
                    total_time = total_time + interval_time;
                }
                ingame_settings.set("total_time", String(total_time));
                standby();
            }
            break;
        
        default:
            break;
    }
}

/**
 * Handles blue button press (only active during STANDBY state)
 **/
void blue_btn_press(){
    switch(state){
        case STANDBY:
            // Set blue player ready / not ready
            if(!btn_black.get()){
                buzzer.beep_double();
                if(mode == RUMBLE){
                    rumble();
                }else if(blue_ready){
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
            // If black button being pressed, alt function is change brightness
            }else{
                buzzer.beep_short();
                ingame_settings.set("brightness",String(graphics.change_brightness()));
            }
            break;
        
        default:
            break;
    }
}

/**
 * Handles red button press (only active during STANDBY state)
 **/
void red_btn_press(){
    switch(state){
        case STANDBY:
            // Set red player ready / not ready
            if(!btn_black.get()){
                buzzer.beep_double();
                if(mode == RUMBLE){
                    rumble();
                }else if(red_ready){
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
            // If black button being pressed, alt function is change number of players
            }else{
                buzzer.beep_short();
                switch(mode){
                    case(TWO_PLAYER):
                        mode = THREE_PLAYER;
                        ingame_settings.set("mode", "1");
                        break;
                    case(THREE_PLAYER):
                        mode = RUMBLE;
                        ingame_settings.set("mode", "2");
                        break;
                    default:
                        mode = TWO_PLAYER;
                        ingame_settings.set("mode", "0");
                        break;      
                }
                red_ready = false;
                blue_ready = false;
                green_ready = false;
                graphics.set_three_players(mode == THREE_PLAYER);
                graphics.set_rumble_mode(mode == RUMBLE);
                num_players();
            }
            break;
        
        default:
            break;
    }
}

/**
 * Load settings from settings file
 **/
void load_settings(){

    // General Settings
    msg_intro = webinterface.load_setting("msg_intro");
    color_intro = webinterface.load_setting("color_intro");
    color_pre = webinterface.load_setting("color_pre");
    color_timer = webinterface.load_setting("color_timer");
    graphics.set_show_aux_lights(webinterface.load_setting("show_aux_lights") == "true");
    graphics.set_show_dim_lights(webinterface.load_setting("show_dim_lights") == "true");
    show_ready = webinterface.load_setting("show_ready") == "true";
    msg_rumble = webinterface.load_setting("msg_rumble");
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
    buzzer.set_buzzer_on(webinterface.load_setting("buzzer_on") != "false");


    // In-Game Settings
    String total_time_string = (ingame_settings.get("total_time"));
    if(total_time_string == ""){
        total_time = 90;
    }else{
        total_time = total_time_string.toInt();
    } 
    if(total_time > max_time) total_time = max_time;
    if(total_time < min_time) total_time = min_time;

    graphics.set_brightness(ingame_settings.get("brightness"));

    mode = ingame_settings.get("mode").toInt();
    graphics.set_three_players(mode == THREE_PLAYER);
    graphics.set_rumble_mode(mode == RUMBLE);

}

/**
 * WiFi setup mode loops until restart
 **/
void wifi_setup(){
    // Display a static wifi symbol on the displays
    graphics.show_wifi();

    // Start a hotspot
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(1,2,3,4),IPAddress(1,2,3,4),IPAddress(255,255,255,0));

    // Load SSID and password settings
    String SSID = webinterface.load_setting("hotspot_SSID");
    if(SSID == ""){
        WiFi.softAP("battlebricks","12345678");
    }else{
        String password = webinterface.load_setting("hotspot_password");
        if(password.length() < 8){
            WiFi.softAP(SSID);
        }else{
            WiFi.softAP(SSID, password);
        }
    }

    // Initialize OTA
    ArduinoOTA.setHostname("battlebricks");
    ArduinoOTA.setPassword("12345678");
    ArduinoOTA.begin();

    // Wifi Loop
    uint64_t button_pressed_time;
    bool button_pressed = false;
    bool restart = false;
    while(true){
        webinterface.handle();
        ArduinoOTA.handle();
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

/**
 * 
 * SETUP
 * 
 * */
void setup(){
    // Set button callbacks
    btn_black.set_posedge_cb(black_btn_press);
    btn_blue.set_posedge_cb(blue_btn_press);
    btn_green.set_posedge_cb(green_btn_press);
    btn_red.set_posedge_cb(red_btn_press);
  
    // Initialize web interface
    webinterface.begin();

    // Initialize displays and LEDs
    graphics.begin();

    // If black button held during start up, enter wifi setup mode
    if(!digitalRead(PIN_BTN_BLACK)){
        wifi_setup();
    }

    // Disable wifi and load settings
    WiFi.mode(WIFI_OFF);
    load_settings();

    // Startup beep
    buzzer.beep(500);
    
    // Display intro message or skip to displaying the number of players if blank
    if(msg_intro == ""){
        num_players();
    }else{
        intro();
    }
    
}   

/**
 * 
 * LOOP
 * 
 * */
void loop(){
    // Handle time-based interrupts
    state_isr.handle();
    buzzer_isr.handle();

    // Handle button inputs
    btn_black.handle();
    btn_blue.handle();
    btn_red.handle();
    btn_green.handle();

    graphics.handle();
    buzzer.handle();
}