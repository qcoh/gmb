#pragma once

#include "types.h"

class ICartridge {
public:
	ICartridge() = default;
	ICartridge(const ICartridge&) = delete;
	ICartridge& operator=(const ICartridge&) = delete;
	ICartridge(ICartridge&&) = delete;
	ICartridge& operator=(ICartridge&&) = delete;
	virtual ~ICartridge() = default;

	virtual u8 read(u16) = 0;
	virtual void write(u16, u8) = 0;
};
