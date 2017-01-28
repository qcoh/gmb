#pragma once

#include <array>
#include <vector>

#include "icartridge.h"

class MBC1 : public ICartridge {
public:
	MBC1(std::vector<u8>&&);
	virtual ~MBC1() = default;

	virtual u8 read8(u16) override;
	virtual void write8(u16, u8) override;

private:
	std::vector<u8> m_data;
	std::array<u8, 0x8000> m_ram{};

	bool m_ramEnable = false;

	// MBC1 can address up to four 8kb ram banks and up to 125 16kb rom
	// banks, which are selected by a single byte. Two accessing modes
	// exist:
	//
	//  The first (0x0) uses the highest bit of m_bankNo to determine if
	//  there is a 8kb ram bank or not and the lower 7 bits to determine
	//  which rom bank to select. The following indices are remapped:
	//  0x00->0x01, 0x20->0x21, 0x40->0x41, 0x60->0x61, resulting in up to
	//  125 banks.
	//
	//  The second (0x1) uses the highest three bits of m_bankNo to select
	//  one of four ram banks and the lower 5 bits to select up to 31 rom
	//  banks.
	//
	//  TODO: does the remapping also happen in mode 0x1?
	u8 m_bankMode : 1;
	u8 m_bankNo = 0;

	// 0x00 -> 32kbyte
	// 0x01 -> 64kbyte (4 banks)
	// 0x02 -> 128kbyte (8 banks)
	// 0x03 -> 256kbyte (16 banks)
	// 0x04 -> 512kbyte (32 banks)
	// 0x05 -> 1mbyte (64 banks, 63 usable)
	// 0x06 -> 2mbyte (128 banks, 125 usable)
	u8 m_romBanks : 3;

	// 0x00 -> None
	// 0x01 -> 2kbyte
	// 0x02 -> 8kbyte
	// 0x03 -> 32kbytes (4 banks of 8kbyte)
	u8 m_ramBanks : 2;
};
