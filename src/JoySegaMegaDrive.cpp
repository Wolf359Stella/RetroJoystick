#include "JoySegaMegaDrive.h"


JoySegaMegaDrive::JoySegaMegaDrive(const int players): _players(players) {
	for (int i = 0; i < sizeof(_inputMap) / sizeof(MegadriveInput); i++) {
        pinMode(_inputMap[i].pin, INPUT);
        digitalWrite(_inputMap[i].pin, HIGH);
    }
    for (int i = 0; i < _players; i++) { 
        pinMode(_select[i], OUTPUT);
        digitalWrite(_select[i], HIGH);
    }
}



void JoySegaMegaDrive::sendStates() {
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

void JoySegaMegaDrive::read3buttons(int player) {
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


void JoySegaMegaDrive::read6buttons(int player) {
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


void JoySegaMegaDrive::loop() {
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