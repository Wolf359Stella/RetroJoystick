/*
  Copyright (c) 2015-2017, Matthew Heironimus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Modified by Matthew Heironimus to support HID Report Descriptors to be in 
  standard RAM in addition to program memory (PROGMEM).

   Copyright (c) 2015, Arduino LLC
   Original code (pre-library): Copyright (c) 2011, Peter Barrett

   Permission to use, copy, modify, and/or distribute this software for
   any purpose with or without fee is hereby granted, provided that the
   above copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
   BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
   OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
   ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
   SOFTWARE.
*/

#ifndef JOYSTICK_h
#define JOYSTICK_h

#include <stdint.h>
#include <Arduino.h>

#if ARDUINO < 10606
    #error The Retro Joystick library requires Arduino IDE 1.6.6 or greater.
#endif
#if !defined(USBCON)
    #error The Retro Joystick library can only be used with a USB MCU (e.g. Arduino Leonardo, Arduino Micro, etc.).
#endif

#ifdef _VARIANT_ARDUINO_DUE_X_
    // The following values are the same as AVR's USBAPI.h
    // Reproduced here because SAM doesn't have these in its own USBAPI.H
    #define USB_EP_SIZE 64
    #define TRANSFER_PGM 0x80
    #include "USB/PluggableUSB.h"
#else
    #include "PluggableUSB.h"
#endif



// DYNAMIC_HID 'Driver'
#define DYNAMIC_HID_GET_REPORT        0x01
#define DYNAMIC_HID_GET_IDLE          0x02
#define DYNAMIC_HID_GET_PROTOCOL      0x03
#define DYNAMIC_HID_SET_REPORT        0x09
#define DYNAMIC_HID_SET_IDLE          0x0A
#define DYNAMIC_HID_SET_PROTOCOL      0x0B

#define DYNAMIC_HID_DESCRIPTOR_TYPE         0x21
#define DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE      0x22
#define DYNAMIC_HID_PHYSICAL_DESCRIPTOR_TYPE    0x23

// HID subclass HID1.11 Page 8 4.2 Subclass
#define DYNAMIC_HID_SUBCLASS_NONE 0
#define DYNAMIC_HID_SUBCLASS_BOOT_INTERFACE 1

// HID Keyboard/Mouse bios compatible protocols HID1.11 Page 9 4.3 Protocols
#define DYNAMIC_HID_PROTOCOL_NONE 0
#define DYNAMIC_HID_PROTOCOL_KEYBOARD 1
#define DYNAMIC_HID_PROTOCOL_MOUSE 2

// Normal or bios protocol (Keyboard/Mouse) HID1.11 Page 54 7.2.5 Get_Protocol Request
// "protocol" variable is used for this purpose.
#define DYNAMIC_HID_BOOT_PROTOCOL	0
#define DYNAMIC_HID_REPORT_PROTOCOL	1

// HID Request Type HID1.11 Page 51 7.2.1 Get_Report Request
#define DYNAMIC_HID_REPORT_TYPE_INPUT   1
#define DYNAMIC_HID_REPORT_TYPE_OUTPUT  2
#define DYNAMIC_HID_REPORT_TYPE_FEATURE 3

typedef struct {
    uint8_t len;      // 9
    uint8_t dtype;    // 0x21
    uint8_t addr;
    uint8_t versionL; // 0x101
    uint8_t versionH; // 0x101
    uint8_t country;
    uint8_t desctype; // 0x22 report
    uint8_t descLenL;
    uint8_t descLenH;
} DYNAMIC_HIDDescDescriptor;


typedef struct {
    InterfaceDescriptor hid;
    DYNAMIC_HIDDescDescriptor   desc;
    EndpointDescriptor  in;
} DYNAMIC_HIDDescriptor;


class DynamicHIDSubDescriptor {
    public:
        DynamicHIDSubDescriptor *next = NULL;
        DynamicHIDSubDescriptor(const void *d, const uint16_t l, const bool ipm = true) 
            : data(d), length(l), inProgMem(ipm) { }

        const void* data;
        const uint16_t length;
        const bool inProgMem;
};


class DynamicHID_ : public PluggableUSBModule {
    public:
        DynamicHID_(void);
        int begin(void);
        int SendReport(uint8_t id, const void* data, int len);
        void AppendDescriptor(DynamicHIDSubDescriptor* node);
        void print();

    protected:
        // Implementation of the PluggableUSBModule
        int getInterface(uint8_t* interfaceCount);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
        uint8_t getShortName(char* name);

    private:
        #ifdef _VARIANT_ARDUINO_DUE_X_
        uint32_t epType[1];
        #else
        uint8_t epType[1];
        #endif

        DynamicHIDSubDescriptor* rootNode;
        uint16_t descriptorSize;

        uint8_t protocol;
        uint8_t idle;
};


// Replacement for global singleton.
// This function prevents static-initialization-order-fiasco
// https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
DynamicHID_& DynamicHID();

#define D_HIDREPORT(length) { 9, 0x21, 0x01, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }

#define JOYSTICK_DEFAULT_REPORT_ID         0x03
#define JOYSTICK_DEFAULT_BUTTON_COUNT        32
#define JOYSTICK_DEFAULT_AXIS_MINIMUM         0
#define JOYSTICK_DEFAULT_AXIS_MAXIMUM      1023
#define JOYSTICK_DEFAULT_SIMULATOR_MINIMUM    0
#define JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM 1023
#define JOYSTICK_DEFAULT_HATSWITCH_COUNT      2
#define JOYSTICK_HATSWITCH_COUNT_MAXIMUM      2
#define JOYSTICK_HATSWITCH_RELEASE           -1
#define JOYSTICK_TYPE_JOYSTICK             0x04
#define JOYSTICK_TYPE_GAMEPAD              0x05
#define JOYSTICK_TYPE_MULTI_AXIS           0x08

typedef struct {
    int32_t xMin = 0;
    int32_t xMax = 1023;
    int32_t yMin = 0;
    int32_t yMax = 1023;
    int32_t zMin = 0;
    int32_t zMax = 1023;
} Axis;

class Joystick_ {
private:

    // Joystick State
    int32_t   _xAxis;
    int32_t   _yAxis;
    int32_t   _zAxis;
    int32_t   _xAxisRotation;
    int32_t   _yAxisRotation;
    int32_t   _zAxisRotation;
    int32_t   _throttle;
    int32_t   _rudder;
    int32_t   _accelerator;
    int32_t   _brake;
    int32_t   _steering;
    int16_t    _hatSwitchValues[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
    uint8_t   *_buttonValues = NULL;

    // Joystick Settings
    bool     _autoSendState;
    uint8_t  _buttonCount;
    uint8_t  _buttonValuesArraySize = 0;
    uint8_t  _hatSwitchCount;
    uint8_t  _includeAxisFlags;
    uint8_t  _includeSimulatorFlags;
    int32_t  _xAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _xAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _yAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _yAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _zAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _zAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _rxAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _rxAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _ryAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _ryAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _rzAxisMinimum = JOYSTICK_DEFAULT_AXIS_MINIMUM;
    int32_t  _rzAxisMaximum = JOYSTICK_DEFAULT_AXIS_MAXIMUM;
    int32_t  _rudderMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
    int32_t  _rudderMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
    int32_t  _throttleMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
    int32_t  _throttleMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
    int32_t  _acceleratorMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
    int32_t  _acceleratorMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
    int32_t  _brakeMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
    int32_t  _brakeMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;
    int32_t  _steeringMinimum = JOYSTICK_DEFAULT_SIMULATOR_MINIMUM;
    int32_t  _steeringMaximum = JOYSTICK_DEFAULT_SIMULATOR_MAXIMUM;

    uint8_t   _hidReportId;
    uint8_t   _hidReportSize; 

protected:
    int buildAndSet16BitValue(bool includeValue, int32_t value, int32_t valueMinimum, 
                                int32_t valueMaximum, int32_t actualMinimum, 
                                int32_t actualMaximum, uint8_t dataLocation[]);
    int buildAndSetAxisValue(bool includeAxis, int32_t axisValue, int32_t axisMinimum, 
                                int32_t axisMaximum, uint8_t dataLocation[]);
    int buildAndSetSimulationValue(bool includeValue, int32_t value, int32_t valueMinimum, 
                                    int32_t valueMaximum, uint8_t dataLocation[]);

public:
    Joystick_(
        uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID,
        uint8_t joystickType = JOYSTICK_TYPE_JOYSTICK,
        uint8_t buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT,
        uint8_t hatSwitchCount = JOYSTICK_DEFAULT_HATSWITCH_COUNT,
        bool includeXAxis = true,
        bool includeYAxis = true,
        bool includeZAxis = true,
        bool includeRxAxis = true,
        bool includeRyAxis = true,
        bool includeRzAxis = true,
        bool includeRudder = true,
        bool includeThrottle = true,
        bool includeAccelerator = true,
        bool includeBrake = true,
        bool includeSteering = true);

    void begin(bool initAutoSendState = true);
    void end();
    
    // Set Range Functions
    inline void setXAxisRange(int32_t minimum, int32_t maximum) {
        _xAxisMinimum = minimum;
        _xAxisMaximum = maximum;
    }
    inline void setYAxisRange(int32_t minimum, int32_t maximum) {
        _yAxisMinimum = minimum;
        _yAxisMaximum = maximum;
    }
    inline void setZAxisRange(int32_t minimum, int32_t maximum) {
        _zAxisMinimum = minimum;
        _zAxisMaximum = maximum;
    }
    inline void setRxAxisRange(int32_t minimum, int32_t maximum) {
        _rxAxisMinimum = minimum;
        _rxAxisMaximum = maximum;
    }
    inline void setRyAxisRange(int32_t minimum, int32_t maximum) {
        _ryAxisMinimum = minimum;
        _ryAxisMaximum = maximum;
    }
    inline void setRzAxisRange(int32_t minimum, int32_t maximum) {
        _rzAxisMinimum = minimum;
        _rzAxisMaximum = maximum;
    }
    inline void setRudderRange(int32_t minimum, int32_t maximum) {
        _rudderMinimum = minimum;
        _rudderMaximum = maximum;
    }
    inline void setThrottleRange(int32_t minimum, int32_t maximum) {
        _throttleMinimum = minimum;
        _throttleMaximum = maximum;
    }
    inline void setAcceleratorRange(int32_t minimum, int32_t maximum) {
        _acceleratorMinimum = minimum;
        _acceleratorMaximum = maximum;
    }
    inline void setBrakeRange(int32_t minimum, int32_t maximum) {
        _brakeMinimum = minimum;
        _brakeMaximum = maximum;
    }
    inline void setSteeringRange(int32_t minimum, int32_t maximum) {
        _steeringMinimum = minimum;
        _steeringMaximum = maximum;
    }

    // Set Axis Values
    void setXAxis(int32_t value);
    void setYAxis(int32_t value);
    void setZAxis(int32_t value);
    void setRxAxis(int32_t value);
    void setRyAxis(int32_t value);
    void setRzAxis(int32_t value);

    // Set Simulation Values
    void setRudder(int32_t value);
    void setThrottle(int32_t value);
    void setAccelerator(int32_t value);
    void setBrake(int32_t value);
    void setSteering(int32_t value);

    void setButton(uint8_t button, uint8_t value);
    void pressButton(uint8_t button);
    void releaseButton(uint8_t button);

    void setHatSwitch(int8_t hatSwitch, int16_t value);

    void sendState();
};


class JoyClassic: public Joystick_ {
    private:
        uint8_t up;
        uint8_t down;
        uint8_t left;
        uint8_t right;
        uint8_t button;
    public:
        JoyClassic(uint8_t up, uint8_t down, uint8_t left, 
                   uint8_t right, uint8_t button);
        /**
        * Main loop for the joystick.
        */
        void loop();
};


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

typedef struct {
    int pin;
    int lowFlag;
    int highFlag;
    int pulse3Flag;
} MegadriveInput;


class JoySegaMegadrive: public Joystick_ {
    private:
        uint16_t _currentState[2] = { 0, 0 };
        uint16_t _lastState[2] = { 0, 0 };
        uint8_t _select[2] = { 8, 9 };
        uint8_t _players;
        bool _sixButtonMode[2] = { false, false };

        void read6buttons(int player);
        void read3buttons(int player);
        void sendStates();
    public:
        static const int ON = 1;
        static const int UP = 2;
        static const int DOWN = 4;
        static const int LEFT = 8;
        static const int RIGHT = 16;
        static const int START = 32;
        static const int A = 64;
        static const int B = 128;
        static const int C = 256;
        static const int X = 512;
        static const int Y = 1024;
        static const int Z = 2048;
        static const int MODE = 4096;
        MegadriveInput _inputMap[12] = {
            { 2,  UP,    UP,     Z    }, // P0 DB9 Pin 1 (UP/Z)
            { 3,  DOWN,  DOWN,   Y    }, // P0 DB9 Pin 2 (DOWN/Y)
            { 4,  ON,    LEFT,   X    }, // P0 DB9 Pin 3 (LEFT/X)
            { 5,  ON,    RIGHT,  MODE }, // P0 DB9 Pin 4 (RIGHT/MODE)
            { 6,  A,     B,      0    }, // P0 DB9 Pin 6 (A/B)
            { 7,  START, C,      0    }, // P0 DB9 Pin 9 (START/C)
            { A0, UP,    UP,     Z    }, // P1 DB9 Pin 1 (UP/Z)
            { A1, DOWN,  DOWN,   Y    }, // P1 DB9 Pin 2 (DOWN/Y)
            { A2, ON,    LEFT,   X    }, // P1 DB9 Pin 3 (LEFT/X)
            { A3, ON,    RIGHT,  MODE }, // P1 DB9 Pin 4 (RIGHT/MODE)
            { A4, A,     B,      0    }, // P1 DB9 Pin 6 (A/B)
            { A5, START, C,      0    }  // P1 DB9 Pin 9 (START/C)
        };
        JoySegaMegadrive(const int players);
        /**
        * Main loop for the joystick.
        */
        void loop();

};


class JoySegaMastersystem: public Joystick_ {
    public:
        JoySegaMastersystem(uint8_t up, uint8_t down, uint8_t left, 
                            uint8_t right, uint8_t b1, uint8_t b2);
        /**
        * Main loop for the joystick.
        */
        void loop();
            
};


#define SNES_B       1      //000000000001
#define SNES_Y       2      //000000000010
#define SNES_SELECT  4      //000000000100
#define SNES_START   8      //000000001000
#define SNES_UP      16     //000000010000
#define SNES_DOWN    32     //000000100000
#define SNES_LEFT    64     //000001000000
#define SNES_RIGHT   128    //000010000000
#define SNES_A       256    //000100000000
#define SNES_X       512    //001000000000
#define SNES_L       1024   //010000000000
#define SNES_R       2048   //100000000000


class JoySNES: public Joystick_ {
    private:
        uint8_t _clock;
        uint8_t _latch;
        uint8_t _data;
    public:
        JoySNES(uint8_t clock, uint8_t latch, uint8_t data);
        /**
        * Main loop for the joystick.
        */
        void loop();
};


#endif // JOYSTICK_h
