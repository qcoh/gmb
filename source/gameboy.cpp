#include "gameboy.h"
#include "cpu.h"
#include "gpu.h"
#include "mmu.h"

GameBoy::GameBoy(const std::string& romPath, const std::string& biosPath)
    : m_cart{fromFile(romPath)},
      m_bios{biosPath},
      m_mmuData{},
      m_mmu{std::make_unique<MMU>(m_mmuData)},
      m_cpuData{},
      m_cpu{std::make_unique<CPU>(m_cpuData)} {
	m_mmuData.bios = &m_bios;
	m_mmuData.cart = m_cart.get();
	m_mmuData.gpu = &m_gpu;
	m_mmuData.intData = &m_intData;
	m_cpuData.intData = &m_intData;
	m_cpuData.mmu = m_mmu.get();
}
