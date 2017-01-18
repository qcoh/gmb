#pragma once

#include <array>

#include "icartridge.h"

class RomOnly : public ICartridge {
public:
	RomOnly(const std::string&);
	virtual ~RomOnly() = default;

	virtual u8 read(u16) override;
	virtual void write(u16, u8) override;

private:
	std::array<u8, 0x10000> m_data;
};
