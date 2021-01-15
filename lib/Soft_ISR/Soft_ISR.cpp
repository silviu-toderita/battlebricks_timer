#include "Soft_ISR.h"

Soft_ISR::Soft_ISR(){}

void Soft_ISR::set_trigger(void_function_pointer do_callback_in){
    type = ISR_TRIGGER;
    enabled = true;
    do_callback = do_callback_in;
}

void Soft_ISR::set_timer(void_function_pointer do_callback_in, uint32_t time){
    type = ISR_TIMER;
    enabled = true;
    do_at_time = millis() + time;
    do_callback = do_callback_in;
}

void Soft_ISR::trigger(){
    enabled = false;
    do_callback();
}

void Soft_ISR::handle(){
    if(enabled){
        if(type == ISR_TIMER){
            if(millis() >= do_at_time){
                enabled = false;
                do_callback();
            }
        }
    }
}