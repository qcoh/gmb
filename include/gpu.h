#pragma once

#include "igpu.h"

class GPU : public IGPU {
public:
	GPU(IGPU::Data&);
	virtual ~GPU() = default;

	virtual u8 read8(u16) const override;
	virtual void write8(u16, u8) override;
	virtual void step(u16) override;

private:
	static const u8 ACCESSING_OAM = 0b10;
	static const u8 ACCESSING_VRAM = 0b11;
	static const u8 HBLANK = 0b00;
	static const u8 VBLANK = 0b01;

	IGPU::Data& m_data;
};
