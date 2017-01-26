#pragma once

#include <dlfcn.h>
#include <csignal>
#include <string>
#include <unordered_set>

#include "cpu.h"
#include "debugmmu.h"
#include "display.h"
#include "gpu.h"
#include "icartridge.h"

class DebugBoy {
public:
	DebugBoy(const std::string&, const std::string&);
	void run();
	virtual ~DebugBoy();

	friend void sigHandle(int);

private:
	static volatile sig_atomic_t signaled;
	void* m_handle = nullptr;

	bool quit = false;

	Display m_display{};
	InterruptData m_intData{};

	std::unique_ptr<ICartridge> m_cart;
	BIOS m_bios;

	IGPU::Data m_gpuData{};
	GPU m_gpu{m_gpuData};

	std::unordered_set<u16> m_watchPoints{};
	bool m_watchMode = false;
	IMMU::Data m_mmuData{};
	DebugMMU m_mmu{m_mmuData, m_watchPoints, m_watchMode};

	ICPU::Data m_cpuData{};
	ICPU::Data m_cpuOldData{};
	std::unique_ptr<ICPU> m_cpu;

	enum class Mode { WAIT, RUN };
	Mode m_mode = Mode::WAIT;

	std::unordered_set<u16> m_breakPoints{};

	void reloadCPU();
	void eval(std::string&);
	void printInstruction(u16);
	static void printTile(const IGPU::Data::Tile&);
	void step();
};
