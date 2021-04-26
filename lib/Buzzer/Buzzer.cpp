/**
 * Buzzer Library
 * Control an externally connected buzzer
 * 
 * Run the handle() function on each loop or as often as possible for correct timing.
 **/
#include "Buzzer.h"

/**
 * Constructor
 * @param pin for buzzer (+)
 * @param buzzer_on
 **/
Buzzer::Buzzer(uint8_t pin, bool buzzer_on){
    _pin = pin;
    _buzzer_on = buzzer_on;

    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _double = false;
    _buzzer_active = false;
}

/**
 * Update buzzer status (run every loop)
 **/
void Buzzer::handle(){
    if(_buzzer_active && millis() >= _time_stop){
        digitalWrite(_pin, LOW);
        _buzzer_active = false;
        if(_double){
            _time_stop = millis() + SHORT_BEEP_TIME;
        }
    }else if(_double && millis() >= _time_stop){
        beep_short();
        _double = false;
    }
}

/**
 * Set buzzer on
 * @param buzzer_on
 **/
void Buzzer::set_buzzer_on(bool buzzer_on){
    _buzzer_on = buzzer_on;
}

/**
 * Beep for some amount of time
 * @param time to beep (ms)
 **/
void Buzzer::beep(uint16_t time) {
    if(_buzzer_on) {
        digitalWrite(_pin, HIGH);
        _time_stop = millis() + time;
        _buzzer_active = true;
    }
}

/**
 * Short beep
 **/
void Buzzer::beep_short() {
    beep(100);
}

/**
 * Double short beep
 **/
void Buzzer::beep_double() {
    beep_short();
    _double = true;
}