#pragma once
#include "RetroJoystick.h"

class JoyNESClone: public Joystick_ {
    private:
        uint8_t _clock;
        uint8_t _latch;
        uint8_t _data;
        uint8_t _last_state1 = 0;
        uint8_t _last_state2 = 0;

        uint8_t read_package(void);
    public:
        JoyNESClone(uint8_t clock, uint8_t latch, uint8_t data);
        
        /**
        * Main loop for the joystick.
        */
        void loop();
};