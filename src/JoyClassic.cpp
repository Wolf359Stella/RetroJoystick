#include "JoyClassic.h"

JoyClassic::JoyClassic(uint8_t up, uint8_t down, uint8_t left, uint8_t right, 
    uint8_t button): up(up), down(down), left(left), right(right),
    button (button) {
    pinMode(up, INPUT_PULLUP);  
    pinMode(down, INPUT_PULLUP);  
    pinMode(left, INPUT_PULLUP);  
    pinMode(right, INPUT_PULLUP);  
    pinMode(button, INPUT_PULLUP); 
}


void JoyClassic::loop() {
    !digitalRead(button) ? pressButton(0) : releaseButton(0);
    _xAxis.val = 0;
    _yAxis.val = 0;
    if (!digitalRead(up)) _yAxis.val = -127;
    if (!digitalRead(down)) _yAxis.val = 127;
    if (!digitalRead(left)) _xAxis.val = -127;
    if (!digitalRead(button)) _xAxis.val = 127;
    sendState();
    delay(50);
}