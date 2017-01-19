#include <bitset>
#include <iomanip>

#include "icpu.h"

std::ostream& operator<<(std::ostream& os, const ICPU::Data& data) {
	os << "af = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.af << " [a = 0b" << std::bitset<8>{data.a} << "][f = 0b"
	   << std::bitset<8>{data.f} << "]\n";

	os << "bc = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.bc << " [b = 0b" << std::bitset<8>{data.a} << "][c = 0b"
	   << std::bitset<8>{data.f} << "]\n";

	os << "de = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.de << " [d = 0b" << std::bitset<8>{data.a} << "][e = 0b"
	   << std::bitset<8>{data.f} << "]\n";

	os << "hl = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.hl << " [h = 0b" << std::bitset<8>{data.a} << "][l = 0b"
	   << std::bitset<8>{data.f} << "]\n";

	os << "pc = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.pc << '\n';

	os << "sp = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.sp << '\n';

	os << "nn = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.nn << " [n = 0x" << std::hex << std::setfill('0')
	   << std::setw(2) << +data.n << ']';

	return os;
}

u8& ICPU::Data::read8(u8 addr) { return registers[addr]; }

u16& ICPU::Data::read16(u8 addr) {
	switch (addr) {
	case 0x0:
		return bc;
	case 0x1:
		return de;
	case 0x2:
		return hl;
	default:
		return sp;
	}
}
