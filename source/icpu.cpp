#include <bitset>
#include <iomanip>

#include "icpu.h"

std::ostream& operator<<(std::ostream& os, const ICPU::Data& data) {
	os << "af = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.af << " == 0b" << std::bitset<16>{data.af} << '\n';
	os << " a =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.a << " == 0b" << std::bitset<8>{data.a} << '\n';
	os << " f =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.a << " == 0b" << std::bitset<8>{data.f} << '\n';

	os << "bc = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.af << " == 0b" << std::bitset<16>{data.bc} << '\n';
	os << " b =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.b << " == 0b" << std::bitset<8>{data.b} << '\n';
	os << " c =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.c << " == 0b" << std::bitset<8>{data.c} << '\n';

	os << "de = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.de << " == 0b" << std::bitset<16>{data.de} << '\n';
	os << " d =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.d << " == 0b" << std::bitset<8>{data.d} << '\n';
	os << " e =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.e << " == 0b" << std::bitset<8>{data.e} << '\n';

	os << "hl = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.hl << " == 0b" << std::bitset<16>{data.hl} << '\n';
	os << " h =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.h << " == 0b" << std::bitset<8>{data.h} << '\n';
	os << " l =   0x" << std::hex << std::setfill('0') << std::setw(2)
	   << +data.l << " == 0b" << std::bitset<8>{data.l} << '\n';

	os << "pc = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.pc << '\n';

	os << "sp = 0x" << std::hex << std::setfill('0') << std::setw(4)
	   << +data.sp << '\n';

	return os;
}
