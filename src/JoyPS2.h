#pragma once
#include "RetroJoystick.h"

#define CTRL_CLK        4
#define CTRL_BYTE_DELAY 3
#define CHK(x,y) (x & (1<<y))

class JoyPS2: public Joystick_ {
    public:
        JoyPS2(uint8_t, uint8_t, uint8_t, uint8_t, bool, bool);
        bool Button(uint16_t);                //will be TRUE if button is being pressed
        unsigned int ButtonDataByte();
        bool NewButtonState();
        bool NewButtonState(unsigned int);    //will be TRUE if button was JUST pressed OR released
        bool ButtonPressed(unsigned int);     //will be TRUE if button was JUST pressed
        bool ButtonReleased(unsigned int);    //will be TRUE if button was JUST released
        void read_gamepad();
        bool  read_gamepad(bool, uint8_t);
        uint8_t readType();
        uint8_t begin(bool);
        void enableRumble();
        bool enablePressures();
        uint8_t Analog(uint8_t);
        void reconfig();

    private:
        inline void CLK_SET(void);
        inline void CLK_CLR(void);
        inline void CMD_SET(void);
        inline void CMD_CLR(void);
        inline void ATT_SET(void);
        inline void ATT_CLR(void);
        inline bool DAT_CHK(void);
        
        unsigned char _gamepad_shiftinout (char);
        unsigned char PS2data[21];
        void sendCommandString(byte*, byte);
        unsigned char i;
        unsigned int last_buttons;
        unsigned int buttons;
      
        uint8_t maskToBitNum(uint8_t);
        uint8_t _clk_mask; 
        volatile uint8_t *_clk_oreg;
        uint8_t _cmd_mask; 
        volatile uint8_t *_cmd_oreg;
        uint8_t _att_mask; 
        volatile uint8_t *_att_oreg;
        uint8_t _dat_mask; 
        volatile uint8_t *_dat_ireg;
      
        unsigned long last_read;
        byte read_delay;
        byte controller_type;
        boolean en_Rumble;
        boolean en_Pressures;
};
