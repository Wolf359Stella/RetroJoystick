#include <RetroJoystick.h>

#define UP = 2;
#define DOWN = 3;
#define LEFT = 4;
#define RIGHT = 5;
#define B1 = 6;
#define B2 = 7;
JoySegaMastersystem joystick(UP, DOWN, LEFT, RIGHT, B1, B2);


void setup() {}


void loop() {
    joystick.loop();
}
