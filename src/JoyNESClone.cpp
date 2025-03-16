#include "JoyNESClone.h"


JoyNESClone::JoyNESClone(uint8_t clock, uint8_t latch, uint8_t data):
        _clock(clock), _latch(latch), _data(data) {
    pinMode(_clock, OUTPUT);
    pinMode(_latch, OUTPUT);
    pinMode(_data, INPUT_PULLUP);
    digitalWrite(_clock, HIGH);
    digitalWrite(_latch, LOW); 
}


uint8_t JoyNESClone::read_package(){
    uint8_t state = 0;
    digitalWrite(_latch, HIGH);
    delay(3);
    digitalWrite(_latch, LOW);
    delayMicroseconds(2150);
    for(int i = 0; i < 8; i++){
        digitalWrite(_clock, LOW);
        delayMicroseconds(1950);
        state |= digitalRead(_data) << i;
        digitalWrite(_clock, HIGH);
        delayMicroseconds(1950);
    }
    return ~state;
}

void JoyNESClone::loop(){
    uint8_t state1 = read_package();
    uint8_t state2 = read_package();
    uint16_t clocked = (state1 ^ state2) & (_last_state1 ^ _last_state2); // square, triangle, R1, L1
    uint16_t state = state1 & _last_state1 & state2 & _last_state2; // 
    
    1 & clocked ? pressButton(0) : releaseButton(0); // triangle (R1 ignored)
    2 & clocked ? pressButton(1) : releaseButton(1); // square (L1 ignored)
    1 & state ? pressButton(2) : releaseButton(2); // circle (R2 ignored)
    2 & state ? pressButton(3) : releaseButton(3); // X (L2 ignored)
    4 & state ? pressButton(4) : releaseButton(4); // Select
    8 & state ? pressButton(5) : releaseButton(5); // Start

    setHatSwitch(0, -1);
    if (16 & ~state) setHatSwitch(0, 0);
    if (128 & ~state) setHatSwitch(0, 90);
    if (32 & ~state) setHatSwitch(0, 180);
    if (64 & ~state) setHatSwitch(0, 270);
    if (16 & ~state && 128 & ~state) setHatSwitch(0, 45);
    if (32 & ~state && 128 & ~state) setHatSwitch(0, 135);
    if (32 & ~state && 64 & ~state) setHatSwitch(0, 225);
    if (16 & ~state && 64 & ~state) setHatSwitch(0, 315);
    
    sendState();
    _last_state1 = state1;
    _last_state2 = state2;
}