#pragma once
#include "RetroJoystick.h"

typedef struct {
    int pin;
    int lowFlag;
    int highFlag;
    int pulse3Flag;
} MegadriveInput;


class JoySegaMegaDrive: public Joystick_ {
    private:
        uint16_t _currentState[2] = { 0, 0 };
        uint16_t _lastState[2] = { 0, 0 };
        uint8_t _select[2] = { 8, 9 };
        uint8_t _players;
        bool _sixButtonMode[2] = { false, false };

        void read6buttons(int player);
        void read3buttons(int player);
        void sendStates();
    public:
        static const int ON = 1;
        static const int UP = 2;
        static const int DOWN = 4;
        static const int LEFT = 8;
        static const int RIGHT = 16;
        static const int START = 32;
        static const int A = 64;
        static const int B = 128;
        static const int C = 256;
        static const int X = 512;
        static const int Y = 1024;
        static const int Z = 2048;
        static const int MODE = 4096;
        MegadriveInput _inputMap[12] = {
            { 2,  UP,    UP,     Z    }, // P0 DB9 Pin 1 (UP/Z)
            { 3,  DOWN,  DOWN,   Y    }, // P0 DB9 Pin 2 (DOWN/Y)
            { 4,  ON,    LEFT,   X    }, // P0 DB9 Pin 3 (LEFT/X)
            { 5,  ON,    RIGHT,  MODE }, // P0 DB9 Pin 4 (RIGHT/MODE)
            { 6,  A,     B,      0    }, // P0 DB9 Pin 6 (A/B)
            { 7,  START, C,      0    }, // P0 DB9 Pin 9 (START/C)
            { A0, UP,    UP,     Z    }, // P1 DB9 Pin 1 (UP/Z)
            { A1, DOWN,  DOWN,   Y    }, // P1 DB9 Pin 2 (DOWN/Y)
            { A2, ON,    LEFT,   X    }, // P1 DB9 Pin 3 (LEFT/X)
            { A3, ON,    RIGHT,  MODE }, // P1 DB9 Pin 4 (RIGHT/MODE)
            { A4, A,     B,      0    }, // P1 DB9 Pin 6 (A/B)
            { A5, START, C,      0    }  // P1 DB9 Pin 9 (START/C)
        };
        JoySegaMegaDrive(const int players);

        /**
        * Main loop for the joystick.
        */
        void loop();

};