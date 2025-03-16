#include <JoyPS2.h>

#define PS2_DAT        13   
#define PS2_CMD        11
#define PS2_SEL        9
#define PS2_CLK        7
#define pressures   false
#define rumble      false


JoyPS2 joystick(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
int error = 0;
byte type = 0;
byte vibrate = 0;


void setup() {
    Serial.begin(9600);
    error = joystick.begin();  

    if(error == 0){
        Serial.println("Found Controller, configured successful ");
    } else if(error == 1)
        Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
    else if(error == 2)
        Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
    else if(error == 3)
        Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
    type = joystick.readType(); 
    switch(type) {
        case 0:
            Serial.print("Unknown Controller type found ");
            break;
        case 1:
            Serial.print("DualShock Controller found ");
            break;
        case 2:
            Serial.print("GuitarHero Controller found ");
            break;
        case 3:
            Serial.print("Wireless Sony DualShock Controller found ");
            break;
    }
}
 

void loop() {
    joystick.loop();
}
 