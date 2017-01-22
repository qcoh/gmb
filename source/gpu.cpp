#include <stdexcept>

#include "gpu.h"

GPU::GPU(IGPU::Data& data) : m_data{data} {}

u8 GPU::read8(u16 addr) const {
	// TODO oam
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		return m_data.vram[addr & 0x1fff];
	case 0xf000:
		switch (addr) {
		case 0xff40:
			return m_data.lcdControl;
		case 0xff41:
			return m_data.lcdStat;
		case 0xff42:
			return m_data.scrollY;
		case 0xff43:
			return m_data.scrollX;
		case 0xff44:
			return m_data.lY;
		case 0xff45:
			return m_data.lYC;
		case 0xff46:
			break;
		case 0xff47:
			return m_data.bgp;
		case 0xff48:
			return m_data.obp0;
		case 0xff49:
			return m_data.obp1;
		case 0xff4a:
			return m_data.wY;
		case 0xff4b:
			return m_data.wX;
		default:
			break;
		};
	default:
		break;
	}
	throw std::runtime_error{"GPU Address not implemented"};
}

void GPU::write8(u16 addr, u8 v) {
	m_data.vram[addr & 0x1fff] = v;

	// TODO oam
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		m_data.vram[addr & 0x1fff] = v;
		return;
	case 0xf000:
		switch (addr) {
		case 0xff40:
			m_data.lcdControl = v;
		case 0xff41:
			// bit 0, 1, 2 are read only
			m_data.lcdStat =
			    (m_data.lcdStat & 0b111) | (v & 0b11111000);
			return;
		case 0xff42:
			m_data.scrollY = v;
			return;
		case 0xff43:
			m_data.scrollX = v;
			return;
		case 0xff44:
			break;
		case 0xff45:
			m_data.lYC = v;
			return;
		case 0xff46:
			// DMA
			break;
		case 0xff47:
			m_data.bgp = v;
			return;
		case 0xff48:
			m_data.obp0 = v;
			return;
		case 0xff49:
			m_data.obp1 = v;
			return;
		case 0xff4a:
			m_data.wY = v;
			return;
		case 0xff4b:
			m_data.wX = v;
			return;
		default:
			break;
		};
	default:
		break;
	}
	throw std::runtime_error{"GPU Address not implemented"};
}
