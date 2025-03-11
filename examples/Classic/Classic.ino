#include <RetroJoystick.h>

JoyClassic joystickw(2, 3, 4, 5, 6);
 

void setup() {
    joystick.begin(false);
}


void loop() {
    joystick.loop();
}
