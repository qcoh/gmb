#pragma once

#include <array>
#include <memory>
#include <string>

#include "bios.h"
#include "icartridge.h"
#include "types.h"

class IMMU {
public:
	struct Data {
		BIOS bios;
		std::unique_ptr<ICartridge> cart;
		std::array<u8, 127> hram;
		std::array<u8, 4096> wram0;
		std::array<u8, 4096> wram1;

		Data(const std::string&, const std::string&);
	};
	IMMU() = default;
	IMMU(const IMMU&) = delete;
	IMMU& operator=(const IMMU&) = delete;
	IMMU(IMMU&&) = delete;
	IMMU& operator=(IMMU&&) = delete;
	virtual ~IMMU() = default;

	virtual u8 read(u16) = 0;
};
