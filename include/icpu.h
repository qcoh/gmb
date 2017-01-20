#pragma once

#include <iostream>
#include <memory>

#include "bitref.h"
#include "types.h"

class ICPU {
public:
	struct Data {
		u16 af = 0;
		u16 bc = 0;
		u16 de = 0;
		u16 hl = 0;

		u8& a{*(static_cast<u8*>(static_cast<void*>(&af)) + 1)};
		u8& f{*(static_cast<u8*>(static_cast<void*>(&af)))};
		u8& b{*(static_cast<u8*>(static_cast<void*>(&bc)) + 1)};
		u8& c{*(static_cast<u8*>(static_cast<void*>(&bc)))};
		u8& d{*(static_cast<u8*>(static_cast<void*>(&de)) + 1)};
		u8& e{*(static_cast<u8*>(static_cast<void*>(&de)))};
		u8& h{*(static_cast<u8*>(static_cast<void*>(&hl)) + 1)};
		u8& l{*(static_cast<u8*>(static_cast<void*>(&hl)))};

		BitRef<u8> zeroFlag{f, 7};
		BitRef<u8> negFlag{f, 6};
		BitRef<u8> halfFlag{f, 5};
		BitRef<u8> carryFlag{f, 4};

		u16 pc = 0;
		u16 sp = 0;

		u16 nn = 0;
		u8& n = static_cast<u8*>(static_cast<void*>(&nn))[0];

		u8 op = 0;

		u16 cycles = 0;

		u8& read8(u8);
		u16& read16(u8);
	};

	ICPU() = default;
	ICPU(const ICPU&) = delete;
	ICPU& operator=(const ICPU&) = delete;
	ICPU(ICPU&&) = delete;
	ICPU& operator=(ICPU&&) = delete;
	virtual ~ICPU() = default;

	virtual void Step() = 0;
};

std::ostream& operator<<(std::ostream&, const ICPU::Data&);
