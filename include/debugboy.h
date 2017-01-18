#pragma once

#include <dlfcn.h>
#include <csignal>
#include "gameboy.h"

class DebugBoy : public GameBoy {
public:
	DebugBoy();
	virtual void Run() override;
	virtual ~DebugBoy();

	friend void sigHandle(int);

private:
	static volatile sig_atomic_t signaled;
	void* m_handle = nullptr;

	void reloadCPU();
};

void sigHandle(int);
