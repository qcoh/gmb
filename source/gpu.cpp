#include "gpu.h"

GPU::GPU(IGPU::Data& data) : m_data{data} {}

u8 GPU::read8(u16 addr) const {
	// TODO oam, registers
	return m_data.vram[addr & 0x1fff];
}

void GPU::write8(u16 addr, u8 v) { m_data.vram[addr & 0x1fff] = v; }
