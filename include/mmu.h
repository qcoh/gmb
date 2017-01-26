#pragma once

#include <memory>
#include <string>

#include "icartridge.h"
#include "immu.h"

class MMU : public IMMU {
public:
	MMU(IMMU::Data&);
	virtual ~MMU() = default;

	virtual u8 read8(u16) override;
	virtual void write8(u16, u8) override;

private:
	IMMU::Data& m_data;
};
