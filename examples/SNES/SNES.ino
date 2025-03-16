#include <JoySNES.h>

#define CLOCK 6
#define LATCH 8
#define DATA 10
JoySNES joystick(CLOCK, LATCH, DATA);


void setup(){}


void loop(){
    joystick.loop();
}

