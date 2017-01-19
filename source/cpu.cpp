#include "cpu.h"
#include <iostream>

CPU::CPU(ICPU::Data& data, IMMU* mmu) : ICPU{}, m_data{data}, m_mmu{mmu} {
	(void)m_data;
	(void)m_mmu;
}

void CPU::Step() {
	fetch();
	exec();
}

void CPU::fetch() {
	m_data.op = m_mmu->read8(m_data.pc);
	m_data.nn = m_mmu->read16(m_data.pc + 1);
	m_data.pc += s_instructions[m_data.op].offset;
}

void CPU::exec() {
	switch (m_data.op) {
	case 0x0:
		// NOP
		break;
	case 0x01:
	case 0x11:
	case 0x21:
	case 0x31:
		// LD __, nn
		LD(m_data.read16((m_data.op >> 4) & 0x3), m_data.nn);
		break;
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
		// XOR A, _
		XOR(m_data.read8(m_data.op & 0x7));
		break;
	default:
		// causes segfault if dynamically loaded
		// (I want to crash anyway)
		throw std::runtime_error{"Instruction not implemented"};
	}
}

extern "C" std::unique_ptr<ICPU> loadCPU(CPU::Data& data, IMMU* mmu) {
	std::cout << "Reloading CPU...\n";
	return std::make_unique<CPU>(data, mmu);
}

void CPU::XOR(const u8& source) {
	m_data.a ^= source;
	m_data.zeroFlag = (m_data.a == 0);
	m_data.negFlag = false;
	m_data.halfFlag = false;
	m_data.carryFlag = false;
}

const std::array<Instruction, 256> CPU::s_instructions = {{
    {0x00, "NOP", 4, 1},

    {0x01, "LD BC, nn", 12, 3},

    {0x02, "LD (BC), A", 8, 1},

    {0x03, "INC BC", 8, 1},

    {0x04, "INC B", 4, 1},

    {0x05, "DEC B", 4, 1},

    {0x06, "LD B, n", 8, 2},

    {0x07, "RLCA", 4, 1},

    {0x08, "LD (nn), SP", 20, 2},

    {0x09, "ADD HL, BC", 8, 1},

    {0x0a, "LD A, (BC)", 8, 1},

    {0x0b, "DEC BC", 8, 1},

    {0x0c, "INC C", 4, 1},

    {0x0d, "DEC", 4, 1},

    {0x0e, "LD C, n", 8, 2},

    {0xf, "RRCA", 4, 1},

    {},

    {0x11, "LD DE, nn", 12, 3},

    {0x12, "LD (DE), A", 8, 1},

    {0x13, "INC DE", 8, 1},

    {0x14, "INC D", 4, 1},

    {0x15, "DEC D", 4, 1},

    {0x16, "LD D, n", 8, 2},

    {0x17, "RLA", 4, 1},

    {0x18, "JR n", 12, 2},

    {0x19, "ADD HL, DE", 8, 1},

    {0x1a, "LD A, (DE)", 8, 1},

    {0x1b, "DEC DE", 8, 1},

    {0x1c, "INC E", 4, 1},

    {0x1d, "DEC E", 4, 1},

    {0x1e, "LD E, n", 8, 2},

    {0x1f, "RRA", 4, 1},

    {0x20, "JR NZ, n", 0, 2},

    {0x21, "LD HL, nn", 12, 3},

    {0x22, "LDI (HL+), A", 8, 1},

    {0x23, "INC HL", 8, 1},

    {0x24, "INC H", 4, 1},

    {0x25, "DEC H", 4, 1},

    {0x26, "LD H, n", 8, 2},

    {0x27, "DAA", 4, 1},

    {0x28, "JR Z, n", 0, 2},

    {0x29, "ADD HL, HL", 8, 1},

    {0x2a, "LDI A, (HL+)", 8, 1},

    {0x2b, "DEC HL", 8, 1},

    {0x2c, "INC L", 4, 1},

    {0x2d, "DEC L", 4, 1},

    {0x2e, "LD L, n", 8, 2},

    {0x2f, "CPL", 4, 1},

    {0x30, "JR NC, n", 0, 1},

    {0x31, "LD SP, nn", 12, 3},

    {0x32, "LDD (HL-), A", 8, 1},

    {0x33, "INC SP", 8, 1},

    {0x34, "INC (HL)", 12, 1},

    {0x35, "DEC (HL)", 12, 1},

    {0x36, "LD (HL), N", 12, 2},

    {0x37, "SCF", 4, 1},

    {0x38, "JR C, n", 0, 2},

    {0x39, "ADD HL, SP", 8, 1},

    {0x3a, "LDD A, (HL-)", 8, 1},

    {0x3b, "DEC SP", 8, 1},

    {0x3c, "INC A", 4, 1},

    {0x3d, "DEC A", 4, 1},

    {0x3e, "LD A, n", 8, 2},

    {0x3f, "CCF", 4, 1},

    {0x40, "LD B, B", 4, 1},

    {0x41, "LD B, C", 4, 1},

    {0x42, "LD B, D", 4, 1},

    {0x43, "LD B, E", 4, 1},

    {0x44, "LD B, H", 4, 1},

    {0x45, "LD B, L", 4, 1},

    {0x46, "LD B, (HL)", 8, 1},

    {0x47, "LD B, A", 4, 1},

    {0x48, "LD C, B", 4, 1},

    {0x49, "LD C, C", 4, 1},

    {0x4a, "LD C, D", 4, 1},

    {0x4b, "LD C, E", 4, 1},

    {0x4c, "LD C, H", 4, 1},

    {0x4d, "LD C, L", 4, 1},

    {0x4e, "LD C, (HL)", 8, 1},

    {0x4f, "LD C, A", 4, 1},

    {0x50, "LD D, B", 4, 1},

    {0x51, "LD D, C", 4, 1},

    {0x52, "LD D, D", 4, 1},

    {0x53, "LD D, E", 4, 1},

    {0x54, "LD D, H", 4, 1},

    {0x55, "LD D, L", 4, 1},

    {0x56, "LD D, (HL)", 8, 1},

    {0x57, "LD D, A", 4, 1},

    {0x58, "LD E, B", 4, 1},

    {0x59, "LD E, C", 4, 1},

    {0x5a, "LD E, D", 4, 1},

    {0x5b, "LD E, E", 4, 1},

    {0x5c, "LD E, H", 4, 1},

    {0x5d, "LD E, L", 4, 1},

    {0x5e, "LD E, (HL)", 8, 1},

    {0x5f, "LD E, A", 4, 1},

    {0x60, "LD H, B", 4, 1},

    {0x61, "LD H, C", 4, 1},

    {0x62, "LD H, D", 4, 1},

    {0x63, "LD H, E", 4, 1},

    {0x64, "LD H, H", 4, 1},

    {0x65, "LD H, L", 4, 1},

    {0x66, "LD H, (HL)", 8, 1},

    {0x67, "LD H, A", 4, 1},

    {0x68, "LD L, B", 4, 1},

    {0x69, "LD L, C", 4, 1},

    {0x6a, "LD L, D", 4, 1},

    {0x6b, "LD L, E", 4, 1},

    {0x6c, "LD L, H", 4, 1},

    {0x6d, "LD L, L", 4, 1},

    {0x6e, "LD L, (HL)", 8, 1},

    {0x6f, "LD L, A", 4, 1},

    {0x70, "LD (HL), B", 8, 1},

    {0x71, "LD (HL), C", 8, 1},

    {0x72, "LD (HL), D", 8, 1},

    {0x73, "LD (HL), E", 8, 1},

    {0x74, "LD (HL), H", 8, 1},

    {0x75, "LD (HL), L", 8, 1},

    {},  // 0x76

    {0x77, "LD (HL), A", 8, 1},

    {0x78, "LD A, B", 4, 1},

    {0x79, "LD A, C", 4, 1},

    {0x7a, "LD A, D", 4, 1},

    {0x7b, "LD A, E", 4, 1},

    {0x7c, "LD A, H", 4, 1},

    {0x7d, "LD A, L", 4, 1},

    {0x7e, "LD A, (HL)", 8, 1},

    {0x7f, "LD A, A", 4, 1},

    {0x80, "ADD A, B", 4, 1},

    {0x81, "ADD A, C", 4, 1},

    {0x82, "ADD A, D", 4, 1},

    {0x83, "ADD A, E", 4, 1},

    {0x84, "ADD A, H", 4, 1},

    {0x85, "ADD A, L", 4, 1},

    {0x86, "ADD A, (HL)", 8, 1},

    {0x87, "ADD A, A", 4, 1},

    {0x88, "ADC A, B", 4, 1},

    {0x89, "ADC A, C", 4, 1},

    {0x8a, "ADC A, D", 4, 1},

    {0x8b, "ADC A, E", 4, 1},

    {0x8c, "ADC A, H", 4, 1},

    {0x8d, "ADC A, L", 4, 1},

    {0x8e, "ADC A, (HL)", 8, 1},

    {0x8f, "ADC A, A", 4, 1},

    {0x90, "SUB A, B", 4, 1},

    {0x91, "SUB A, C", 4, 1},

    {0x92, "SUB A, D", 4, 1},

    {0x93, "SUB A, E", 4, 1},

    {0x94, "SUB A, H", 4, 1},

    {0x95, "SUB A, L", 4, 1},

    {0x96, "SUB A, (HL)", 8, 1},

    {0x97, "SUB A, A", 4, 1},

    {0x98, "SBC A, B", 4, 1},

    {0x99, "SBC A, C", 4, 1},

    {0x9a, "SBC A, D", 4, 1},

    {0x9b, "SBC A, E", 4, 1},

    {0x9c, "SBC A, H", 4, 1},

    {0x9d, "SBC A, L", 4, 1},

    {0x9e, "SBC A, (HL)", 8, 1},

    {0x9f, "SBC A, A", 4, 1},

    {0xa0, "AND A, B", 4, 1},

    {0xa1, "AND A, C", 4, 1},

    {0xa2, "AND A, D", 4, 1},

    {0xa3, "AND A, E", 4, 1},

    {0xa4, "AND A, H", 4, 1},

    {0xa5, "AND A, L", 4, 1},

    {0xa6, "AND A, (HL)", 8, 1},

    {0xa7, "AND A, A", 4, 1},

    {0xa8, "XOR A, B", 4, 1},

    {0xa9, "XOR A, C", 4, 1},

    {0xaa, "XOR A, D", 4, 1},

    {0xab, "XOR A, E", 4, 1},

    {0xac, "XOR A, H", 4, 1},

    {0xad, "XOR A, L", 4, 1},

    {0xae, "XOR A, (HL)", 8, 1},

    {0xaf, "XOR A, A", 4, 1},

    {0xb0, "OR A, B", 4, 1},

    {0xb1, "OR A, C", 4, 1},

    {0xb2, "OR A, D", 4, 1},

    {0xb3, "OR A, E", 4, 1},

    {0xb4, "OR A, H", 4, 1},

    {0xb5, "OR A, L", 4, 1},

    {0xb6, "OR A, (HL)", 8, 1},

    {0xb7, "OR A, A", 4, 1},

    {0xb8, "CP A, B", 4, 1},

    {0xb9, "CP A, C", 4, 1},

    {0xba, "CP A, D", 4, 1},

    {0xbb, "CP A, E", 4, 1},

    {0xbc, "CP A, H", 4, 1},

    {0xbd, "CP A, L", 4, 1},

    {0xbe, "CP A, (HL)", 8, 1},

    {0xbf, "CP A, A", 4, 1},

    {0xc0, "RET NZ", 0, 1},

    {0xc1, "POP BC", 12, 1},

    {0xc2, "JP NZ, nn", 0, 3},

    {0xc3, "JP nn", 0, 3},

    {},  // 0xc4

    {0xc5, "PUSH BC", 16, 1},

    {0xc6, "ADD A, n", 8, 2},

    {},  // 0xc7

    {0xc8, "RET Z", 0, 1},

    {0xc9, "RET", 16, 1},  // !!!

    {0xca, "JP Z, nn", 0, 3},

    {0xcb, "CB", 4, 2},

    {},  // 0xcc

    {0xcd, "CALL nn", 24, 3},  // !!!

    {0xce, "ADC A, n", 8, 2},

    {0xcf, "RST 0x0008", 16, 1},  // !!!

    {0xd0, "RET NC", 0, 1},

    {0xd1, "POP DE", 12, 1},

    {0xd2, "JP NC, nn", 0, 3},

    {},  // 0xd3

    {},  // 0xd4

    {0xd5, "PUSH DE", 16, 1},

    {0xd6, "SUB A, n", 8, 2},

    {},  // 0xd7

    {0xd8, "RET C", 0, 1},

    {0xd9, "RETI", 16, 1},

    {0xda, "JP C, nn", 0, 3},

    {},  // 0xdb

    {},  // 0xdc

    {},  // 0xdd

    {0xde, "SBC A, n", 8, 2},

    {0xdf, "RST 0x0018", 16, 1},  // !!!

    {0xe0, "LD (N+0xff00), A", 12, 2},

    {0xe1, "POP HL", 12, 1},

    {0xe2, "LD (C+0xff00), A", 8, 1},

    {},  // 0xe3

    {},  // 0xe4

    {0xe5, "PUSH HL", 16, 1},

    {0xe6, "AND A, n", 8, 2},

    {},  // 0xe7

    {0xe8, "ADD SP, n", 16, 2},

    {0xe9, "JP HL", 4, 1},  // !!! docs say (HL) but this is wrong (and

    // makes little sense)
    {0xea, "LD (nn), A", 16, 3},

    {},  // 0xeb

    {},  // 0xec

    {},  // 0xed

    {0xee, "XOR A, n", 8, 2},

    {0xef, "RST 0x0028", 16, 1},  // !!!

    {0xf0, "LD A, (N+0xff00)", 12, 2},

    {0xf1, "POP AF", 12, 1},

    {0xf2, "LD A, (C+0xff00)", 8, 1},

    {0xf3, "DI", 4, 1},

    {},  // 0xf4

    {0xf5, "PUSH AF", 16, 1},

    {0xf6, "OR A, n", 8, 2},

    {},  // 0xf7

    {0xf8, "LD HL, SP+n", 12, 2},

    {},  // 0xf9

    {0xfa, "LD A, (nn)", 16, 3},

    {0xfb, "EI", 4, 1},

    {},  // 0xfc

    {},  // 0xfd

    {0xfe, "CP A, n", 8, 2},

    {0xff, "RST 0x0038", 16, 1},  // !!!
}};
