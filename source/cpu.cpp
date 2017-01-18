#include "cpu.h"
#include <iostream>

CPU::CPU(ICPU::Data& data) : ICPU{}, m_data{data} { (void)m_data; }

void CPU::Step() { std::cout << "lel\n"; }

extern "C" std::unique_ptr<ICPU> loadCPU(CPU::Data& data) {
	std::cout << "Reloading CPU...\n";
	return std::make_unique<CPU>(data);
}
