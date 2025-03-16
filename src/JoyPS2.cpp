#include "JoyPS2.h"


static uint8_t enter_config[]={0x01,0x43,0x00,0x01,0x00};
static uint8_t set_mode[]={0x01,0x44,0x00,0x01,0x03,0x00,0x00,0x00,0x00};
static uint8_t set_bytes_large[]={0x01,0x4F,0x00,0xFF,0xFF,0x03,0x00,0x00,0x00};
static uint8_t exit_config[]={0x01,0x43,0x00,0x00,0x5A,0x5A,0x5A,0x5A,0x5A};
static uint8_t enable_rumble[]={0x01,0x4D,0x00,0x00,0x01};
static uint8_t type_read[]={0x01,0x45,0x00,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A};


JoyPS2::JoyPS2(uint8_t clk, uint8_t cmd, uint8_t att, uint8_t dat, bool pressures, bool rumble):
        en_Pressures(pressures), en_Rumble(rumble) {
    _clk_mask = digitalPinToBitMask(clk);
    _clk_oreg = portOutputRegister(digitalPinToPort(clk));
    _cmd_mask = digitalPinToBitMask(cmd);
    _cmd_oreg = portOutputRegister(digitalPinToPort(cmd));
    _att_mask = digitalPinToBitMask(att);
    _att_oreg = portOutputRegister(digitalPinToPort(att));
    _dat_mask = digitalPinToBitMask(dat);
    _dat_ireg = portInputRegister(digitalPinToPort(dat));
    pinMode(clk, OUTPUT); //configure ports
    pinMode(att, OUTPUT);
    pinMode(cmd, OUTPUT);
    pinMode(dat, INPUT_PULLUP); 
}


uint8_t JoyPS2::begin() {
    uint8_t temp[sizeof(type_read)];
    delay(1000);
    CMD_SET();
    CLK_SET();
    read_gamepad();  // New error checking.
    read_gamepad();  // Fead gamepad a few times to see if it's talking (41, 73 or 79)
   	if(PS2data[1] != 0x41 && PS2data[1] != 0x42 && PS2data[1] != 0x73 && PS2data[1] != 0x79){ 
       Serial.println("Controller mode not matched or no controller found");
       Serial.print("Expected 0x41, 0x42, 0x73 or 0x79, but got ");
       Serial.println(PS2data[1], HEX);
       _error = 1;
       return 1;
   	}
   	read_delay = 1;  //try setting mode, increasing delays if need be.
   	for(int y = 0; y <= 10; y++) {
        sendCommandString(enter_config, sizeof(enter_config)); //start config run
        delayMicroseconds(CTRL_BYTE_DELAY);  //read type
        CMD_SET();
        CLK_SET();
        ATT_CLR(); // low enable joystick
        delayMicroseconds(CTRL_BYTE_DELAY);
        for (int i = 0; i<9; i++) {
            temp[i] = _gamepad_shiftinout(type_read[i]);
        }
        ATT_SET(); // HI disable joystick
        controller_type = temp[3];
        sendCommandString(set_mode, sizeof(set_mode));
        if(en_Rumble){ sendCommandString(enable_rumble, sizeof(enable_rumble)); }
        if(en_Pressures){ sendCommandString(set_bytes_large, sizeof(set_bytes_large)); }
        sendCommandString(exit_config, sizeof(exit_config));
        read_gamepad();
        if(en_Pressures){
            if(PS2data[1] == 0x79) break;
            if(PS2data[1] == 0x73) {
                _error = 3;
                return 3;
            }
		}
		if(PS2data[1] == 0x73) break;
        if(y == 10){
        	#ifdef PS2X_DEBUG
            Serial.println("Controller not accepting commands");
            Serial.print("mode still set at");
            Serial.println(PS2data[1], HEX);
           	#endif
            _error = 2;
           	return 2; //exit function with error
		}
		read_delay += 1; //add 1ms to read_delay
	}
    delay(1000);
    _error = 0;
	return 0; //no error if here
}


bool JoyPS2::NewButtonState() {
    return ((last_buttons ^ buttons) > 0);
}


bool JoyPS2::NewButtonState(unsigned int button) {
    return (((last_buttons ^ buttons) & button) > 0);
}


bool JoyPS2::ButtonPressed(unsigned int button) {
    return(NewButtonState(button) & Button(button));
}


bool JoyPS2::ButtonReleased(unsigned int button) {
    return((NewButtonState(button)) & ((~last_buttons & button) > 0));
}


bool JoyPS2::Button(uint16_t button) {
    return ((~buttons & button) > 0);
}


unsigned int JoyPS2::ButtonDataByte() {
    return (~buttons);
}


uint8_t JoyPS2::Analog(uint8_t button) {
    return PS2data[button];
}


unsigned char JoyPS2::_gamepad_shiftinout (char byte) {
    unsigned char tmp = 0;
    for(unsigned char i=0;i<8;i++) {
        if(CHK(byte,i)) CMD_SET();
        else CMD_CLR();
        CLK_CLR();
        delayMicroseconds(CTRL_CLK);
        if(DAT_CHK()) bitSet(tmp,i);
        CLK_SET();
       }
    CMD_SET();
    delayMicroseconds(CTRL_BYTE_DELAY);
    return tmp;
}


void JoyPS2::read_gamepad() {
    read_gamepad(false, 0x00);
}

 
bool JoyPS2::read_gamepad(bool motor1, byte motor2) {
    double temp = millis() - last_read;
    if (temp > 1500) //waited to long
       reconfig();
    if(temp < read_delay)  //waited too short
       delay(read_delay - temp);
    if(motor2 != 0x00)
       motor2 = map(motor2,0,255,0x40,0xFF); //noting below 40 will make it spin
    uint8_t dword[9] = {0x01,0x42,0,motor1,motor2,0,0,0,0};
    uint8_t dword2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
 
    // Try a few times to get valid data...
    for (uint8_t RetryCnt = 0; RetryCnt < 5; RetryCnt++) {
        CMD_SET();
        CLK_SET();
        ATT_CLR(); // low enable joystick
    
        delayMicroseconds(CTRL_BYTE_DELAY);
        //Send the command to send button and joystick data;
        for (int i = 0; i<9; i++) {
            PS2data[i] = _gamepad_shiftinout(dword[i]);
        }
        if(PS2data[1] == 0x79) {  //if controller is in full data return mode, get the rest of data
            for (int i = 0; i<12; i++) {
                PS2data[i+9] = _gamepad_shiftinout(dword2[i]);
            }
        }
        ATT_SET(); // HI disable joystick
        // Check to see if we received valid data or not.  
        // We should be in analog mode for our data to be valid (analog == 0x7_)
        if ((PS2data[1] & 0xf0) == 0x70)
            break;
        // If we got to here, we are not in analog mode, try to recover...
        reconfig(); // try to get back into Analog mode.
        delay(read_delay);
    }
    // If we get here and still not in analog mode (=0x7_), try increasing the read_delay...
    if ((PS2data[1] & 0xf0) != 0x70) {
		if (read_delay < 10)
			read_delay++;   // see if this helps out...
    }
    #ifdef JOY_COM_DEBUG
        Serial.print("OUT:IN ");
        for(int i=0; i<9; i++){
            Serial.print(dword[i], HEX);
            Serial.print(":");
            Serial.print(PS2data[i], HEX);
            Serial.print(" ");
        }
        for (int i = 0; i<12; i++) {
            Serial.print(dword2[i], HEX);
            Serial.print(":");
            Serial.print(PS2data[i+9], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    #endif
    last_buttons = buttons; //store the previous buttons states
    buttons = *(uint16_t*)(PS2data+3);   //store as one value for multiple functions
    last_read = millis();
    return ((PS2data[1] & 0xf0) == 0x70);  // 1 = OK = analog mode - 0 = NOK
}


void JoyPS2::sendCommandString(byte string[], byte len) {
    #ifdef JOY_COM_DEBUG
        byte temp[len];
        ATT_CLR(); // low enable joystick
        delayMicroseconds(CTRL_BYTE_DELAY);
        for (int y=0; y < len; y++)
            temp[y] = _gamepad_shiftinout(string[y]);
        ATT_SET(); //high disable joystick
        delay(read_delay); //wait a few
        Serial.println("OUT:IN Configure");
        for(int i=0; i<len; i++) {
            Serial.print(string[i], HEX);
            Serial.print(":");
            Serial.print(temp[i], HEX);
            Serial.print(" ");
        }
        Serial.println("");
    #else
        ATT_CLR(); // low enable joystick
        delayMicroseconds(CTRL_BYTE_DELAY);
        for (int y=0; y < len; y++)
              _gamepad_shiftinout(string[y]);
        ATT_SET(); //high disable joystick
        delay(read_delay);  //wait a few
    #endif
}
    

uint8_t JoyPS2::readType() {
    Serial.print("Controller_type: ");
    Serial.println(controller_type, HEX);
    if(controller_type == 0x03)
        return 1;
    else if(controller_type == 0x01 && PS2data[1] == 0x42)
        return 4;
    else if(controller_type == 0x01 && PS2data[1] != 0x42)
        return 2;
    else if(controller_type == 0x0C)  
        return 3;  //2.4G Wireless Dual Shock PS2 Game Controller
    return 0;
}
    

void JoyPS2::enableRumble() {
    sendCommandString(enter_config, sizeof(enter_config));
    sendCommandString(enable_rumble, sizeof(enable_rumble));
    sendCommandString(exit_config, sizeof(exit_config));
    en_Rumble = true;
}
    

bool JoyPS2::enablePressures() {
    sendCommandString(enter_config, sizeof(enter_config));
    sendCommandString(set_bytes_large, sizeof(set_bytes_large));
    sendCommandString(exit_config, sizeof(exit_config));
    read_gamepad();
    read_gamepad();
    if(PS2data[1] != 0x79)
        return false;
    en_Pressures = true;
    return true;
}
    

void JoyPS2::reconfig(){
    sendCommandString(enter_config, sizeof(enter_config));
    sendCommandString(set_mode, sizeof(set_mode));
    if (en_Rumble)
        sendCommandString(enable_rumble, sizeof(enable_rumble));
    if (en_Pressures)
        sendCommandString(set_bytes_large, sizeof(set_bytes_large));
    sendCommandString(exit_config, sizeof(exit_config));
}
    

inline void  JoyPS2::CLK_SET(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_clk_oreg |= _clk_mask;
    SREG = old_sreg;
}


inline void  JoyPS2::CLK_CLR(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_clk_oreg &= ~_clk_mask;
    SREG = old_sreg;
}


inline void  JoyPS2::CMD_SET(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_cmd_oreg |= _cmd_mask; // SET(*_cmd_oreg,_cmd_mask);
    SREG = old_sreg;
}


inline void  JoyPS2::CMD_CLR(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_cmd_oreg &= ~_cmd_mask; // SET(*_cmd_oreg,_cmd_mask);
    SREG = old_sreg;
}


inline void  JoyPS2::ATT_SET(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_att_oreg |= _att_mask ;
    SREG = old_sreg;
}


inline void JoyPS2::ATT_CLR(void) {
    register uint8_t old_sreg = SREG;
    cli();
    *_att_oreg &= ~_att_mask;
    SREG = old_sreg;
}


inline bool JoyPS2::DAT_CHK(void) {
    return (*_dat_ireg & _dat_mask) ? true : false;
}


inline bool JoyPS2::loop(void) {
    if(_error == 1) //skip loop if no controller found
        return; 
    if(type == 2){ //Guitar Hero Controller
        joystick.read_gamepad();          //read controller 
        if(joystick.ButtonPressed(GREEN_FRET))
            pressButton(1);
            Serial.println("Green Fret Pressed");
        if(joystick.ButtonPressed(RED_FRET))
            Serial.println("Red Fret Pressed");
        if(joystick.ButtonPressed(YELLOW_FRET))
            Serial.println("Yellow Fret Pressed");
        if(joystick.ButtonPressed(BLUE_FRET))
            Serial.println("Blue Fret Pressed");
        if(joystick.ButtonPressed(ORANGE_FRET))
            Serial.println("Orange Fret Pressed");
        if(joystick.ButtonPressed(STAR_POWER))
            Serial.println("Star Power Command");
        if(joystick.Button(UP_STRUM))          //will be TRUE as long as button is pressed
            Serial.println("Up Strum");
        if(joystick.Button(DOWN_STRUM))
            Serial.println("DOWN Strum");
        if(joystick.Button(PSB_START))                   //will be TRUE as long as button is pressed
            Serial.println("Start is being held");
        if(joystick.Button(PSB_SELECT))
            Serial.println("Select is being held");
        if(joystick.Button(ORANGE_FRET)) {
            Serial.print("Wammy Bar Position:");
            Serial.println(joystick.Analog(WHAMMY_BAR), DEC); 
        } 
    }
    else { //DualShock Controller
        joystick.read_gamepad(false, vibrate);          //read controller and set large motor to spin at 'vibrate' speed
        if(joystick.Button(PSB_START))                   //will be TRUE as long as button is pressed
            Serial.println("Start is being held");
        if(joystick.Button(PSB_SELECT))
            Serial.println("Select is being held"); 
        if(joystick.Button(PSB_PAD_UP)) {         //will be TRUE as long as button is pressed
            Serial.print("Up held this hard: ");
            Serial.println(joystick.Analog(PSAB_PAD_UP), DEC);
        }
        if(joystick.Button(PSB_PAD_RIGHT)){
            Serial.print("Right held this hard: ");
            Serial.println(joystick.Analog(PSAB_PAD_RIGHT), DEC);
        }
        if(joystick.Button(PSB_PAD_LEFT)){
            Serial.print("LEFT held this hard: ");
            Serial.println(joystick.Analog(PSAB_PAD_LEFT), DEC);
        }
        if(joystick.Button(PSB_PAD_DOWN)){
            Serial.print("DOWN held this hard: ");
            Serial.println(joystick.Analog(PSAB_PAD_DOWN), DEC);
        }
        vibrate = joystick.Analog(PSAB_BLUE);        //this will set the large motor vibrate speed based on 
                                                //how hard you press the blue (X) button    
        if (joystick.NewButtonState()) {
            if(joystick.Button(PSB_L3))
                Serial.println("L3 pressed");
            if(joystick.Button(PSB_R3))
                Serial.println("R3 pressed");
            if(joystick.Button(PSB_L2))
                Serial.println("L2 pressed");
            if(joystick.Button(PSB_R2))
                Serial.println("R2 pressed");
            if(joystick.Button(PSB_GREEN))
                Serial.println("Triangle pressed");
        }    
        if(joystick.ButtonPressed(PSB_RED))             //will be TRUE if button was JUST pressed
            Serial.println("Circle just pressed");
        if(joystick.ButtonReleased(PSB_PINK))             //will be TRUE if button was JUST released
            Serial.println("Square just released");     
        if(joystick.NewButtonState(PSB_BLUE))            //will be TRUE if button was JUST pressed OR released
            Serial.println("X just changed");    
        if(joystick.Button(PSB_L1) || joystick.Button(PSB_R1)) {
            Serial.print("Stick Values:");
            Serial.print(joystick.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
            Serial.print(",");
            Serial.print(joystick.Analog(PSS_LX), DEC); 
            Serial.print(",");
            Serial.print(joystick.Analog(PSS_RY), DEC); 
            Serial.print(",");
            Serial.println(joystick.Analog(PSS_RX), DEC); 
        } 
    }
    sendState();
    delay(50);
}