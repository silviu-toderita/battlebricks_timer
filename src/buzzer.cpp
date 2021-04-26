#include "buzzer.h"

Buzzer::Buzzer(uint8_t pin, bool buzzer_on){
    _pin = pin;
    _buzzer_on = buzzer_on;

    pinMode(_pin, OUTPUT);
    _double = false;
    _buzzer_active = false;
}

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

void Buzzer::set_buzzer_on(bool buzzer_on){
    _buzzer_on = buzzer_on;
}

void Buzzer::beep(uint16_t time) {
    if(_buzzer_on) {
        digitalWrite(_pin, HIGH);
        _time_stop = millis() + time;
        _buzzer_active = true;
    }
}

void Buzzer::beep_short() {
    beep(100);
}

void Buzzer::beep_double() {
    beep_short();
    _double = true;
}