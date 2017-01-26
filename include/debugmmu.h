#pragma once

#include <memory>

#include "mmu.h"

class DebugMMU : public MMU {
public:
	DebugMMU(IMMU::Data&);
	virtual ~DebugMMU() = default;

	virtual u8 read8(u16) override;
	virtual void write8(u16, u8) override;
	virtual void write16(u16, u16) override;

	void undo();

private:
	enum class Type {
		READ,
		WRITE8,
		WRITE16,
	};

	Type m_type = Type::READ;
	u16 m_addr = 0;
	u8 m_val8 = 0;
	u16 m_val16 = 0;
};
