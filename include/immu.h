#pragma once

#include <array>
#include "types.h"

class IMMU {
public:
	struct Data {
		std::array<u8, 256> bios;
		std::array<u8, 127> hram;
		std::array<u8, 4096> wram0;
		std::array<u8, 4096> wram1;

		// mapper, etc.
	};
	IMMU() = default;
	IMMU(const IMMU&) = delete;
	IMMU& operator=(const IMMU&) = delete;
	IMMU(IMMU&&) = delete;
	IMMU& operator=(IMMU&&) = delete;
	virtual ~IMMU() = default;

	virtual u8 read(u16) = 0;
};
