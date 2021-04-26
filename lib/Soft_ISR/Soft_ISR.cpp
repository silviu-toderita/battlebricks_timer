#include "Soft_ISR.h"

Soft_ISR::Soft_ISR(){}

/**
 * Set a trigger-based interrupt
 * @param do_callback_in void function to call
 **/
void Soft_ISR::set_trigger(void_function_pointer do_callback_in){
    type = ISR_TRIGGER;
    enabled = true;
    do_callback = do_callback_in;
}

/**
 * Set a timer-based interrupt
 * @param do_callback_in void function to call\
 * @param time to call function (ms)
 **/
void Soft_ISR::set_timer(void_function_pointer do_callback_in, uint32_t time){
    type = ISR_TIMER;
    enabled = true;
    do_at_time = millis() + time;
    do_callback = do_callback_in;
}

/**
 * Trigger the current interrupt if it's enabled by initiating callback
 **/
void Soft_ISR::trigger(){
    if(enabled) {
        enabled = false;
        do_callback();
    }
}

/**
 * Remove the interrupt
 **/
void Soft_ISR::remove(){
    enabled = false;
}

/**
 * Check time for timer-based interrupt (call every loop)
 **/
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