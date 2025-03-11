#include <RetroJoystick.h>


JoySegaMegadrive joystick(2);
 

void setup() {
    joystick.begin(false);  
}
 

void loop() {
    joystick.loop();
}
 