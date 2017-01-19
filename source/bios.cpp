#include <algorithm>
#include <fstream>

#include "bios.h"

BIOS::BIOS(const std::string& path) {
	std::ifstream f{};
	f.exceptions(std::ifstream::failbit);
	f.open(path, std::ios::binary);
	std::copy_n(std::istreambuf_iterator<char>{f}, 0x100,
		    std::begin(m_data));
}

u8 BIOS::read8(u16 addr) const { return m_data[addr]; }
