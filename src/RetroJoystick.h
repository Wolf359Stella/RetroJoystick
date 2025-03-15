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
#define JOYSTICK_DEFAULT_HATSWITCH_COUNT      2
#define JOYSTICK_HATSWITCH_COUNT_MAXIMUM      2
#define JOYSTICK_HATSWITCH_RELEASE           -1
#define JOYSTICK_TYPE_JOYSTICK             0x04
#define JOYSTICK_TYPE_GAMEPAD              0x05
#define JOYSTICK_TYPE_MULTI_AXIS           0x08

typedef struct {
    int32_t val = 0;
    int32_t min = 0;
    int32_t max = 1023;
} Value;


typedef struct {
    uint8_t mask; 
    volatile uint8_t *ptr;
} Reg_;


typedef struct __attribute__ ((packed)) {
    bool XAxis: 1;
    bool YAxis: 1;
    bool ZAxis: 1;
    bool RxAxis: 1;
    bool RyAxis: 1;
    bool RzAxis: 1;
    bool Rudder: 1;
    bool Throttle: 1;
    bool Accelerator: 1;
    bool Brake: 1;
    bool Steering: 1;
} IncludeFlags;


class Joystick_ {
    private:

        // Joystick State
        int16_t    _hatSwitchValues[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
        uint8_t   *_buttonValues = NULL;

        // Joystick Settings
        uint8_t  _buttonCount;
        uint8_t  _buttonValuesArraySize = 0;
        uint8_t  _hatSwitchCount;
        IncludeFlags _include;

        uint8_t   _hidReportId;
        uint8_t   _hidReportSize; 

    protected:
        int buildAndSet16BitValue(bool includeValue, Value value, int32_t actualMinimum, 
                                    int32_t actualMaximum, uint8_t dataLocation[]);
        int buildAndSetAxisValue(bool includeAxis, Value value, uint8_t dataLocation[]);
        int buildAndSetSimulationValue(bool includeValue, Value value, uint8_t dataLocation[]);

    public:
        Value _xAxis;
        Value _yAxis;
        Value _zAxis;
        Value _xAxisRotation;
        Value _yAxisRotation;
        Value _zAxisRotation;
        Value _throttle;
        Value _rudder;
        Value _accelerator;
        Value _brake;
        Value _steering;

        Joystick_(
            uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID,
            uint8_t joystickType = JOYSTICK_TYPE_JOYSTICK,
            uint8_t buttonCount = JOYSTICK_DEFAULT_BUTTON_COUNT,
            uint8_t hatSwitchCount = JOYSTICK_DEFAULT_HATSWITCH_COUNT,
            IncludeFlags includes = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

        inline bool reg_check(Reg_ reg) {
            return (*reg.ptr & reg.mask) ? true : false;
        }

        void pressButton(uint8_t button);
        void releaseButton(uint8_t button);
        void setHatSwitch(int8_t hatSwitch, int16_t value);
        void sendState();
};


#endif // JOYSTICK_h
