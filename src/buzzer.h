#include "Arduino.h"

#define SHORT_BEEP_TIME 100

class Buzzer{
    public:
        Buzzer(uint8_t pin, bool buzzer_on);

        void handle();
        void set_buzzer_on(bool buzze_on);

        void beep(uint16_t time);
        void beep_short();
        void beep_double();
    private:
        uint8_t _pin;
        bool _buzzer_on;

        bool _buzzer_active;
        long _time_stop;
        bool _double;
};