#pragma once

#include "immu.h"

class MemRef {
public:
	MemRef(const u16& addr, IMMU* mmu) : m_addr{addr}, m_mmu{mmu} {}
	MemRef(const MemRef&) = default;
	MemRef& operator=(const MemRef&) = delete;

	operator u8() const { return m_mmu->read8(m_addr); }

	void operator=(u8 rhs) { m_mmu->write8(m_addr, rhs); }

private:
	const u16& m_addr;
	IMMU* m_mmu;
};
