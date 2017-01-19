#include "mmu.h"

MMU::MMU(IMMU::Data& data) : IMMU{}, m_data{data} { (void)m_data; }

u8 MMU::read(u16 addr) {
	(void)addr;
	return 0;
}
