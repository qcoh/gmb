#pragma once

#include <array>
#include <memory>
#include <string>

#include "bios.h"
#include "gpu.h"
#include "icartridge.h"
#include "interruptdata.h"
#include "joypad.h"
#include "types.h"

class IMMU {
public:
	struct Data {
		bool biosMode = true;
		BIOS* bios = nullptr;
		ICartridge* cart = nullptr;
		GPU* gpu = nullptr;
		std::array<u8, 127> hram = {{0}};
		std::array<u8, 4096> wram0 = {{0}};
		std::array<u8, 4096> wram1 = {{0}};
		InterruptData* intData = nullptr;
		Joypad* joypad = nullptr;
	};
	IMMU() = default;
	IMMU(const IMMU&) = delete;
	IMMU& operator=(const IMMU&) = delete;
	IMMU(IMMU&&) = delete;
	IMMU& operator=(IMMU&&) = delete;
	virtual ~IMMU() = default;

	virtual u8 read8(u16) = 0;
	virtual void write8(u16, u8) = 0;

	virtual u16 read16(u16);
	virtual void write16(u16, u16);
};
