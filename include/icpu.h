#pragma once

#include <iostream>
#include <memory>

#include "types.h"

class ICPU {
public:
	struct Data {
		u8 registers[8] = {0};
		u16& af = static_cast<u16*>(static_cast<void*>(registers))[0];
		u16& bc = static_cast<u16*>(static_cast<void*>(registers))[1];
		u16& de = static_cast<u16*>(static_cast<void*>(registers))[2];
		u16& hl = static_cast<u16*>(static_cast<void*>(registers))[3];

		u8& a = static_cast<u8*>(static_cast<void*>(registers))[1];
		u8& f = static_cast<u8*>(static_cast<void*>(registers))[0];
		u8& b = static_cast<u8*>(static_cast<void*>(registers))[3];
		u8& c = static_cast<u8*>(static_cast<void*>(registers))[2];
		u8& d = static_cast<u8*>(static_cast<void*>(registers))[5];
		u8& e = static_cast<u8*>(static_cast<void*>(registers))[4];
		u8& h = static_cast<u8*>(static_cast<void*>(registers))[7];
		u8& l = static_cast<u8*>(static_cast<void*>(registers))[6];

		u16 pc = 0;
		u16 sp = 0;

		u16 nn = 0;
		u8& n = static_cast<u8*>(static_cast<void*>(&nn))[0];

		u8 op = 0;
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
