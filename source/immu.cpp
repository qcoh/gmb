#include "immu.h"
#include "romonly.h"

IMMU::Data::Data(const std::string& biosPath, ICartridge* cart_, IGPU* gpu_)
    : bios{biosPath}, cart{cart_}, gpu{gpu_} {}

u16 IMMU::read16(u16 addr) {
	return static_cast<u16>(read8(addr) | (read8(addr + 1) << 8));
}

void IMMU::write16(u16 addr, u16 v) {
	write8(addr, static_cast<u8>(v));
	write8(addr + 1, static_cast<u8>(v >> 8));
}
