#include <iomanip>
#include <stdexcept>

#include "debugmmu.h"

DebugMMU::DebugMMU(IMMU::Data& data, std::unordered_set<u16>& watchPoints,
		   bool& watchMode)
    : MMU{data}, m_watchPoints{watchPoints}, m_watchMode{watchMode} {}

void DebugMMU::write8(u16 addr, u8 v) {
	if (m_watchMode) {
		if (m_watchPoints.find(addr) != std::end(m_watchPoints)) {
			m_watchMode = false;
			u8 old = read8(addr);
			throw WatchEvent{addr, old, v};
		}
	}
	MMU::write8(addr, v);
}

std::ostream& operator<<(std::ostream& os, const DebugMMU::WatchEvent& ev) {
	os << "WatchEvent: [0x" << std::hex << std::setw(4) << std::setfill('0')
	   << +ev.addr << "] == 0x" << std::setw(2) << +ev.oldVal << " <- 0x"
	   << std::setw(2) << +ev.newVal;
	return os;
}
