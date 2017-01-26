#pragma once

#include <dlfcn.h>
#include <csignal>
#include <string>
#include <unordered_set>

#include "gameboy.h"

class DebugBoy : public GameBoy {
public:
	DebugBoy(const std::string&, const std::string&);
	virtual void Run() override;
	virtual ~DebugBoy();

	friend void sigHandle(int);

private:
	enum class Mode { WAIT, RUN };
	Mode m_mode = Mode::WAIT;

	static volatile sig_atomic_t signaled;
	void* m_handle = nullptr;

	std::unordered_set<u16> m_breakPoints{};

	ICPU::Data m_cpuOldData = {};

	void reloadCPU();
	void eval(std::string&);
	void printInstruction(u16);
	static void printTile(const IGPU::Data::Tile&);
	void step();
};
