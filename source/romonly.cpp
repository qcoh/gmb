#include <algorithm>
#include <fstream>

#include "romonly.h"

RomOnly::RomOnly(const std::string& path) {
	std::ifstream f{path, std::ios::binary};
	std::copy_n(std::istreambuf_iterator<char>{f}, 0x10000,
		    std::begin(m_data));
}

u8 RomOnly::read(u16 addr) { return m_data[addr]; }

void RomOnly::write(u16, u8) { return; }
