#pragma once

#include "icpu.h"

class GameBoy {
public:
	GameBoy();
	virtual void Run() = 0;
	virtual ~GameBoy() = default;

protected:
	// cpu, mapper, mmu, display, ...
	ICPU::Data m_cpuData;
	std::unique_ptr<ICPU> m_cpu;
};
