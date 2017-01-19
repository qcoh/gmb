#pragma once

#include <dlfcn.h>
#include <csignal>
#include <string>
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

	void reloadCPU();
	void parseCommands(std::string&);
};

void sigHandle(int);
