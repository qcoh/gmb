#pragma once

#include <array>

#include "types.h"

class IGPU {
public:
	struct Data {
		std::array<u8, 8192> vram = {{0}};
	};
	IGPU() = default;
	IGPU(const IGPU&) = delete;
	IGPU& operator=(const IGPU&) = delete;
	IGPU(IGPU&&) = delete;
	IGPU& operator=(IGPU&&) = delete;
	virtual ~IGPU() = default;

	virtual u8 read8(u16) const = 0;
	virtual void write8(u16, u8) = 0;
};
