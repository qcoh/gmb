#pragma once

#include <memory>
#include <string>

#include "types.h"

class ICartridge {
public:
	ICartridge() = default;
	ICartridge(const ICartridge&) = delete;
	ICartridge& operator=(const ICartridge&) = delete;
	ICartridge(ICartridge&&) = delete;
	ICartridge& operator=(ICartridge&&) = delete;
	virtual ~ICartridge() = default;

	virtual u8 read8(u16) = 0;
	virtual void write8(u16, u8) = 0;
};

std::unique_ptr<ICartridge> fromFile(const std::string&);
