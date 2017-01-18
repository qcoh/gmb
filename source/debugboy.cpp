#include <dlfcn.h>
#include <iostream>
#include <sstream>

#include "cpu.h"
#include "debugboy.h"

void sigHandle(int signal) { DebugBoy::signaled = signal; }

volatile sig_atomic_t DebugBoy::signaled = 0;

DebugBoy::DebugBoy() : GameBoy{} { std::signal(SIGUSR1, sigHandle); }

DebugBoy::~DebugBoy() {
	if (m_handle != nullptr) {
		dlclose(m_handle);
	}
}

void DebugBoy::Run() {
	for (;;) {
		if (signaled != 0) {
			reloadCPU();
			std::cout << "Reloading CPU...\n";
			signaled = 0;
		}
		if (m_mode == Mode::WAIT) {
			std::string input{};
			std::cout << "> ";
			std::getline(std::cin, input);
			parseCommands(input);
		} else {
			m_cpu->Step();
		}
	}
}

void DebugBoy::parseCommands(std::string& input) {
	std::stringstream stream{input};
	std::string cmd{};
	stream >> cmd;
	std::cout << "Cmd: " << cmd << '\n';
	if (cmd == "continue") {
		m_mode = Mode::RUN;
	} else if (cmd == "next") {
		m_cpu->Step();
	} else if (cmd == "break") {
		// set breakpoints
	} else if (cmd == "trace") {
		// set tracepoints
	}
}

void DebugBoy::reloadCPU() {
	// need to call destructor from shared library before unloading
	m_cpu = nullptr;

	if (m_handle != nullptr) {
		if (dlclose(m_handle) != 0) {
			throw std::runtime_error{"dlclose error"};
		}
		m_handle = nullptr;
	}

	m_handle = dlopen("./cpu.so", RTLD_NOW);
	if (m_handle == nullptr) {
		throw std::runtime_error{dlerror()};
	}

	using Fn = std::unique_ptr<CPU> (*)(ICPU::Data&);

	dlerror();
	Fn f = reinterpret_cast<Fn>(dlsym(m_handle, "loadCPU"));
	const char* dlsym_err = dlerror();
	if (dlsym_err != nullptr) {
		dlclose(m_handle);
		throw std::runtime_error{dlsym_err};
	}

	m_cpu = f(m_cpuData);
}
