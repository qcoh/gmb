#include "gameboy.h"
#include "cpu.h"
#include "gpu.h"
#include "mmu.h"
#include "romonly.h"

GameBoy::GameBoy(const std::string& romPath, const std::string& biosPath)
    : m_cart{std::make_unique<RomOnly>(romPath)},
      m_bios{biosPath},
      m_gpuData{},
      m_gpu{std::make_unique<GPU>(m_gpuData)},
      m_mmuData{&m_bios, m_cart.get(), m_gpu.get()},
      m_mmu{std::make_unique<MMU>(m_mmuData)},
      m_cpuData{},
      m_cpu{std::make_unique<CPU>(m_cpuData, m_mmu.get())} {}
