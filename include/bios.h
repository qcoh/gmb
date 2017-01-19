#pragma once

#include <array>
#include <string>

#include "types.h"

class BIOS {
public:
	BIOS() = default;
	BIOS(const std::string&);
	virtual ~BIOS() = default;

	u8 read8(u16) const;
	u16 read16(u16) const;

protected:
	std::array<u8, 0x100> m_data = {{0}};
};
