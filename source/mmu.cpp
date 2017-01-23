#include <iostream>
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
			return m_data.bios->read8(addr);
		} else {
			return m_data.cart->read8(addr);
		}
	case 0x8000:
	case 0x9000:
		return m_data.gpu->read8(addr);
	case 0xc000:
		return m_data.wram0[addr & 0x0fff];
	case 0xd000:
		return m_data.wram1[addr & 0x0fff];
	case 0xe000:
	case 0xf000:
		if (addr < 0xfe00) {
			// mirror of wram
			break;
		}
		if (addr < 0xfea0) {
			// oam
			break;
		}
		if (addr < 0xff00) {
			// unused
			break;
		}
		if (addr < 0xff80) {
			// io registers
			switch (addr & 0x00f0) {
			case 0x0000:
			// timer, serial
			case 0x0010:
			case 0x0020:
			case 0x0030:
				// audio, ignore for now
				return 0;
			case 0x0040:
				// video
				return m_data.gpu->read8(addr);
			default:
				break;
			}
			break;
		}
		if (addr < 0xffff) {
			// hram
			return m_data.hram[addr & 0x007f];
		}
		if (addr == 0xffff) {
			// interrupt enable
			break;
		}
	default:
		break;
	}

	std::cout << "Address: 0x" << std::hex << +addr << '\n';
	throw std::runtime_error{"Read from unknown address"};
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
		m_data.cart->write8(addr, v);
		return;
	case 0x8000:
	case 0x9000:
		m_data.gpu->write8(addr, v);
		return;
	case 0xc000:
		m_data.wram0[addr & 0x0fff] = v;
		return;
	case 0xd000:
		m_data.wram1[addr & 0x0fff] = v;
		return;
	case 0xe000:
	case 0xf000:
		if (addr < 0xfe00) {
			// mirror of wram
			break;
		}
		if (addr < 0xfea0) {
			// oam
			break;
		}
		if (addr < 0xff00) {
			// unused
			break;
		}
		if (addr < 0xff80) {
			// io registers
			switch (addr & 0x00f0) {
			case 0x0000:
			// timer, serial
			case 0x0010:
			case 0x0020:
			case 0x0030:
				// audio, ignore for now
				return;
			case 0x0040:
				// video
				m_data.gpu->write8(addr, v);
				return;
			case 0x0050:
				if (addr == 0xff50) {
					m_data.biosMode = false;
					return;
				}
				break;
			default:
				break;
			}
			break;
		}
		if (addr < 0xffff) {
			// hram
			m_data.hram[addr & 0x007f] = v;
			return;
		}
		if (addr == 0xffff) {
			// interrupt enable
			break;
		}
	default:
		break;
	}
	std::cout << "Address: 0x" << std::hex << +addr << '\n';
	throw std::runtime_error{"Write to unknown address"};
}

extern "C" std::unique_ptr<IMMU> loadMMU(IMMU::Data& data) {
	return std::make_unique<MMU>(data);
}
