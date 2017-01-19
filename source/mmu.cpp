#include <stdexcept>

#include "mmu.h"

MMU::MMU(IMMU::Data& data) : IMMU{}, m_data{data} { (void)m_data; }

u8 MMU::read8(u16 addr) {
	switch (addr & 0xf000) {
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		if (m_data.biosMode && addr < 0x100) {
			return m_data.bios.read8(addr);
		} else {
			return m_data.cart->read(addr);
		}
	default:
		throw std::runtime_error{"Read from unknown address"};
	}
}

void MMU::write8(u16 addr, u8 v) {
	switch (addr & 0xf000) {
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		m_data.cart->write(addr, v);
	default:
		throw std::runtime_error{"Write to unknown address"};
	}
}
