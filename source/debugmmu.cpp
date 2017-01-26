#include <iomanip>
#include <stdexcept>

#include "debugmmu.h"

DebugMMU::DebugMMU(IMMU::Data& data) : MMU{data} {}

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

void DebugMMU::clear() {
	m_watchPoints.clear();
	m_watchMode = false;
}

void DebugMMU::watch(u16 addr) {
	m_watchPoints.insert(addr);
	m_watchMode = true;
}

bool& DebugMMU::watchMode() { return m_watchMode; }

std::ostream& operator<<(std::ostream& os, const DebugMMU::WatchEvent& ev) {
	os << "WatchEvent: [0x" << std::hex << std::setw(4) << std::setfill('0')
	   << +ev.addr << "] == 0x" << std::setw(2) << +ev.oldVal;
	return os;
}
