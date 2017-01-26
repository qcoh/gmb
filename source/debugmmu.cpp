#include "debugmmu.h"

DebugMMU::DebugMMU(IMMU::Data& data) : MMU{data} {}

u8 DebugMMU::read8(u16 addr) {
	m_type = Type::READ;
	m_addr = addr;
	return MMU::read8(addr);
}

void DebugMMU::write8(u16 addr, u8 v) {
	m_type = Type::WRITE8;
	m_addr = addr;
	m_val8 = MMU::read8(addr);
	MMU::write8(addr, v);
}

void DebugMMU::write16(u16 addr, u16 v) {
	m_type = Type::WRITE16;
	m_addr = addr;
	m_val16 = MMU::read16(addr);
	MMU::write16(addr, v);
}

void DebugMMU::undo() {
	if (m_type == Type::WRITE8) {
		MMU::write8(m_addr, m_val8);
		m_type = Type::READ;
	}
	if (m_type == Type::WRITE16) {
		MMU::write16(m_addr, m_val16);
		m_type = Type::READ;
	}
}
