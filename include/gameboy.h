#pragma once

#include "icpu.h"
#include "igpu.h"
#include "immu.h"

class GameBoy {
public:
	GameBoy(const std::string&, const std::string&);
	virtual void Run() = 0;
	virtual ~GameBoy() = default;

protected:
	// cpu, mapper, mmu, display, ...
	std::unique_ptr<ICartridge> m_cart;
	BIOS m_bios;

	IGPU::Data m_gpuData;
	std::unique_ptr<IGPU> m_gpu;

	IMMU::Data m_mmuData;
	std::unique_ptr<IMMU> m_mmu;

	ICPU::Data m_cpuData;
	std::unique_ptr<ICPU> m_cpu;
};
