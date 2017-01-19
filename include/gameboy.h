#pragma once

#include "icpu.h"
#include "immu.h"

class GameBoy {
public:
	GameBoy(const std::string&, const std::string&);
	virtual void Run() = 0;
	virtual ~GameBoy() = default;

protected:
	// cpu, mapper, mmu, display, ...
	IMMU::Data m_mmuData;
	std::unique_ptr<IMMU> m_mmu;

	ICPU::Data m_cpuData;
	std::unique_ptr<ICPU> m_cpu;
};
