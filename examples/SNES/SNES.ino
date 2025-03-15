#include <JoySNES.h>

#define CLOCK 6
#define LATCH 7
#define DATA 12
JoySNES joystick(CLOCK, LATCH, DATA);


void setup(){}


void loop(){
    joystick.loop();
}

