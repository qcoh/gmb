#include "romonly.h"

RomOnly::RomOnly(std::vector<u8>&& data) : m_data{std::move(data)} {}

u8 RomOnly::read8(u16 addr) { return m_data[addr]; }

void RomOnly::write8(u16, u8) { return; }
