#pragma once

#include "bitref.h"
#include "types.h"

struct InterruptData {
	u8 interruptEnable = 0;

	// V-Blank interrupt
	BitRef<u8> vBlankEnable{interruptEnable, 0};
	// LCD Stat interrupt
	BitRef<u8> lcdStatEnable{interruptEnable, 1};
	// Timer interrupt
	BitRef<u8> timerEnable{interruptEnable, 2};
	// Serial interrupt
	BitRef<u8> serialEnable{interruptEnable, 3};
	// Joypad interrupt
	BitRef<u8> joypadEnable{interruptEnable, 4};

	u8 interruptFlag = 0;

	// V-Blank interrupt
	BitRef<u8> vBlankFlag{interruptFlag, 0};
	// LCD Stat interrupt
	BitRef<u8> lcdStatFlag{interruptFlag, 1};
	// Timer interrupt
	BitRef<u8> timerFlag{interruptFlag, 2};
	// Serial interrupt
	BitRef<u8> serialFlag{interruptFlag, 3};
	// Joypad interrupt
	BitRef<u8> joypadFlag{interruptFlag, 4};

	bool interruptMasterEnable = false;
};
