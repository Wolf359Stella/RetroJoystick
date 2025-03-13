#pragma once
#include "RetroJoystick.h"

class JoySNES: public Joystick_ {
    private:
        uint8_t _clock;
        uint8_t _latch;
        uint8_t _data;
    public:
        JoySNES(uint8_t clock, uint8_t latch, uint8_t data);
        /**
        * Main loop for the joystick.
        */
        void loop();
};
