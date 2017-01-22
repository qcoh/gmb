#include <dlfcn.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include "cpu.h"
#include "debugboy.h"
#include "mmu.h"

void sigHandle(int signal) { DebugBoy::signaled = signal; }

volatile sig_atomic_t DebugBoy::signaled = 0;

DebugBoy::DebugBoy(const std::string& romPath, const std::string& biosPath)
    : GameBoy{romPath, biosPath} {
	std::signal(SIGUSR1, sigHandle);
}

DebugBoy::~DebugBoy() {
	if (m_handle != nullptr) {
		dlclose(m_handle);
	}
}

void DebugBoy::printCurrentInstruction() {
	u8 op = m_mmu->read8(m_cpuData.pc);

	std::cout << "(0x" << std::hex << std::setfill('0') << std::setw(4)
		  << +m_cpuData.pc << ") == 0x" << +op
		  << " == " << CPU::s_instructions[op].mnemonic;

	if (op == 0xcb) {
		std::cout << " " << CPU::s_extended[m_cpuData.n].mnemonic
			  << '\n';
	} else {
		std::cout << '\n';
	}
}

void DebugBoy::Run() {
	for (;;) {
		if (m_mode == Mode::WAIT) {
			m_cpuData.nn = m_mmu->read16(m_cpuData.pc + 1);
			std::cout << m_cpuData << '\n';
			printCurrentInstruction();

			std::string input{};
			std::cout << "> ";
			std::getline(std::cin, input);
			if (signaled != 0) {
				reloadCPU();
				signaled = 0;
			}
			parseCommands(input);
		} else {
			if (m_breakPoints.find(m_cpuData.pc) ==
			    m_breakPoints.end()) {
				m_cpu->Step();
			} else {
				m_mode = Mode::WAIT;
			}
		}
	}
}

void DebugBoy::parseCommands(std::string& input) {
	std::stringstream stream{input};
	std::string cmd{};
	stream >> cmd;
	if (cmd == "next" || cmd == "") {
		m_cpu->Step();
	} else if (cmd == "continue") {
		m_mode = Mode::RUN;
	} else if (cmd == "print") {
		u16 addr = 0;
		if (stream >> std::hex >> addr) {
			u8 v = m_mmu->read8(addr);
			std::cout << "(0x" << std::hex << std::setfill('0')
				  << std::setw(4) << +addr << ") == 0x"
				  << std::setw(2) << v << '\n';
		}
	} else if (cmd == "break") {
		u16 addr = 0;
		if (stream >> std::hex >> addr) {
			m_breakPoints.insert(addr);
		}
	} else if (cmd == "breakpoints") {
		std::cout << "Breakpoints: ";
		std::copy(std::begin(m_breakPoints), std::end(m_breakPoints),
			  std::ostream_iterator<int>(std::cout, ", "));
		std::cout << '\n';
	} else if (cmd == "clear") {
		m_breakPoints.clear();
	} else if (cmd == "trace") {
		// set tracepoints
	}
}

void DebugBoy::reloadCPU() {
	// need to call destructor from shared library before unloading
	m_cpu = nullptr;
	m_mmu = nullptr;

	if (m_handle != nullptr) {
		if (dlclose(m_handle) != 0) {
			throw std::runtime_error{"dlclose error"};
		}
		m_handle = nullptr;
	}

	m_handle = dlopen("./cpu.so", RTLD_LAZY | RTLD_GLOBAL);
	if (m_handle == nullptr) {
		throw std::runtime_error{dlerror()};
	}

	// load MMU
	using MMUFn = std::unique_ptr<MMU> (*)(IMMU::Data&);
	dlerror();
	MMUFn g = reinterpret_cast<MMUFn>(dlsym(m_handle, "loadMMU"));
	const char* dlsym_err = dlerror();
	if (dlsym_err != nullptr) {
		dlclose(m_handle);
		throw std::runtime_error{dlsym_err};
	}
	m_mmu = g(m_mmuData);

	// load CPU
	using CPUFn = std::unique_ptr<CPU> (*)(ICPU::Data&, IMMU*);
	dlerror();
	CPUFn f = reinterpret_cast<CPUFn>(dlsym(m_handle, "loadCPU"));
	dlsym_err = dlerror();
	if (dlsym_err != nullptr) {
		dlclose(m_handle);
		throw std::runtime_error{dlsym_err};
	}
	m_cpu = f(m_cpuData, m_mmu.get());
}
