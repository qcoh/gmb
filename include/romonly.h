#pragma once

#include <vector>

#include "icartridge.h"

class RomOnly : public ICartridge {
public:
	RomOnly(std::vector<u8>&&);
	virtual ~RomOnly() = default;

	virtual u8 read8(u16) override;
	virtual void write8(u16, u8) override;

private:
	std::vector<u8> m_data;
};
