#include <retro_joystick.h>

#define CLOCK 6
#define LATCH 7
#define DATA 12
JoySNES joystick(CLOCK, LATCH, DATA);


void setup(){
    joystick.begin(false);
}


void loop(){
    joystick.loop();
}

