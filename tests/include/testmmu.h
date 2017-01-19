#pragma once

#include <array>

#include "immu.h"
#include "types.h"

class TestMMU : public IMMU {
public:
	TestMMU(std::array<u8, 0x10000>&);

	virtual u8 read8(u16) override;
	virtual void write8(u16, u8) override;

private:
	std::array<u8, 0x10000>& m_data;
};
