#pragma once

#include "igpu.h"

class GPU : public IGPU {
public:
	GPU(IGPU::Data&);
	virtual ~GPU() = default;

	virtual u8 read8(u16) const override;
	virtual void write8(u16, u8) override;

private:
	IGPU::Data& m_data;
};
