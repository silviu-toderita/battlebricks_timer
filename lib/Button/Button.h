#include "Arduino.h"

typedef void (*void_function_pointer)();

// Time to wait for button input to settle
#define DEBOUNCE_TIME 50

class Button{

    public:

        Button(uint8_t pin_in, bool internal_pullup);

        void 
            handle(),
            set_posedge_cb(void_function_pointer),
            set_negedge_cb(void_function_pointer);
        
        bool get();

    private:

        bool current_state = false;
        uint8_t pin;
        void_function_pointer posedge_cb = NULL;
        void_function_pointer negedge_cb = NULL;
        uint64_t last_change = 0;
};