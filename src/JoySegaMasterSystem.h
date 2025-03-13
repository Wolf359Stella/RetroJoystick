#pragma once
#include "RetroJoystick.h"

class JoySegaMasterSystem: public Joystick_ {
    private:
        uint8_t _up;
        uint8_t _down;
        uint8_t _left;
        uint8_t _right;
        uint8_t _b1;
        uint8_t _b2;
    public:
        JoySegaMasterSystem(uint8_t up, uint8_t down, uint8_t left, 
                            uint8_t right, uint8_t b1, uint8_t b2);
        /**
        * Main loop for the joystick.
        */
        void loop();
};




