#pragma once

#include "display.h"
#include "icpu.h"
#include "igpu.h"
#include "immu.h"
#include "interruptdata.h"

class GameBoy {
public:
	GameBoy(const std::string&, const std::string&);
	virtual void Run() = 0;
	virtual ~GameBoy() = default;

protected:
	// cpu, mapper, mmu, display, ...
	Display m_display{};

	InterruptData m_intData;

	std::unique_ptr<ICartridge> m_cart;
	BIOS m_bios;

	IGPU::Data m_gpuData;
	std::unique_ptr<IGPU> m_gpu;

	IMMU::Data m_mmuData;
	std::unique_ptr<IMMU> m_mmu;

	ICPU::Data m_cpuData;
	std::unique_ptr<ICPU> m_cpu;

	bool quit = false;
};
