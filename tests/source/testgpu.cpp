#include "testgpu.h"

TestGPU::TestGPU() : GPU{nullptr, nullptr} {}

u8 TestGPU::lcdControl() { return m_lcdControl; }
u8 TestGPU::lcdStat() { return m_lcdStat; }
u8 TestGPU::scrollX() { return m_scrollX; }
u8 TestGPU::scrollY() { return m_scrollY; }
u8 TestGPU::lYC() { return m_lYC; }
u8 TestGPU::bgp() { return m_bgp; }
u8 TestGPU::obp0() { return m_obp0; }
u8 TestGPU::obp1() { return m_obp1; }
u8 TestGPU::wX() { return m_wX; }
u8 TestGPU::wY() { return m_wY; }
const std::array<u8, 160>& TestGPU::oam() { return m_oam; }
