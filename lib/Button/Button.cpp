#include "Button.h"

/**
 * Constructor
 * @param pin_in of button (GND)
 * @param internal_pullup enabled on Arduino
 **/
Button::Button(uint8_t pin_in, bool internal_pullup){
    pin = pin_in;
    if(internal_pullup) pinMode(pin_in, INPUT_PULLUP);
    else pinMode(pin_in, INPUT);
}

/**
 * Check button status (run every loop)
 **/
void Button::handle(){
    bool new_state = !digitalRead(pin);
    if(new_state != current_state && millis() > last_change + DEBOUNCE_TIME){
        if(new_state){
            if(posedge_cb != NULL) posedge_cb();
        }
        if(!new_state){
            if(negedge_cb != NULL) negedge_cb();
        }
        current_state = new_state;
        last_change = millis();
    }
}

/**
 * Set callback for button going from not pressed to pressed
 * @param posedge_cb_in void function to call
 **/
void Button::set_posedge_cb(void_function_pointer posedge_cb_in){
    posedge_cb = posedge_cb_in;
}

/**
 * Set callback for button going from pressed to not pressed
 * @param posedge_cb_in void function to call
 **/
void Button::set_negedge_cb(void_function_pointer negedge_cb_in){
    negedge_cb = negedge_cb_in;
}

/**
 * Get current button status
 * @return current_state of button (TRUE = pressed, FALSE = not pressed)
 **/
bool Button::get(){
    return current_state;
}