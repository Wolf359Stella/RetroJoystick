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

#include "retro_joystick.h"


#ifdef _VARIANT_ARDUINO_DUE_X_
	#define USB_SendControl USBD_SendControl
	#define USB_Send USBD_Send
#endif

#define JOYSTICK_REPORT_ID_INDEX 7
#define JOYSTICK_AXIS_MINIMUM 0
#define JOYSTICK_AXIS_MAXIMUM 65535
#define JOYSTICK_SIMULATOR_MINIMUM 0
#define JOYSTICK_SIMULATOR_MAXIMUM 65535

#define JOYSTICK_INCLUDE_X_AXIS  B00000001
#define JOYSTICK_INCLUDE_Y_AXIS  B00000010
#define JOYSTICK_INCLUDE_Z_AXIS  B00000100
#define JOYSTICK_INCLUDE_RX_AXIS B00001000
#define JOYSTICK_INCLUDE_RY_AXIS B00010000
#define JOYSTICK_INCLUDE_RZ_AXIS B00100000

#define JOYSTICK_INCLUDE_RUDDER      B00000001
#define JOYSTICK_INCLUDE_THROTTLE    B00000010
#define JOYSTICK_INCLUDE_ACCELERATOR B00000100
#define JOYSTICK_INCLUDE_BRAKE       B00001000
#define JOYSTICK_INCLUDE_STEERING    B00010000


DynamicHID_& DynamicHID() {
	static DynamicHID_ obj;
	return obj;
}


int DynamicHID_::getInterface(uint8_t* interfaceCount) {
	*interfaceCount += 1; // uses 1
	DYNAMIC_HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, DYNAMIC_HID_SUBCLASS_NONE, DYNAMIC_HID_PROTOCOL_NONE),
		D_HIDREPORT(descriptorSize),
		D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
	};
	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
}


int DynamicHID_::getDescriptor(USBSetup& setup) {
	// Check if this is a HID Class Descriptor request
	if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
	if (setup.wValueH != DYNAMIC_HID_REPORT_DESCRIPTOR_TYPE) { return 0; }
	// In a HID Class Descriptor wIndex cointains the interface number
	if (setup.wIndex != pluggedInterface) { return 0; }
	int total = 0;
	DynamicHIDSubDescriptor* node;
	for (node = rootNode; node; node = node->next) {
		int res = USB_SendControl((node->inProgMem ? TRANSFER_PGM : 0), node->data, node->length);
		if (res == -1)
			return -1;
		total += res;
	}
	// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
	// due to the USB specs, but Windows and Linux just assumes its in report mode.
	protocol = DYNAMIC_HID_REPORT_PROTOCOL;
	return total;
}


uint8_t DynamicHID_::getShortName(char *name) {
	name[0] = 'H';
	name[1] = 'I';
	name[2] = 'D';
	name[3] = 'A' + (descriptorSize & 0x0F);
	name[4] = 'A' + ((descriptorSize >> 4) & 0x0F);
	return 5;
}


void DynamicHID_::AppendDescriptor(DynamicHIDSubDescriptor *node) {
	if (!rootNode) {
		rootNode = node;
	} else {
		DynamicHIDSubDescriptor *current = rootNode;
		while (current->next) {
			current = current->next;
		}
		current->next = node;
	}
	descriptorSize += node->length;
}


int DynamicHID_::SendReport(uint8_t id, const void* data, int len) {
	uint8_t p[len + 1];
	p[0] = id;
	memcpy(&p[1], data, len);
	return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, p, len + 1);
}


bool DynamicHID_::setup(USBSetup& setup) {
	if (pluggedInterface != setup.wIndex) {
		return false;
	}
	uint8_t request = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;
	if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
		if (request == DYNAMIC_HID_GET_REPORT) {
			// TODO: DYNAMIC_HID_GetReport();
			return true;
		}
		if (request == DYNAMIC_HID_GET_PROTOCOL) {
			// TODO: Send8(protocol);
			return true;
		}
		if (request == DYNAMIC_HID_GET_IDLE) {
			// TODO: Send8(idle);
		}
	}
	if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
		if (request == DYNAMIC_HID_SET_PROTOCOL) {
			// The USB Host tells us if we are in boot or report mode.
			// This only works with a real boot compatible device.
			protocol = setup.wValueL;
			return true;
		}
		if (request == DYNAMIC_HID_SET_IDLE) {
			idle = setup.wValueL;
			return true;
		}
		if (request == DYNAMIC_HID_SET_REPORT) {
			//uint8_t reportID = setup.wValueL;
			//uint16_t length = setup.wLength;
			//uint8_t data[length];
			// Make sure to not read more data than USB_EP_SIZE.
			// You can read multiple times through a loop.
			// The first byte (may!) contain the reportID on a multreport.
			//USB_RecvControl(data, length);
		}
	}
	return false;
}


DynamicHID_::DynamicHID_(void) : PluggableUSBModule(1, 1, epType),
                   rootNode(NULL), descriptorSize(0),
                   protocol(DYNAMIC_HID_REPORT_PROTOCOL), idle(1) {
	epType[0] = EP_TYPE_INTERRUPT_IN;
	PluggableUSB().plug(this);
}


int DynamicHID_::begin(void) {
	return 0;
}


Joystick_::Joystick_(
	uint8_t hidReportId,
	uint8_t joystickType,
    uint8_t buttonCount,
	uint8_t hatSwitchCount,
	bool includeXAxis,
	bool includeYAxis,
	bool includeZAxis,
	bool includeRxAxis,
	bool includeRyAxis,
	bool includeRzAxis,
	bool includeRudder,
	bool includeThrottle,
	bool includeAccelerator,
	bool includeBrake,
	bool includeSteering) {
    // Set the USB HID Report ID
    _hidReportId = hidReportId;

    // Save Joystick Settings
    _buttonCount = buttonCount;
	_hatSwitchCount = hatSwitchCount;
	_includeAxisFlags = 0;
	_includeAxisFlags |= (includeXAxis ? JOYSTICK_INCLUDE_X_AXIS : 0);
	_includeAxisFlags |= (includeYAxis ? JOYSTICK_INCLUDE_Y_AXIS : 0);
	_includeAxisFlags |= (includeZAxis ? JOYSTICK_INCLUDE_Z_AXIS : 0);
	_includeAxisFlags |= (includeRxAxis ? JOYSTICK_INCLUDE_RX_AXIS : 0);
	_includeAxisFlags |= (includeRyAxis ? JOYSTICK_INCLUDE_RY_AXIS : 0);
	_includeAxisFlags |= (includeRzAxis ? JOYSTICK_INCLUDE_RZ_AXIS : 0);
	_includeSimulatorFlags = 0;
	_includeSimulatorFlags |= (includeRudder ? JOYSTICK_INCLUDE_RUDDER : 0);
	_includeSimulatorFlags |= (includeThrottle ? JOYSTICK_INCLUDE_THROTTLE : 0);
	_includeSimulatorFlags |= (includeAccelerator ? JOYSTICK_INCLUDE_ACCELERATOR : 0);
	_includeSimulatorFlags |= (includeBrake ? JOYSTICK_INCLUDE_BRAKE : 0);
	_includeSimulatorFlags |= (includeSteering ? JOYSTICK_INCLUDE_STEERING : 0);
	
    // Build Joystick HID Report Description
	
	// Button Calculations
	uint8_t buttonsInLastByte = _buttonCount % 8;
	uint8_t buttonPaddingBits = 0;
	if (buttonsInLastByte > 0) {
		buttonPaddingBits = 8 - buttonsInLastByte;
	}
	
	// Axis Calculations
	uint8_t axisCount = (includeXAxis == true)
		+  (includeYAxis == true)
		+  (includeZAxis == true)
		+  (includeRxAxis == true)
		+  (includeRyAxis == true)
		+  (includeRzAxis == true);
		
	uint8_t simulationCount = (includeRudder == true)
		+ (includeThrottle == true)
		+ (includeAccelerator == true)
		+ (includeBrake == true)
		+ (includeSteering == true); 
		
    uint8_t tempHidReportDescriptor[150];
    int hidReportDescriptorSize = 0;

    // USAGE_PAGE (Generic Desktop)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    // USAGE (Joystick - 0x04; Gamepad - 0x05; Multi-axis Controller - 0x08)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
    tempHidReportDescriptor[hidReportDescriptorSize++] = joystickType;

    // COLLECTION (Application)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa1;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    // REPORT_ID (Default: 3)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;
    tempHidReportDescriptor[hidReportDescriptorSize++] = _hidReportId;
	
	if (_buttonCount > 0) {

		// USAGE_PAGE (Button)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

		// USAGE_MINIMUM (Button 1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// USAGE_MAXIMUM (Button 32)            
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29;
		tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_SIZE (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// REPORT_COUNT (# of buttons)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = _buttonCount;

		// UNIT_EXPONENT (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x55;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// UNIT (None)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

		if (buttonPaddingBits > 0) {
			
			// REPORT_SIZE (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// REPORT_COUNT (# of padding bits)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = buttonPaddingBits;
					
			// INPUT (Const,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
			
		} // Padding Bits Needed

	} // Buttons

	if ((axisCount > 0) || (_hatSwitchCount > 0)) {
	
		// USAGE_PAGE (Generic Desktop)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
		
	}

	if (_hatSwitchCount > 0) {

		// USAGE (Hat Switch)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (7)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;

		// PHYSICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// PHYSICAL_MAXIMUM (315)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// UNIT (Eng Rot:Angular Pos)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;

		// REPORT_SIZE (4)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

		// REPORT_COUNT (1)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
						
		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		if (_hatSwitchCount > 1) {
			
			// USAGE (Hat Switch)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x39;

			// LOGICAL_MINIMUM (0)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

			// LOGICAL_MAXIMUM (7)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x07;

			// PHYSICAL_MINIMUM (0)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

			// PHYSICAL_MAXIMUM (315)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x46;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x3B;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// UNIT (Eng Rot:Angular Pos)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x65;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x14;

			// REPORT_SIZE (4)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

			// REPORT_COUNT (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;
							
			// INPUT (Data,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		} else {
		
			// Use Padding Bits
		
			// REPORT_SIZE (1)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

			// REPORT_COUNT (4)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;
					
			// INPUT (Const,Var,Abs)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;
			
		} // One or Two Hat Switches?

	} // Hat Switches

	if (axisCount > 0) {
	
		// USAGE (Pointer)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (65535)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x27;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0XFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0XFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// REPORT_SIZE (16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;

		// REPORT_COUNT (axisCount)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = axisCount;
						
		// COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		if (includeXAxis == true) {
			// USAGE (X)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x30;
		}

		if (includeYAxis == true) {
			// USAGE (Y)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x31;
		}
		
		if (includeZAxis == true) {
			// USAGE (Z)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x32;
		}
		
		if (includeRxAxis == true) {
			// USAGE (Rx)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x33;
		}
		
		if (includeRyAxis == true) {
			// USAGE (Ry)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x34;
		}
		
		if (includeRzAxis == true) {
			// USAGE (Rz)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x35;
		}
		
		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// END_COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
		
	} // X, Y, Z, Rx, Ry, and Rz Axis	
	
	if (simulationCount > 0) {
	
		// USAGE_PAGE (Simulation Controls)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// LOGICAL_MINIMUM (0)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// LOGICAL_MAXIMUM (65535)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x27;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0XFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0XFF;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		// REPORT_SIZE (16)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x10;

		// REPORT_COUNT (simulationCount)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
		tempHidReportDescriptor[hidReportDescriptorSize++] = simulationCount;

		// COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

		if (includeRudder == true) {
			// USAGE (Rudder)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBA;
		}

		if (includeThrottle == true) {
			// USAGE (Throttle)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xBB;
		}

		if (includeAccelerator == true) {
			// USAGE (Accelerator)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC4;
		}

		if (includeBrake == true) {
			// USAGE (Brake)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC5;
		}

		if (includeSteering == true) {
			// USAGE (Steering)
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
			tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC8;
		}

		// INPUT (Data,Var,Abs)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
		
		// END_COLLECTION (Physical)
		tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;
	
	} // Simulation Controls

    // END_COLLECTION
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;

	// Create a copy of the HID Report Descriptor template that is just the right size
	uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);
	
	// Register HID Report Description
	DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
	DynamicHID().AppendDescriptor(node);
	
    // Setup Joystick State
	if (buttonCount > 0) {
		_buttonValuesArraySize = _buttonCount / 8;
		if ((_buttonCount % 8) > 0) {
			_buttonValuesArraySize++;
		}
		_buttonValues = new uint8_t[_buttonValuesArraySize];
	}
	
	// Calculate HID Report Size
	_hidReportSize = _buttonValuesArraySize;
	_hidReportSize += (_hatSwitchCount > 0);
	_hidReportSize += (axisCount * 2);
	_hidReportSize += (simulationCount * 2);
	
	// Initialize Joystick State
	_xAxis = 0;
	_yAxis = 0;
	_zAxis = 0;
	_xAxisRotation = 0;
	_yAxisRotation = 0;
	_zAxisRotation = 0;
	_throttle = 0;
	_rudder = 0;
	_accelerator = 0;
	_brake = 0;
	_steering = 0;
	for (int index = 0; index < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; index++) {
		_hatSwitchValues[index] = JOYSTICK_HATSWITCH_RELEASE;
	}
    for (int index = 0; index < _buttonValuesArraySize; index++) {
        _buttonValues[index] = 0;
    }
}

void Joystick_::begin(bool initAutoSendState) {
	_autoSendState = initAutoSendState;
	sendState();
}

void Joystick_::end() {
}

void Joystick_::setButton(uint8_t button, uint8_t value) {
	if (value == 0) {
		releaseButton(button);
	} else {
		pressButton(button);
	}
}


void Joystick_::pressButton(uint8_t button) {
    if (button >= _buttonCount) return;
    int index = button / 8;
    int bit = button % 8;
	bitSet(_buttonValues[index], bit);
	if (_autoSendState) sendState();
}


void Joystick_::releaseButton(uint8_t button) {
    if (button >= _buttonCount) return;
    int index = button / 8;
    int bit = button % 8;
    bitClear(_buttonValues[index], bit);
	if (_autoSendState) sendState();
}


void Joystick_::setXAxis(int32_t value) {
	_xAxis = value;
	if (_autoSendState) sendState();
}


void Joystick_::setYAxis(int32_t value) {
	_yAxis = value;
	if (_autoSendState) sendState();
}


void Joystick_::setZAxis(int32_t value) {
	_zAxis = value;
	if (_autoSendState) sendState();
}


void Joystick_::setRxAxis(int32_t value) {
	_xAxisRotation = value;
	if (_autoSendState) sendState();
}


void Joystick_::setRyAxis(int32_t value) {
	_yAxisRotation = value;
	if (_autoSendState) sendState();
}


void Joystick_::setRzAxis(int32_t value) {
	_zAxisRotation = value;
	if (_autoSendState) sendState();
}


void Joystick_::setRudder(int32_t value) {
	_rudder = value;
	if (_autoSendState) sendState();
}


void Joystick_::setThrottle(int32_t value) {
	_throttle = value;
	if (_autoSendState) sendState();
}


void Joystick_::setAccelerator(int32_t value) {
	_accelerator = value;
	if (_autoSendState) sendState();
}


void Joystick_::setBrake(int32_t value) {
	_brake = value;
	if (_autoSendState) sendState();
}


void Joystick_::setSteering(int32_t value) {
	_steering = value;
	if (_autoSendState) sendState();
}


void Joystick_::setHatSwitch(int8_t hatSwitchIndex, int16_t value) {
	if (hatSwitchIndex >= _hatSwitchCount) return;
	_hatSwitchValues[hatSwitchIndex] = value;
	if (_autoSendState) sendState();
}


int Joystick_::buildAndSet16BitValue(bool includeValue, int32_t value, int32_t valueMinimum, int32_t valueMaximum, 
									 int32_t actualMinimum, int32_t actualMaximum, uint8_t dataLocation[])  {
	int32_t convertedValue;
	uint8_t highByte;
	uint8_t lowByte;
	int32_t realMinimum = min(valueMinimum, valueMaximum);
	int32_t realMaximum = max(valueMinimum, valueMaximum);
	if (includeValue == false) return 0;
	if (value < realMinimum) {
		value = realMinimum;
	}
	if (value > realMaximum) {
		value = realMaximum;
	}
	if (valueMinimum > valueMaximum) {
		// Values go from a larger number to a smaller number (e.g. 1024 to 0)
		value = realMaximum - value + realMinimum;
	}
	convertedValue = map(value, realMinimum, realMaximum, actualMinimum, actualMaximum);
	highByte = (uint8_t)(convertedValue >> 8);
	lowByte = (uint8_t)(convertedValue & 0x00FF);
	dataLocation[0] = lowByte;
	dataLocation[1] = highByte;
	return 2;
}


int Joystick_::buildAndSetAxisValue(bool includeAxis, int32_t axisValue, int32_t axisMinimum, int32_t axisMaximum, uint8_t dataLocation[])  {
	return buildAndSet16BitValue(includeAxis, axisValue, axisMinimum, axisMaximum, JOYSTICK_AXIS_MINIMUM, JOYSTICK_AXIS_MAXIMUM, dataLocation);
}


int Joystick_::buildAndSetSimulationValue(bool includeValue, int32_t value, int32_t valueMinimum, int32_t valueMaximum, uint8_t dataLocation[])  {
	return buildAndSet16BitValue(includeValue, value, valueMinimum, valueMaximum, JOYSTICK_SIMULATOR_MINIMUM, JOYSTICK_SIMULATOR_MAXIMUM, dataLocation);
}


void Joystick_::sendState() {
	uint8_t data[_hidReportSize];
	int index = 0;
	for (; index < _buttonValuesArraySize; index++) {  // Load Button State
		data[index] = _buttonValues[index];		
	}
	if (_hatSwitchCount > 0) {  // Set Hat Switch Values
		// Calculate hat-switch values
		uint8_t convertedHatSwitch[JOYSTICK_HATSWITCH_COUNT_MAXIMUM];
		for (int hatSwitchIndex = 0; hatSwitchIndex < JOYSTICK_HATSWITCH_COUNT_MAXIMUM; hatSwitchIndex++) {
			if (_hatSwitchValues[hatSwitchIndex] < 0) {
				convertedHatSwitch[hatSwitchIndex] = 8;
			} else {
				convertedHatSwitch[hatSwitchIndex] = (_hatSwitchValues[hatSwitchIndex] % 360) / 45;
			}			
		}
		// Pack hat-switch states into a single byte
		data[index++] = (convertedHatSwitch[1] << 4) | (B00001111 & convertedHatSwitch[0]);
	}

	// Set Axis Values
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_X_AXIS, _xAxis, _xAxisMinimum, _xAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Y_AXIS, _yAxis, _yAxisMinimum, _yAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_Z_AXIS, _zAxis, _zAxisMinimum, _zAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RX_AXIS, _xAxisRotation, _rxAxisMinimum, _rxAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RY_AXIS, _yAxisRotation, _ryAxisMinimum, _ryAxisMaximum, &(data[index]));
	index += buildAndSetAxisValue(_includeAxisFlags & JOYSTICK_INCLUDE_RZ_AXIS, _zAxisRotation, _rzAxisMinimum, _rzAxisMaximum, &(data[index]));
	
	// Set Simulation Values
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_RUDDER, _rudder, _rudderMinimum, _rudderMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_THROTTLE, _throttle, _throttleMinimum, _throttleMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_ACCELERATOR, _accelerator, _acceleratorMinimum, _acceleratorMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_BRAKE, _brake, _brakeMinimum, _brakeMaximum, &(data[index]));
	index += buildAndSetSimulationValue(_includeSimulatorFlags & JOYSTICK_INCLUDE_STEERING, _steering, _steeringMinimum, _steeringMaximum, &(data[index]));
	DynamicHID().SendReport(_hidReportId, data, _hidReportSize);
}


JoyClassic::JoyClassic(uint8_t up, uint8_t down, uint8_t left, uint8_t right, 
		uint8_t button): up(up), down(down), left(left), right(right),
		button (button) {
	pinMode(up, INPUT_PULLUP);  
    pinMode(down, INPUT_PULLUP);  
    pinMode(left, INPUT_PULLUP);  
    pinMode(right, INPUT_PULLUP);  
    pinMode(button, INPUT_PULLUP); 
}


void JoyClassic::loop() {
    !digitalRead(button) ? pressButton(0) : releaseButton(0);
    setXAxis(0);
    setYAxis(0);
    if (!digitalRead(up)) setYAxis(-127);
    if (!digitalRead(down)) setYAxis(127);
    if (!digitalRead(left)) setXAxis(-127);
    if (!digitalRead(button)) setXAxis(127);
    sendState();
    delay(50);
}


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


uint8_t JoyPS2::begin(bool initAutoSendState) {
	uint8_t temp[sizeof(type_read)];
	Joystick_::begin(initAutoSendState);
	CMD_SET();
	CLK_SET();
 
   //new error checking. First, read gamepad a few times to see if it's talking
   read_gamepad();
   read_gamepad();
 
   //see if it talked - see if mode came back. 
   //If still anything but 41, 73 or 79, then it's not talking
   if(PS2data[1] != 0x41 && PS2data[1] != 0x42 && PS2data[1] != 0x73 && PS2data[1] != 0x79){ 
	   Serial.println("Controller mode not matched or no controller found");
	   Serial.print("Expected 0x41, 0x42, 0x73 or 0x79, but got ");
	   Serial.println(PS2data[1], HEX);
	   return 1; //return error code 1
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
			if(PS2data[1] == 0x79)
				break;
			if(PS2data[1] == 0x73)
				return 3;
	   	}
	   	if(PS2data[1] == 0x73)
			break;
		if(y == 10){
		#ifdef PS2X_DEBUG
			Serial.println("Controller not accepting commands");
			Serial.print("mode still set at");
			Serial.println(PS2data[1], HEX);
	   	#endif
		   return 2; //exit function with error
	   	}
	   	read_delay += 1; //add 1ms to read_delay
   	}
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

 
bool JoyPS2::read_gamepad(boolean motor1, byte motor2) {
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
	#ifdef PS2X_COM_DEBUG
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
	#ifdef PS2X_COM_DEBUG
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
		delay(read_delay);                  //wait a few
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


JoySegaMegadrive::JoySegaMegadrive(const int players): _players(players) {
	for (int i = 0; i < sizeof(_inputMap) / sizeof(MegadriveInput); i++) {
        pinMode(_inputMap[i].pin, INPUT);
        digitalWrite(_inputMap[i].pin, HIGH);
    }
    for (int i = 0; i < _players; i++) { 
        pinMode(_select[i], OUTPUT);
        digitalWrite(_select[i], HIGH);
    }
}


void JoySegaMegadrive::sendStates() {
    for (int i = 0; i < _players; i++) {
		for (int j = 0; j < 12; j++) {
			uint16_t flag = (2<j);
			int last = (_lastState[i] & flag);
			int current = (_currentState[i] & flag);
			if (last != current) {
				if (current == flag) {
					pressButton(i*12 + j);
				} else {
					releaseButton(i*12 + j);
				}
			}
		}
		_lastState[i] = _currentState[i];
    }
    sendState();
}


void JoySegaMegadrive::read3buttons(int player) {
    digitalWrite(_select[player], LOW);
    delayMicroseconds(20);
    for (int i = 0; i < sizeof(_inputMap) / sizeof(MegadriveInput); i++) {
        if (i/6 == player && digitalRead(_inputMap[i].pin) == LOW) {
            _currentState[player] |= _inputMap[i].lowFlag;
        }
    }
    digitalWrite(_select[player], HIGH);  // Set _select HIGH and read highFlag
    delayMicroseconds(20);
    for (int i = 0; i < sizeof(_inputMap) / sizeof(MegadriveInput); i++) {
        if (i/6 == player && digitalRead(_inputMap[i].pin) == LOW) {
            _currentState[player] |= _inputMap[i].highFlag;
        }
    }
    // When a six-button first connects, it'll spam UP and DOWN,
    // which signals the game to switch to 6-button polling
    if (_currentState[player] == (ON | UP | DOWN)) {
        _sixButtonMode[player] = true;
    }
    // When a controller disconnects, revert to three-button polling
    else if ((_currentState[player] & ON) == 0) {
        _sixButtonMode[player] = false;
    }
    delayMicroseconds(20);
}


void JoySegaMegadrive::read6buttons(int player) {
    read3buttons(player);  // Poll for three-button states twice
    read3buttons(player);
    // After two three-button polls, pulse the _select line
    // so the six-button reports the higher button states
    digitalWrite(_select[player], LOW);
    delayMicroseconds(20);
    digitalWrite(_select[player], HIGH);
    for(int i = 0; i < sizeof(_inputMap) / sizeof(MegadriveInput); i++) {
        if (i/6 == player && digitalRead(_inputMap[i].pin) == LOW) {
            _currentState[player] |= _inputMap[i].pulse3Flag;
        }
    }
    delayMicroseconds(1360); // Increased from 1000 for Sega Mega Drive original Arcade 6 Button Pad Controller MK-1653-50
}


void JoySegaMegadrive::loop() {
    for (int i = 0; i < _players; i++) {
		_currentState[i] = 0;
        if (_sixButtonMode[i]) {
           read6buttons(i);
        } else {
            read3buttons(i);
        }
    }
	sendStates();
}


JoySegaMastersystem::JoySegaMastersystem(uint8_t up, uint8_t down, 
		uint8_t left, uint8_t right, uint8_t b1, uint8_t b2) {
	pinMode(up, INPUT_PULLUP);  
	pinMode(down, INPUT_PULLUP);  
	pinMode(left, INPUT_PULLUP);  
	pinMode(right, INPUT_PULLUP);  
	pinMode(b1, INPUT_PULLUP);  
	pinMode(b2, INPUT_PULLUP); 
}


JoySegaMastersystem::loop(){
	!digitalRead(CONTROL_PAD_UP) ? Joystick.pressButton(0) : Joystick.releaseButton(0);
	!digitalRead(CONTROL_PAD_DOWN) ? Joystick.pressButton(1) : Joystick.releaseButton(1);
	!digitalRead(CONTROL_PAD_LEFT) ? Joystick.pressButton(2) : Joystick.releaseButton(2);
	!digitalRead(CONTROL_PAD_RIGHT) ? Joystick.pressButton(3) : Joystick.releaseButton(3);
	!digitalRead(CONTROL_PAD_B1) ? Joystick.pressButton(4) : Joystick.releaseButton(4);
	!digitalRead(CONTROL_PAD_B2) ? Joystick.pressButton(5) : Joystick.releaseButton(5);
	sendState();
	delay(25);
}


JoySNES::JoySNES(uint8_t clock, uint8_t latch, uint8_t data):
		_clock(clock), _latch(latch), _data(data) {
    pinMode(clock, OUTPUT);
    digitalWrite(clock, HIGH);
    pinMode(latch, OUTPUT);
    digitalWrite(latch, LOW);
    pinMode(data, INPUT_PULLUP);
}


void JoySNES::loop(){
    uint16_t state = 0;
    digitalWrite(_latch, HIGH);
    delayMicroseconds(12); // 12us latch
    digitalWrite(_latch, LOW);
    delayMicroseconds(6);
    for(int i = 0; i < 16; i++){
        digitalWrite(_clock, LOW);
        delayMicroseconds(6);
        state |= digitalRead(_data) << i;
        digitalWrite(_clock, HIGH);
        delayMicroseconds(6);
    }
    SNES_B & ~state ? pressButton(0) : releaseButton(0);
    SNES_Y & ~state ? pressButton(1) : releaseButton(1);
    SNES_SELECT & ~state ? pressButton(2) : releaseButton(2);
    SNES_START & ~state ? pressButton(3) : releaseButton(3);
    SNES_A & ~state ? pressButton(4) : releaseButton(4);
    SNES_X & ~state ? pressButton(5) : releaseButton(5);
    SNES_L & ~state ? pressButton(6) : releaseButton(6);
    SNES_R & ~state ? pressButton(7) : releaseButton(7);
    // 360° Hat Switch 0
    setHatSwitch(0, -1); // release
    if (SNES_UP & ~state) setHatSwitch(0, 0);
    if (SNES_RIGHT & ~state) setHatSwitch(0, 90);
    if (SNES_DOWN & ~state) setHatSwitch(0, 180);
    if (SNES_LEFT & ~state) setHatSwitch(0, 270);
    if (SNES_UP & ~state && SNES_RIGHT & ~state) setHatSwitch(0, 45);
    if (SNES_DOWN & ~state && SNES_RIGHT & ~state) setHatSwitch(0, 135);
    if (SNES_DOWN & ~state && SNES_LEFT & ~state) setHatSwitch(0, 225);
    if (SNES_UP & ~state && SNES_LEFT & ~state) setHatSwitch(0, 315);
    sendState();
    delay(25);
}