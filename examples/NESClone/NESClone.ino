#include <JoyNESClone.h>


const int PIN_CLOCK = 6;
const int PIN_LATCH = 8;
const int PIN_DATA = 10;


JoyNESClone joystick(PIN_CLOCK, PIN_LATCH, PIN_DATA);


void setup() {}


void loop(){
    joystick.loop();
}
