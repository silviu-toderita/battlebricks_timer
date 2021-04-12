#include "graphics.h"

Soft_ISR isr;

bool show_brightness;

void show_brightness_off() {
    show_brightness = false;
}

void Graphics::begin() {
    // Initialize Matrix displays
    display_1.begin();
    display_2.begin();
    update_brightness();
}

void Graphics::handle() {
    isr.handle();
    // Clear Displays
    display_1.clear();
    display_2.clear();

    // Draw brightness display
    if(show_brightness){
        draw_brightness();
    } else {
        // Draw text
        draw_text();
        // Draw player ready bars
        if(show_player_bar) draw_players_ready();
    };

    // Refresh display
    display_1.show();
    display_2.show();
}

// Set new static text (centered)
void Graphics::text_static(String text, String color){
    text_scroll = false;
    text_xpos = 15 - (text.length() * 3);
    text_color = parse_color(color);
    text_string = text;
}

// Set new dynamic (scrolling) text
void Graphics::text_dynamic(String text, String color){
    text_xpos = 32;
    text_color = parse_color(color);
    text_string = text;
    text_scroll = true;
}

// Set new dynamic (scrolling) text
void Graphics::text_dynamic(String text, String color, void_function_pointer _callback){
    text_dynamic(text, color);
    isr.set_trigger(_callback);
}

void Graphics::set_brightness(String input){
    if(input == ""){
        brightness = 2;
    }else{
        brightness = input.toInt();
        if(brightness > 8) brightness = 8;
        if(brightness < 1) brightness = 1;
    }
    update_brightness();
}

uint8_t Graphics::change_brightness() {
    if(brightness == 8){
        brightness = 1;
    }else{
        brightness++;
    }
    update_brightness();
    
    show_brightness = true;
    isr.set_timer(show_brightness_off,1000);

    return brightness;
}

void Graphics::show_wifi() {
    display_1.clear();
    display_2.clear();
    display_1.setBrightness(32);
    display_2.setBrightness(24);
    display_1.drawBitmap(8,3,bmp_wifi_l,16,9,CYAN);
    display_2.drawBitmap(4,0,bmp_wifi_s,8,8,CYAN);
    display_1.show();
    display_2.show();
}

void Graphics::set_three_players(bool in){
    three_players = in;
}
void Graphics::set_red_ready(bool in){
    red_ready = in;
}
void Graphics::set_blue_ready(bool in){
    blue_ready = in;
}
void Graphics::set_green_ready(bool in){
    green_ready = in;
}
void Graphics::set_show_player_bar(){
    show_player_bar = true;
}

// Setters
void Graphics::set_show_aux_lights(bool in){
    show_aux_lights = in;
}
void Graphics::set_show_dim_lights(bool in){
    show_dim_lights = in;
}

void Graphics::draw_three_players_ready() {
    if(green_ready){
        display_1.drawRect(8,0,16,2,GREEN);
        display_2.drawLine(4,0,11,0,GREEN);
    }else if(show_dim_lights){
        display_1.drawRect(8,0,16,2,GREEN_DIM);
        display_2.drawLine(4,0,11,0,GREEN_DIM);
    } 
    
    if(show_aux_lights){
        if(blue_ready){
            display_1.drawRect(0,0,6,2,BLUE);
            display_2.drawLine(13,0,15,0,BLUE);
        }else if(show_dim_lights){
            display_1.drawRect(0,0,6,2,BLUE_DIM);
            display_2.drawLine(13,0,15,0,BLUE_DIM);
        }

        if(red_ready){
            display_1.drawRect(26,0,6,2,RED);
            display_2.drawLine(0,0,2,0,RED);
        }else if(show_dim_lights){
            display_1.drawRect(26,0,6,2,RED_DIM);
            display_2.drawLine(0,0,2,0,RED_DIM);
        }
    }
}

void Graphics::draw_two_players_ready() {
    if(blue_ready){
        display_1.drawRect(0,0,14,2,BLUE);
        display_2.drawLine(9,0,15,0,BLUE);
    }else if(show_dim_lights){
        display_1.drawRect(0,0,14,2,BLUE_DIM);
        display_2.drawLine(9,0,15,0,BLUE_DIM);
    }
    
    if(red_ready){
        display_1.drawRect(18,0,14,2,RED);
        display_2.drawLine(0,0,6,0,RED);
    }else if(show_dim_lights){
        display_1.drawRect(18,0,14,2,RED_DIM);
        display_2.drawLine(0,0,6,0,RED_DIM);
    }
}

// Handle drawing players ready lights
void Graphics::draw_players_ready(){
    if(three_players){
        draw_three_players_ready();
    }else if(show_aux_lights){
        draw_two_players_ready();
    }

    if(blue_ready) digitalWrite(PIN_LED_BLUE, HIGH);
    else digitalWrite(PIN_LED_BLUE, LOW);

    if(red_ready) digitalWrite(PIN_LED_RED, HIGH);
    else digitalWrite(PIN_LED_RED, LOW);
}

// Handle drawing text
void Graphics::draw_text(){
    if(text_scroll){
        text_xpos--;

        int16_t text_length = text_string.length() * 8;
        if(text_xpos < -text_length){
            text_xpos = 32;
            isr.trigger();
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

void Graphics::draw_brightness() {
    display_1.drawBitmap(1,2,bmp_brightness_l,16,13,WHITE);
    display_2.drawBitmap(0,0,bmp_brightness_s,8,8,WHITE);

    display_1.setCursor(20, 12);
    display_2.setCursor(10, 6);

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
}




void Graphics::update_brightness(){
    display_1.setBrightness(brightness * 10 + 10);
    display_2.setBrightness(brightness * 10);
}

// Parse color from string
uint16_t Graphics::parse_color(String input){
    if(input == "Blue") return BLUE;
    if(input == "White") return WHITE;
    if(input == "Green") return GREEN;
    if(input == "Cyan") return CYAN;
    if(input == "Magenta") return MAGENTA;
    if(input == "Yellow") return YELLOW;
    return RED;
}