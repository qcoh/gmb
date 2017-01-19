#include "gameboy.h"
#include "cpu.h"
#include "mmu.h"
#include "romonly.h"

GameBoy::GameBoy(const std::string& romPath, const std::string& biosPath)
    : m_mmuData{romPath, biosPath},
      m_mmu{std::make_unique<MMU>(m_mmuData)},
      m_cpuData{},
      m_cpu{std::make_unique<CPU>(m_cpuData)} {}
