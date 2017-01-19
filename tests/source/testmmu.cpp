#include "testmmu.h"

TestMMU::TestMMU(std::array<u8, 0x10000>& data) : m_data{data} {}

u8 TestMMU::read8(u16 addr) { return m_data[addr]; }

void TestMMU::write8(u16 addr, u8 v) { m_data[addr] = v; }
