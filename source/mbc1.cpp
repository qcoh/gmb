#include <iostream>
#include <stdexcept>

#include "mbc1.h"

MBC1::MBC1(std::vector<u8>&& data) : m_data{std::move(data)}, m_bankNo{0} {
	(void)m_bankNo;
	(void)m_bankMode;
	(void)m_ramEnable;

	m_romBanks = m_data[0x148];
	m_ramBanks = m_data[0x149];
}

u8 MBC1::read8(u16 addr) {
	u32 offset = 0;

	switch (addr & 0xf000) {
	case 0x0000:
	case 0x1000:
	case 0x2000:
	case 0x3000:
		return m_data[addr];
	case 0x4000:
	case 0x5000:
	case 0x6000:
	case 0x7000:
		offset = (m_bankMode == 0) ? m_bankNo : (m_bankNo & 0b00011111);
		offset = (offset == 0) ? offset + 1 : offset;
		offset *= 0x4000;
		return m_data[offset + (addr & 0x3fff)];
	case 0xa000:
	case 0xb000:
		// TODO: not sure what to return when ram is disabled?
		offset = (m_bankMode == 0) ? 0 : ((m_bankNo >> 5) & 0b11);
		offset *= 0x2000;
		return m_ram[offset + (addr & 0x1fff)];
	default:
		break;
	}
	std::cout << "Addr: 0x" << std::hex << +addr << '\n';
	throw std::runtime_error{"Read from MBC1"};
}

void MBC1::write8(u16 addr, u8 v) {
	u32 offset = 0;

	switch (addr & 0xf000) {
	case 0x0000:
	case 0x1000:  // RAM enable
		m_ramEnable = ((addr & 0xf) == 0xa);
		return;
	case 0x2000:
	case 0x3000:  // ROM bank select
		// TODO: 0x20, 0x40, 0x60?
		v = (v == 0) ? 1 : v;
		m_bankNo =
		    static_cast<u8>((m_bankNo & 0b11100000) | (v & 0b00011111));
		return;
	case 0x4000:
	case 0x5000:  // RAM bank select/upper bits of ROM bank
		m_bankNo =
		    static_cast<u8>((m_bankNo & 0b00011111) | (v & 0b11100000));
		return;
	case 0x6000:
	case 0x7000:  // Bank mode select
		m_bankMode = v;
		return;
	case 0xa000:
	case 0xb000:  // ram
		offset = (m_bankMode == 0) ? 0 : ((m_bankNo >> 5) & 0b11);
		offset *= 0x2000;
		m_ram[offset + (addr & 0x1fff)] = v;
		return;

	default:  // not reachable
		break;
	}
	std::cout << "Addr: 0x" << std::hex << +addr << ", Val: 0x" << std::hex
		  << +v << '\n';
	throw std::runtime_error{"Write to MBC1"};
}
