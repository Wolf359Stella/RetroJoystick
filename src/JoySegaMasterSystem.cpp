#include "JoySegaMasterSystem.h"


JoySegaMasterSystem::JoySegaMasterSystem(uint8_t up, uint8_t down, 
    uint8_t left, uint8_t right, uint8_t b1, uint8_t b2):
    _up(up), _down(down), _left(left), _right(right), _b1(b1), _b2(b2) {
    pinMode(up, INPUT_PULLUP);  
    pinMode(down, INPUT_PULLUP);  
    pinMode(left, INPUT_PULLUP);  
    pinMode(right, INPUT_PULLUP);  
    pinMode(b1, INPUT_PULLUP);  
    pinMode(b2, INPUT_PULLUP); 
}


void JoySegaMasterSystem::loop(){
    !digitalRead(_up) ? pressButton(0) : releaseButton(0);
    !digitalRead(_down) ? pressButton(1) : releaseButton(1);
    !digitalRead(_left) ? pressButton(2) : releaseButton(2);
    !digitalRead(_right) ? pressButton(3) : releaseButton(3);
    !digitalRead(_b1) ? pressButton(4) : releaseButton(4);
    !digitalRead(_b2) ? pressButton(5) : releaseButton(5);
    sendState();
    delay(25);
}