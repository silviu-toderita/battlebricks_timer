#include "Arduino.h"

typedef enum {
    ISR_TIMER   = 0,
    ISR_TRIGGER = 1
} ISR_Type;

typedef void (*void_function_pointer)();

class Soft_ISR{

    public:

        Soft_ISR();

        void 
            set_trigger(void_function_pointer),
            set_timer(void_function_pointer, uint32_t),
            trigger(),
            handle();

    private:

        ISR_Type type;
        bool enabled = false;
        uint64_t do_at_time;
        void_function_pointer do_callback;
};