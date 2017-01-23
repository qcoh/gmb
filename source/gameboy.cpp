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
      m_mmuData{},
      m_mmu{std::make_unique<MMU>(m_mmuData)},
      m_cpuData{},
      m_cpu{std::make_unique<CPU>(m_cpuData, m_mmu.get())} {
	m_gpuData.display = &m_display;
	m_mmuData.bios = &m_bios;
	m_mmuData.cart = m_cart.get();
	m_mmuData.gpu = m_gpu.get();
	m_mmuData.intData = &m_intData;
	m_cpuData.intData = &m_intData;
}
