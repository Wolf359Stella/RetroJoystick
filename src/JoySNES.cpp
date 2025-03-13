#include "JoySNES.h"

#define SNES_B       1      //000000000001
#define SNES_Y       2      //000000000010
#define SNES_SELECT  4      //000000000100
#define SNES_START   8      //000000001000
#define SNES_UP      16     //000000010000
#define SNES_DOWN    32     //000000100000
#define SNES_LEFT    64     //000001000000
#define SNES_RIGHT   128    //000010000000
#define SNES_A       256    //000100000000
#define SNES_X       512    //001000000000
#define SNES_L       1024   //010000000000
#define SNES_R       2048   //100000000000


JoySNES::JoySNES(uint8_t clock, uint8_t latch, uint8_t data):
		_clock(clock), _latch(latch), _data(data) {
    pinMode(clock, OUTPUT);
    digitalWrite(clock, HIGH);
    pinMode(latch, OUTPUT);
    digitalWrite(latch, LOW);
    pinMode(data, INPUT_PULLUP);
}


void JoySNES::loop(){
    uint16_t state = 0;
    digitalWrite(_latch, HIGH);
    delayMicroseconds(12); // 12us latch
    digitalWrite(_latch, LOW);
    delayMicroseconds(6);
    for(int i = 0; i < 16; i++){
        digitalWrite(_clock, LOW);
        delayMicroseconds(6);
        state |= digitalRead(_data) << i;
        digitalWrite(_clock, HIGH);
        delayMicroseconds(6);
    }
    SNES_B & ~state ? pressButton(0) : releaseButton(0);
    SNES_Y & ~state ? pressButton(1) : releaseButton(1);
    SNES_SELECT & ~state ? pressButton(2) : releaseButton(2);
    SNES_START & ~state ? pressButton(3) : releaseButton(3);
    SNES_A & ~state ? pressButton(4) : releaseButton(4);
    SNES_X & ~state ? pressButton(5) : releaseButton(5);
    SNES_L & ~state ? pressButton(6) : releaseButton(6);
    SNES_R & ~state ? pressButton(7) : releaseButton(7);
    // 360° Hat Switch 0
    setHatSwitch(0, -1); // release
    if (SNES_UP & ~state) setHatSwitch(0, 0);
    if (SNES_RIGHT & ~state) setHatSwitch(0, 90);
    if (SNES_DOWN & ~state) setHatSwitch(0, 180);
    if (SNES_LEFT & ~state) setHatSwitch(0, 270);
    if (SNES_UP & ~state && SNES_RIGHT & ~state) setHatSwitch(0, 45);
    if (SNES_DOWN & ~state && SNES_RIGHT & ~state) setHatSwitch(0, 135);
    if (SNES_DOWN & ~state && SNES_LEFT & ~state) setHatSwitch(0, 225);
    if (SNES_UP & ~state && SNES_LEFT & ~state) setHatSwitch(0, 315);
    sendState();
    delay(25);
}