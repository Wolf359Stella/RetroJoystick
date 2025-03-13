#pragma once
#include "RetroJoystick.h"


class JoyClassic: public Joystick_ {
    private:
        uint8_t up;
        uint8_t down;
        uint8_t left;
        uint8_t right;
        uint8_t button;
    public:
        JoyClassic(uint8_t up, uint8_t down, uint8_t left, 
                   uint8_t right, uint8_t button);
        /**
        * Main loop for the joystick.
        */
        void loop();
};
