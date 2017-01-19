#pragma once

#include <array>
#include <memory>
#include <string>

#include "bios.h"
#include "icartridge.h"
#include "igpu.h"
#include "types.h"

class IMMU {
public:
	struct Data {
		bool biosMode = true;
		BIOS bios;
		ICartridge* cart;
		IGPU* gpu;
		std::array<u8, 127> hram = {{0}};
		std::array<u8, 4096> wram0 = {{0}};
		std::array<u8, 4096> wram1 = {{0}};

		Data(const std::string&, ICartridge*, IGPU*);
	};
	IMMU() = default;
	IMMU(const IMMU&) = delete;
	IMMU& operator=(const IMMU&) = delete;
	IMMU(IMMU&&) = delete;
	IMMU& operator=(IMMU&&) = delete;
	virtual ~IMMU() = default;

	virtual u8 read8(u16) = 0;
	virtual void write8(u16, u8) = 0;

	u16 read16(u16);
	void write16(u16, u16);
};
