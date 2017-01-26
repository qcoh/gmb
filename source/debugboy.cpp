#include <dlfcn.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include "cpu.h"
#include "debugboy.h"
#include "debugmmu.h"

void sigHandle(int signal) { DebugBoy::signaled = signal; }

volatile sig_atomic_t DebugBoy::signaled = 0;

DebugBoy::DebugBoy(const std::string& romPath, const std::string& biosPath)
    : GameBoy{romPath, biosPath} {
	m_mmu = std::make_unique<DebugMMU>(m_mmuData);
	m_cpuData.mmu = m_mmu.get();

	std::signal(SIGUSR1, sigHandle);
}

DebugBoy::~DebugBoy() {
	if (m_handle != nullptr) {
		// need to call destructor first
		m_cpu = nullptr;
		dlclose(m_handle);
	}
}

void DebugBoy::step() {
	try {
		u16 cycles = m_cpu->step();
		m_gpu->step(cycles);
		m_cpuOldData = m_cpuData;
	} catch (DebugMMU::WatchEvent& ev) {
		std::cout << ev << '\n';
		m_cpuData = m_cpuOldData;
		m_mode = Mode::WAIT;
	}

	SDL_Event ev = {0};
	SDL_PollEvent(&ev);
	switch (ev.type) {
	case SDL_QUIT:
		quit = true;
	default:
		break;
	}
}

void DebugBoy::printInstruction(u16 addr) {
	u8 op = m_mmu->read8(addr);
	std::cout << "[0x" << std::hex << std::setw(4) << std::setfill('0')
		  << +m_cpuData.pc << "] == 0x" << std::setw(2) << +op
		  << " == " << CPU::s_instructions[op].mnemonic;
	if (op == 0xcb) {
		u8 n = m_mmu->read8(addr + 1);
		std::cout << " " << CPU::s_extended[n].mnemonic;
	} else if (CPU::s_instructions[op].offset == 2) {
		u8 n = m_mmu->read8(m_cpuData.pc + 1);
		std::cout << " (n == 0x" << +n << ')';
	} else if (CPU::s_instructions[op].offset == 3) {
		u16 nn = m_mmu->read16(m_cpuData.pc + 1);
		std::cout << " (n == 0x" << +nn << ')';
	}
	std::cout << '\n';
}

void DebugBoy::Run() {
	while (!quit) {
		if (m_mode == Mode::WAIT) {
			std::string input{};
			std::cout << "> ";
			if (std::getline(std::cin, input).fail()) {
				break;
			}

			if (signaled != 0) {
				reloadCPU();
				signaled = 0;
			}

			eval(input);
		} else {
			if (m_breakPoints.find(m_cpuData.pc) ==
			    m_breakPoints.end()) {
				step();
			} else {
				m_mode = Mode::WAIT;
			}
		}
	}
}

void DebugBoy::eval(std::string& input) {
	u16 addr = 0;
	std::stringstream stream{input};
	std::string cmd{};
	stream >> cmd;

	if (cmd == "n" || cmd == "") {
		// execute a single instruction
		step();
		printInstruction(m_cpuData.pc);
	} else if (cmd == "co") {
		// execute until breakpoint or watchpoint
		step();
		// enable watch mode
		m_mode = Mode::RUN;
	} else if (cmd == "pc") {
		// print cpu registers
		std::cout << m_cpuData << '\n';
	} else if (cmd == "pd") {
		// print disassembly
		if (stream >> std::hex >> addr) {
			printInstruction(addr);
		}
	} else if (cmd == "pm") {
		// print memory
		if (stream >> std::hex >> addr) {
			u8 v = m_mmu->read8(addr);
			std::cout << "[0x" << std::hex << std::setw(4)
				  << std::setfill('0') << +addr << "] == 0x"
				  << std::setw(2) << +v << '\n';
		}
	} else if (cmd == "pt") {
		// print tile
		if (stream >> std::hex >> addr && addr < 384) {
			printTile(m_gpuData.tiles[addr]);
		}
	} else if (cmd == "pb") {
		// print breakpoints
		for (const auto& bp : m_breakPoints) {
			std::cout << "0x" << std::hex << std::setw(2)
				  << std::setfill('0') << +bp << ", ";
		}
		std::cout << '\n';
	} else if (cmd == "pw") {
		// print watchpoints
	} else if (cmd == "b") {
		// add breakpoint
		if (stream >> std::hex >> addr) {
			m_breakPoints.insert(addr);
		}
	} else if (cmd == "w") {
		// add watchpoint
	} else if (cmd == "cb") {
		// clear breakpoints
		m_breakPoints.clear();
	} else if (cmd == "cw") {
		// clear watchpoints
	} else if (cmd == "q") {
		// quit
		quit = true;
	} else if (cmd == "h") {
		// display help
		std::cout << "n\t\t Execute next instruction\n"
			     "co\t\t Execute instructions until breakpoint "
			     "or "
			     "watchpoint\n"
			     "pc\t\t Print CPU registers\n"
			     "pd 0xnnnn\t Print instruction at 0xnnnn\n"
			     "pm 0xnnnn\t Print memory at 0xnnnn\n"
			     "pt 0xnnn\t Print tile at 0xnnn (0 - 383)\n"
			     "pb\t\t Print breakpoints\n"
			     "pw\t\t Print watchpoints\n"
			     "b 0xnnnn\t Add breakpoint\n"
			     "w 0xnnnn\t Add watchpoint\n"
			     "cb\t\t Clear breakpoints\n"
			     "cw\t\t Clear watchpoints\n"
			     "q\t\t Quit\n"
			     "h\t\t Display this help\n";
	}
}

void DebugBoy::printTile(const IGPU::Data::Tile& tile) {
	for (const auto& row : tile) {
		for (u8 i = 0; i < 8; i++) {
			u8 color = static_cast<u8>(((row[0] >> i) & 1) << 1 |
						   ((row[0] >> i) & 1));
			switch (color) {
			case 0x0:
				std::cout << " ";
				break;
			case 0x1:
				std::cout << "░";
				break;
			case 0x2:
				std::cout << "▓";
				break;
			default:
				std::cout << "█";
				break;
			}
		}
		std::cout << '\n';
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

	m_handle = dlopen("./cpu.so", RTLD_LAZY | RTLD_GLOBAL);
	if (m_handle == nullptr) {
		throw std::runtime_error{dlerror()};
	}

	// load CPU
	using CPUFn = std::unique_ptr<CPU> (*)(ICPU::Data&);
	dlerror();
	CPUFn f = reinterpret_cast<CPUFn>(dlsym(m_handle, "loadCPU"));
	const char* dlsym_err = dlerror();
	if (dlsym_err != nullptr) {
		dlclose(m_handle);
		throw std::runtime_error{dlsym_err};
	}
	m_cpu = f(m_cpuData);
}
