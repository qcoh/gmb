#pragma once

#include <memory>
#include "types.h"

class ICPU {
public:
	struct Data {
		u8 registers[10] = {0};
		u16& af = static_cast<u16*>(static_cast<void*>(registers))[0];
		u16& bc = static_cast<u16*>(static_cast<void*>(registers))[1];
		u16& de = static_cast<u16*>(static_cast<void*>(registers))[2];
		u16& hl = static_cast<u16*>(static_cast<void*>(registers))[3];
		u16& nn = static_cast<u16*>(static_cast<void*>(registers))[4];

		u8& a = static_cast<u8*>(static_cast<void*>(registers))[1];
		u8& f = static_cast<u8*>(static_cast<void*>(registers))[0];
		u8& b = static_cast<u8*>(static_cast<void*>(registers))[3];
		u8& c = static_cast<u8*>(static_cast<void*>(registers))[2];
		u8& d = static_cast<u8*>(static_cast<void*>(registers))[5];
		u8& e = static_cast<u8*>(static_cast<void*>(registers))[4];
		u8& h = static_cast<u8*>(static_cast<void*>(registers))[7];
		u8& l = static_cast<u8*>(static_cast<void*>(registers))[6];

		u8& n = static_cast<u8*>(static_cast<void*>(registers))[8];
	};

	ICPU() = default;
	ICPU(const ICPU&) = delete;
	ICPU& operator=(const ICPU&) = delete;
	ICPU(ICPU&&) = delete;
	ICPU& operator=(ICPU&&) = delete;
	virtual ~ICPU() = default;

	virtual void Step() = 0;
};
