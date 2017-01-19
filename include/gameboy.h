#pragma once

#include "bios.h"
#include "icartridge.h"
#include "icpu.h"

class GameBoy {
public:
	GameBoy(const std::string&, const std::string&);
	virtual void Run() = 0;
	virtual ~GameBoy() = default;

protected:
	// cpu, mapper, mmu, display, ...
	ICPU::Data m_cpuData;
	std::unique_ptr<ICPU> m_cpu;
	BIOS m_bios;
	std::unique_ptr<ICartridge> m_cart;
};
