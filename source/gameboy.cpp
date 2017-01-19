#include "gameboy.h"
#include "cpu.h"
#include "romonly.h"

GameBoy::GameBoy(const std::string& romPath, const std::string& biosPath)
    : m_cpu{std::make_unique<CPU>(m_cpuData)},
      m_bios{biosPath},
      m_cart{std::make_unique<RomOnly>(romPath)} {}
