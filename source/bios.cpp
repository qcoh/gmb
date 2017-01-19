#include <algorithm>
#include <fstream>

#include "bios.h"

BIOS::BIOS(const std::string& path) {
	std::ifstream f{path, std::ios::binary};
	std::copy_n(std::istreambuf_iterator<char>{f}, 0x100,
		    std::begin(m_data));
}

u8 BIOS::read8(u16 addr) const { return m_data[addr]; }

u16 BIOS::read16(u16 addr) const {
	return static_cast<u16>(m_data[addr] | m_data[addr + 1] << 8);
}
