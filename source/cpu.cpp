#include <iostream>

#include "cpu.h"
#include "memref.h"

CPU::CPU(ICPU::Data& data) : ICPU{}, m_data{data} {}

u16 CPU::step() {
	fetch();
	exec();
	processInterrupts();

	return m_data.cycles;
}

void CPU::ADDmixed(u16& target, u16 source16, u8 source8) {
	// http://forums.nesdev.com/viewtopic.php?p=42143#p42143
	m_data.zeroFlag = false;
	m_data.negFlag = false;
	m_data.halfFlag = (((source16 & 0xf) + (source8 & 0xf)) > 0x0f);
	m_data.carryFlag = (((source16 & 0xff) + source8) > 0xff);
	target = static_cast<u16>(source16 + source8);
}

void CPU::ADD16(const u16& source) {
	m_data.carryFlag = (m_data.hl + source) > 0xffff;
	m_data.halfFlag = ((m_data.hl & 0x0fff) + (source & 0x0fff)) > 0x0fff;
	m_data.hl = static_cast<u16>(m_data.hl + source);
	m_data.negFlag = false;
}

void CPU::RST(const u16& addr) {
	m_data.sp -= 2;
	m_data.mmu->write16(m_data.sp, m_data.pc);
	m_data.pc = addr;
}

void CPU::CPL() {
	m_data.a ^= 0xff;
	m_data.negFlag = true;
	m_data.halfFlag = true;
}

void CPU::SCF() {
	m_data.carryFlag = true;
	m_data.halfFlag = false;
	m_data.negFlag = false;
}
void CPU::CCF() {
	m_data.carryFlag = !m_data.carryFlag;
	m_data.halfFlag = false;
	m_data.negFlag = false;
}

void CPU::DI() { m_data.intData->ime = false; }

void CPU::EI() { m_data.intData->ime = true; }

void CPU::DAA() {
	// A binary coded decimal (BCD) is a hexidecimal number 0xuv *thought of
	// as the decimal number uv*, whence the digits u and v are in {0, 1,
	// ..., 9}. If we call the GameBoy's arithmetic operations (e.g. ADD,
	// INC, SUB, DEC, ...) on a BCD, the result need not be a BCD, e.g.
	// 0x9 + 0x1 == 0xa.
	//
	// The purpose of DAA (decimal adjust A?) is to modify the result of an
	// arithmetic operation such that it is a valid BCD and it is the
	// expected result of the previous operation, e.g.
	//
	// LD A, 0x9
	// INC A
	// DAA
	//
	// Then A == 0x10.
	//
	//
	// The logic for a single hexadecimal digit is as follows: suppose we
	// add 0xu + 0xv, where u and v are in {0, 1, ..., 9}, then
	//
	// If 0xu + 0xv < 0xa: The sum is correct both as hexadecimal as well
	// as decimal and we need not do anything.
	//
	// If 0x9 < 0xu + 0xv = 0xa + 0xw < 0x13 (note w is in {0, 1, 2, 3, 4,
	// 5, 7, 8}), then
	//
	// 0xu + 0xv + 0x6 = 0xa + 0x6 + 0xw = 0x1w
	//
	// which is the correct BCD. For all additions, we can determine whether
	// we have to perform the adjustment by checking (0xu + 0xv > 0x9) and
	// halfFlag!
	//
	// TODO: two hex digits, subtraction
	int temp = m_data.a;
	u8 correction = 0;

	if (m_data.a > 0x99 || m_data.carryFlag) {
		correction |= 0x60;
		m_data.carryFlag = true;
	}
	if ((m_data.a & 0x0f) > 0x9 || m_data.halfFlag) {
		correction |= 0x06;
	}

	temp = (m_data.negFlag) ? (temp - correction) : (temp + correction);
	m_data.halfFlag = false;
	m_data.a = static_cast<u8>(temp);
	m_data.zeroFlag = (m_data.a == 0);
}

void CPU::CALL(const bool& cond) {
	m_data.cycles = 12;
	if (cond) {
		m_data.sp -= 2;
		m_data.mmu->write16(m_data.sp, m_data.pc);
		m_data.pc = m_data.nn;
		m_data.cycles += 12;
	}
}

void CPU::PUSH(const u16& reg) {
	m_data.sp -= 2;
	m_data.mmu->write16(m_data.sp, reg);
}

void CPU::POP(u16& reg) {
	reg = m_data.mmu->read16(m_data.sp);
	m_data.sp += 2;
}

void CPU::INC16(u16& reg) { reg++; }

void CPU::DEC16(u16& reg) { reg--; }

void CPU::RET() {
	m_data.pc = m_data.mmu->read16(m_data.sp);
	m_data.sp += 2;
}

void CPU::RETI() {
	RET();
	EI();
}

void CPU::RET(const bool& cond) {
	m_data.cycles = 8;
	if (cond) {
		RET();
		m_data.cycles += 12;
	}
}

void CPU::JP(const bool& cond, const u16& addr) {
	m_data.cycles = 12;
	if (cond) {
		m_data.pc = addr;
		m_data.cycles += 4;
	}
}

void CPU::processInterrupts() {
	if (!m_data.intData->ime) {
		return;
	}
	if (m_data.intData->vBlankEnable && m_data.intData->vBlankFlag) {
		// TODO: investigate cycle count for interrupts
		m_data.cycles += 12;
		m_data.intData->ime = false;
		m_data.intData->vBlankFlag = false;
		// m_data.sp -= 2;
		// m_data.mmu->write16(m_data.sp, m_data.pc);
		// m_data.pc = 0x40;
		RST(0x40);
	} else if (m_data.intData->lcdStatEnable &&
		   m_data.intData->lcdStatFlag) {
		m_data.cycles += 12;
		m_data.intData->ime = false;
		m_data.intData->lcdStatFlag = false;
		RST(0x48);
	}
	// TODO: other interrupts
}

void CPU::fetch() {
	m_data.op = m_data.mmu->read8(m_data.pc);
	m_data.nn = m_data.mmu->read16(m_data.pc + 1);
	m_data.pc += s_instructions[m_data.op].offset;
	m_data.cycles = s_instructions[m_data.op].cycles;
}

void CPU::exec() {
	MemRef mhl{m_data.hl, m_data.mmu};

	switch (m_data.op) {
	case 0x0:  // NOP
		break;
	case 0x01:  // LD BC, nn
		LD(m_data.bc, m_data.nn);
		break;
	case 0x02:  // LD (BC), A
	{
		MemRef mbc{m_data.bc, m_data.mmu};
		LD(mbc, m_data.a);
		break;
	}
	case 0x03:  // INC BC
		INC16(m_data.bc);
		break;
	case 0x04:  // INC B
		INC(m_data.b);
		break;
	case 0x05:  // DEC B
		DEC(m_data.b);
		break;
	case 0x06:  // LD B, n
		LD(m_data.b, m_data.n);
		break;
	case 0x07:  // RLCA
		RLC(m_data.a);
		m_data.zeroFlag = false;
		break;
	case 0x09:  // ADD HL, BC
		ADD16(m_data.bc);
		break;
	case 0x0a:  // LD A, (BC)
	{
		MemRef mbc{m_data.bc, m_data.mmu};
		LD(m_data.a, mbc);
		break;
	}
	case 0x0b:  // DEC BC
		DEC16(m_data.bc);
		break;
	case 0x0c:  // INC C
		INC(m_data.c);
		break;
	case 0x0d:  // DEC C
		DEC(m_data.c);
		break;
	case 0x0e:  // LD C, n
		LD(m_data.c, m_data.n);
		break;
	case 0x0f:  // RRCA
		RRC(m_data.a);
		m_data.zeroFlag = false;
		break;
	case 0x11:  // LD DE, nn
		LD(m_data.de, m_data.nn);
		break;
	case 0x12:  // LD (DE), A
	{
		MemRef mde{m_data.de, m_data.mmu};
		LD(mde, m_data.a);
		break;
	}
	case 0x13:  // INC DE
		INC16(m_data.de);
		break;
	case 0x14:  // INC D
		INC(m_data.d);
		break;
	case 0x15:  // DEC D
		DEC(m_data.d);
		break;
	case 0x16:  // LD D, n
		LD(m_data.d, m_data.n);
		break;
	case 0x17:  // RLA
		RL(m_data.a);
		m_data.zeroFlag = false;
		break;
	case 0x18:  // JR n
		JR(true);
		break;
	case 0x19:  // ADD HL, DE
		ADD16(m_data.de);
		break;
	case 0x1a:  // LD A, (DE)
	{
		MemRef mde{m_data.de, m_data.mmu};
		LD(m_data.a, mde);
		break;
	}
	case 0x1b:  // DEC DE
		DEC16(m_data.de);
		break;
	case 0x1c:  // INC E
		INC(m_data.e);
		break;
	case 0x1d:  // DEC E
		DEC(m_data.e);
		break;
	case 0x1e:  // LD E, n
		LD(m_data.e, m_data.n);
		break;
	case 0x1f:  // RRA
		RR(m_data.a);
		m_data.zeroFlag = false;
		break;
	case 0x20:  // JR NZ, n
		JRn(m_data.zeroFlag);
		break;
	case 0x21:  // LD HL, nn
		LD(m_data.hl, m_data.nn);
		break;
	case 0x22:  // LD (HL+), A
		LD(mhl, m_data.a);
		m_data.hl++;
		break;
	case 0x23:  // INC HL
		INC16(m_data.hl);
		break;
	case 0x24:  // INC H
		INC(m_data.h);
		break;
	case 0x25:  // DEC H
		DEC(m_data.h);
		break;
	case 0x26:  // LD H, n
		LD(m_data.h, m_data.n);
		break;
	case 0x27:  // DAA
		DAA();
		break;
	case 0x28:  // JR Z, n
		JR(m_data.zeroFlag);
		break;
	case 0x29:  // ADD HL, HL
		ADD16(m_data.hl);
		break;
	case 0x2a:  // LD A, (HL+)
		LD(m_data.a, mhl);
		m_data.hl++;
		break;
	case 0x2b:  // DEC HL
		DEC16(m_data.hl);
		break;
	case 0x2c:  // INC L
		INC(m_data.l);
		break;
	case 0x2d:  // DEC L
		DEC(m_data.l);
		break;
	case 0x2e:  // LD L, n
		LD(m_data.l, m_data.n);
		break;
	case 0x2f:  // CPL
		CPL();
		break;
	case 0x30:  // JR NC, n
		JRn(m_data.carryFlag);
		break;
	case 0x31:  // LD SP, nn
		LD(m_data.sp, m_data.nn);
		break;
	case 0x32:  // LD (HL-), A
		LD(mhl, m_data.a);
		m_data.hl--;
		break;
	case 0x33:  // INC SP
		INC16(m_data.sp);
		break;
	case 0x34:  // INC (HL)
		INC(mhl);
		break;
	case 0x35:  // DEC (HL)
		DEC(mhl);
		break;
	case 0x36:  // LD (HL), n
		LD(mhl, m_data.n);
		break;
	case 0x37:  // SCF
		SCF();
		break;
	case 0x38:  // JR C, n
		JR(m_data.carryFlag);
		break;
	case 0x39:  // ADD HL, SP
		ADD16(m_data.sp);
		break;
	case 0x3a:  // LD A, (HL-)
		LD(m_data.a, mhl);
		m_data.hl--;
		break;
	case 0x3b:  // DEC SP
		DEC16(m_data.sp);
		break;
	case 0x3c:  // INC A
		INC(m_data.a);
		break;
	case 0x3d:  // DEC A
		DEC(m_data.a);
		break;
	case 0x3e:  // LD A, n
		LD(m_data.a, m_data.n);
		break;
	case 0x3f:  // CCF
		CCF();
		break;

	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	// case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	// case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	// case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	// case 0x5e:
	case 0x5f:
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	// case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	// case 0x6e:
	case 0x6f:
	// case 0x70:
	// case 0x71:
	// case 0x72:
	// case 0x73:
	// case 0x74:
	// case 0x75:
	// case 0x76:
	// case 0x77:
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	// case 0x7e:
	case 0x7f:  // LD _, _
		LD(m_data.read8((m_data.op >> 3) & 0x7),
		   m_data.read8(m_data.op & 0x7));
		break;
	case 0x46:
	case 0x4e:
	case 0x56:
	case 0x5e:
	case 0x66:
	case 0x6e:
	// case 0x76:
	case 0x7e:  // LD _, (HL)
		LD(m_data.read8((m_data.op >> 3) & 0x7), mhl);
		break;

	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x77:  // LD (HL), _
		LD(mhl, m_data.read8(m_data.op & 0x7));
		break;

	case 0x76:  // HALT, ignore for now
		break;

	case 0x80:  // ADD A, B
		ADD(m_data.b);
		break;
	case 0x81:  // ADD A, C
		ADD(m_data.c);
		break;
	case 0x82:  // ADD A, D
		ADD(m_data.d);
		break;
	case 0x83:  // ADD A, E
		ADD(m_data.e);
		break;
	case 0x84:  // ADD A, H
		ADD(m_data.h);
		break;
	case 0x85:  // ADD A, L
		ADD(m_data.l);
		break;
	case 0x86:  // ADD A, (HL)
		ADD(mhl);
		break;
	case 0x87:  // ADD A, A
		ADD(m_data.a);
		break;
	case 0x88:  // ADC A, B
		ADC(m_data.b);
		break;
	case 0x89:  // ADC A, C
		ADC(m_data.c);
		break;
	case 0x8a:  // ADC A, D
		ADC(m_data.d);
		break;
	case 0x8b:  // ADC A, E
		ADC(m_data.e);
		break;
	case 0x8c:  // ADC A, H
		ADC(m_data.h);
		break;
	case 0x8d:  // ADC A, L
		ADC(m_data.l);
		break;
	case 0x8e:  // ADC A, (HL)
		ADC(mhl);
		break;
	case 0x8f:  // ADC A, A
		ADC(m_data.a);
		break;
	case 0x090:  // SUB A, B
		SUB(m_data.b);
		break;
	case 0x91:  // SUB A, C
		SUB(m_data.c);
		break;
	case 0x92:  // SUB A, D
		SUB(m_data.d);
		break;
	case 0x93:  // SUB A, E
		SUB(m_data.e);
		break;
	case 0x94:  // SUB A, H
		SUB(m_data.h);
		break;
	case 0x95:  // SUB A, L
		SUB(m_data.l);
		break;
	case 0x96:  // SUB A, (HL)
		SUB(mhl);
		break;
	case 0x97:  // SUB A, A
		SUB(m_data.a);
		break;
	case 0x98:  // SBC B
		SBC(m_data.b);
		break;
	case 0x99:  // SBC C
		SBC(m_data.c);
		break;
	case 0x9a:  // SBC D
		SBC(m_data.d);
		break;
	case 0x9b:  // SBC E
		SBC(m_data.e);
		break;
	case 0x9c:  // SBC H
		SBC(m_data.h);
		break;
	case 0x9d:  // SBC L
		SBC(m_data.l);
		break;
	case 0x9e:  // SBC (HL)
		SBC(mhl);
		break;
	case 0x9f:  // SBC A
		SBC(m_data.a);
		break;
	case 0xa0:  // AND B
		AND(m_data.b);
		break;
	case 0xa1:  // AND C
		AND(m_data.c);
		break;
	case 0xa2:  // AND D
		AND(m_data.d);
		break;
	case 0xa3:  // AND E
		AND(m_data.e);
		break;
	case 0xa4:  // AND H
		AND(m_data.h);
		break;
	case 0xa5:  // AND L
		AND(m_data.l);
		break;
	case 0xa6:  // AND (HL)
		AND(mhl);
		break;
	case 0xa7:  // AND A
		AND(m_data.a);
		break;

	case 0xa8:  // XOR B
		XOR(m_data.b);
		break;
	case 0xa9:  // XOR C
		XOR(m_data.c);
		break;
	case 0xaa:  // XOR D
		XOR(m_data.d);
		break;
	case 0xab:  // XOR E
		XOR(m_data.e);
		break;
	case 0xac:  // XOR H
		XOR(m_data.h);
		break;
	case 0xad:  // XOR L
		XOR(m_data.l);
		break;
	case 0xae:  // XOR (HL)
		XOR(mhl);
		break;
	case 0xaf:  // XOR A
		XOR(m_data.a);
		break;
	case 0xb0:  // OR B
		OR(m_data.b);
		break;
	case 0xb1:  // OR C
		OR(m_data.c);
		break;
	case 0xb2:  // OR D
		OR(m_data.d);
		break;
	case 0xb3:  // OR E
		OR(m_data.e);
		break;
	case 0xb4:  // OR H
		OR(m_data.h);
		break;
	case 0xb5:  // OR L
		OR(m_data.l);
		break;
	case 0xb6:  // OR (HL)
		OR(mhl);
		break;
	case 0xb7:  // OR A
		OR(m_data.a);
		break;

	case 0xb8:  // CP B
		CP(m_data.b);
		break;
	case 0xb9:  // CP C
		CP(m_data.c);
		break;
	case 0xba:  // CP D
		CP(m_data.d);
		break;
	case 0xbb:  // CP E
		CP(m_data.e);
		break;
	case 0xbc:  // CP H
		CP(m_data.h);
		break;
	case 0xbd:  // CP L
		CP(m_data.l);
		break;
	case 0xbe:  // CP (HL)
		CP(mhl);
		break;
	case 0xbf:  // CP A
		CP(m_data.a);
		break;
	case 0xc0:  // RET NZ
		RET(!m_data.zeroFlag);
		break;
	case 0xc1:  // POP BC
		POP(m_data.bc);
		break;
	case 0xc2:  // JP NZ, nn
		JP(!m_data.zeroFlag, m_data.nn);
		break;
	case 0xc3:  // JP nn
		JP(true, m_data.nn);
		break;
	case 0xc4:  // CALL NZ, nn
		CALL(!m_data.zeroFlag);
		break;
	case 0xc5:  // PUSH BC
		PUSH(m_data.bc);
		break;
	case 0xc6:  // ADD A, n
		ADD(m_data.n);
		break;
	case 0xc7:  // RST 0x00
		RST(0);
		break;
	case 0xc8:  // RET Z
		RET(m_data.zeroFlag);
		break;
	case 0xc9:  // RET
		RET();
		break;
	case 0xca:  // JP Z, nn
		JP(m_data.zeroFlag, m_data.nn);
		break;
	case 0xcb:  // CB
		CB();
		break;
	case 0xcc:  // CALL Z, nn
		CALL(m_data.zeroFlag);
		break;
	case 0xcd:  // CALL nn
		CALL(true);
		break;
	case 0xce:  // ADC A, n
		ADC(m_data.n);
		break;
	case 0xcf:  // RST 0x08
		RST(0x08);
		break;
	case 0xd0:  // RET NC
		RET(!m_data.carryFlag);
		break;
	case 0xd1:  // POP DE
		POP(m_data.de);
		break;
	case 0xd2:  // JP NC, nn
		JP(!m_data.carryFlag, m_data.nn);
		break;
	case 0xd4:  // CALL NC, nn
		CALL(!m_data.carryFlag);
		break;
	case 0xd5:  // PUSH DE
		PUSH(m_data.de);
		break;
	case 0xd6:  // SUB A, n
		SUB(m_data.n);
		break;
	case 0xd7:  // RST 0x10
		RST(0x10);
		break;
	case 0xd8:  // RET C
		RET(m_data.carryFlag);
		break;
	case 0xd9:  // RETI
		RETI();
		break;
	case 0xda:  // JP C, nn
		JP(m_data.carryFlag, m_data.nn);
		break;
	case 0xdc:  // CALL C, nn
		CALL(m_data.carryFlag);
		break;
	case 0xde:  // SBC n
		SBC(m_data.n);
		break;
	case 0xdf:  // RST 0x18
		RST(0x18);
		break;
	case 0xe0:  // LD (n + 0xff00), A
	{
		MemRef mn{m_data.n + 0xff00, m_data.mmu};
		LD(mn, m_data.a);
		break;
	}
	case 0xe1:  // POP HL
		POP(m_data.hl);
		break;
	case 0xe2:  // LD (C + 0xff00), A
	{
		MemRef mc{m_data.c + 0xff00, m_data.mmu};
		LD(mc, m_data.a);
		break;
	}
	case 0xe5:  // PUSH HL
		PUSH(m_data.hl);
		break;
	case 0xe6:  // AND n
		AND(m_data.n);
		break;
	case 0xe7:  // RST 0x20
		RST(0x20);
		break;
	case 0xe8:  // ADD SP, n
		ADDmixed(m_data.sp, m_data.sp, m_data.n);
		break;
	case 0xe9:  // JP HL (error in docs!!!)
		JP(true, m_data.hl);
		m_data.cycles = 4;
		break;
	case 0xea:  // LD (nn), A
	{
		MemRef mnn{m_data.nn, m_data.mmu};
		LD(mnn, m_data.a);
		break;
	}
	case 0xee:  // XOR n
		XOR(m_data.n);
		break;
	case 0xef:  // RST 0x28
		RST(0x28);
		break;
	case 0xf0:  // LD A, (n + 0xff00)
	{
		MemRef mn{m_data.n + 0xff00, m_data.mmu};
		LD(m_data.a, mn);
		break;
	}
	case 0xf1:  // POP AF
		POP(m_data.af);
		break;
	case 0xf2:  // LD A, (C + 0xff00)
	{
		MemRef mc{m_data.c + 0xff00, m_data.mmu};
		LD(m_data.a, mc);
		break;
	}
	case 0xf3:  // DI
		DI();
		break;
	case 0xf5:  // PUSH AF
		PUSH(m_data.af);
		break;
	case 0xf6:  // OR n
		OR(m_data.n);
		break;
	case 0xf7:  // RST 0x30
		RST(0x30);
		break;
	case 0xf8:  // LD HL, SP + n
		ADDmixed(m_data.hl, m_data.sp, m_data.n);
		break;
	case 0xfa:  // LD A, (nn)
	{
		MemRef mnn{m_data.nn, m_data.mmu};
		LD(m_data.a, mnn);
		break;
	}
	case 0xfb:  // EI
		EI();
		break;
	case 0xfe:  // CP n
		CP(m_data.n);
		break;
	case 0xff:  // RST 0x38
		RST(0x38);
		break;
	default:
		// causes segfault if dynamically loaded
		// (I want to crash anyway)
		std::cout << "Instruction 0x" << std::hex << +m_data.op
			  << " not implemented\n";
		throw std::runtime_error{"Instruction not implemented"};
	}
}

extern "C" std::unique_ptr<ICPU> loadCPU(CPU::Data& data) {
	std::cout << "Reloading CPU...\n";
	return std::make_unique<CPU>(data);
}

void CPU::CB() {
	m_data.cycles += CPU::s_extended[m_data.n].cycles;
	MemRef mhl{m_data.hl, m_data.mmu};

	switch (m_data.n) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	// case 0x06:
	case 0x07:  // RLC _
		RLC(m_data.read8(m_data.n & 0x7));
		break;
	case 0x06:  // RLC (HL)
		RLC(mhl);
		break;

	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	// case 0x0e:
	case 0x0f:  // RRC _
		RRC(m_data.read8(m_data.n & 0x7));
		break;

	case 0x0e:  // RRC (HL)
		RRC(mhl);
		break;
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	// case 0x16:
	case 0x17:  // RL _
		RL(m_data.read8(m_data.n & 0x7));
		break;
	case 0x16:  // RL (HL)
		RL(mhl);
		break;
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	// case 0x1e:
	case 0x1f:  // RR _
		RR(m_data.read8(m_data.n & 0x7));
		break;
	case 0x1e:  // RR (HL)
		RR(mhl);
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	// case 0x26:
	case 0x27:  // SLA _
		SLA(m_data.read8(m_data.n & 0x7));
		break;
	case 0x26:  // SLA (HL)
		SLA(mhl);
		break;
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	// case 0x2e:
	case 0x2f:  // SRA _
		SRA(m_data.read8(m_data.n & 0x7));
		break;
	case 0x2e:  // SRA (HL)
		SRA(mhl);
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	// case 0x36:
	case 0x37:  // SWAP _
		SWAP(m_data.read8(m_data.n & 0x7));
		break;
	case 0x36:
		SWAP(mhl);
		break;

	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	// case 0x3e:
	case 0x3f:  // SRL _
		SRL(m_data.read8(m_data.n & 0x7));
		break;
	case 0x3e:
		SRL(mhl);
		break;

	case 0x40:
	case 0x41:
	case 0x42:
	case 0x43:
	case 0x44:
	case 0x45:
	// case 0x46:
	case 0x47:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4b:
	case 0x4c:
	case 0x4d:
	// case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x51:
	case 0x52:
	case 0x53:
	case 0x54:
	case 0x55:
	// case 0x56:
	case 0x57:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5d:
	// case 0x5e:
	case 0x5f:
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	// case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6b:
	case 0x6c:
	case 0x6d:
	// case 0x6e:
	case 0x6f:
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	// case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7a:
	case 0x7b:
	case 0x7c:
	case 0x7d:
	// case 0x7e:
	case 0x7f:  // BIT _, _
		BIT(BitRef<u8>{m_data.read8(m_data.nn & 0x7),
			       (m_data.nn >> 3) & 0x7});
		break;
	case 0x46:
	case 0x4e:
	case 0x56:
	case 0x5e:
	case 0x66:
	case 0x6e:
	case 0x76:
	case 0x7e:  // BIT (HL), _
		BIT(BitRef<MemRef>{mhl, (m_data.nn >> 3) & 0x7});
		break;

	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	case 0x84:
	case 0x85:
	// case 0x86:
	case 0x87:
	case 0x88:
	case 0x89:
	case 0x8a:
	case 0x8b:
	case 0x8c:
	case 0x8d:
	// case 0x8e:
	case 0x8f:
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	// case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9b:
	case 0x9c:
	case 0x9d:
	// case 0x9e:
	case 0x9f:
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	// case 0xa6:
	case 0xa7:
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xab:
	case 0xac:
	case 0xad:
	// case 0xae:
	case 0xaf:
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
	case 0xb4:
	case 0xb5:
	// case 0xb6:
	case 0xb7:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbb:
	case 0xbc:
	case 0xbd:
	// case 0xbe:
	case 0xbf:  // RES _, _
		RES(BitRef<u8>{m_data.read8(m_data.nn & 0x7),
			       (m_data.nn >> 3) & 0x7});
		break;
	case 0x86:
	case 0x8e:
	case 0x96:
	case 0x9e:
	case 0xa6:
	case 0xae:
	case 0xb6:
	case 0xbe:  // RES (HL), _
		RES(BitRef<MemRef>{mhl, (m_data.nn >> 3) & 0x7});
		break;

	case 0xc0:
	case 0xc1:
	case 0xc2:
	case 0xc3:
	case 0xc4:
	case 0xc5:
	// case 0xc6:
	case 0xc7:
	case 0xc8:
	case 0xc9:
	case 0xca:
	case 0xcb:
	case 0xcc:
	case 0xcd:
	// case 0xce:
	case 0xcf:
	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	// case 0xd6:
	case 0xd7:
	case 0xd8:
	case 0xd9:
	case 0xda:
	case 0xdb:
	case 0xdc:
	case 0xdd:
	// case 0xde:
	case 0xdf:
	case 0xe0:
	case 0xe1:
	case 0xe2:
	case 0xe3:
	case 0xe4:
	case 0xe5:
	// case 0xe6:
	case 0xe7:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xeb:
	case 0xec:
	case 0xed:
	// case 0xee:
	case 0xef:
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	// case 0xf6:
	case 0xf7:
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
	case 0xfc:
	case 0xfd:
	// case 0xfe:
	case 0xff:  // SET _, _
		SET(BitRef<u8>{m_data.read8(m_data.nn & 0x7),
			       (m_data.nn >> 3) & 0x7});
		break;
	case 0xc6:
	case 0xce:
	case 0xd6:
	case 0xde:
	case 0xe6:
	case 0xee:
	case 0xf6:
	case 0xfe:  // SET (HL), _
		SET(BitRef<MemRef>{mhl, (m_data.nn >> 3) & 0x7});
		break;
	}
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

    {0x0d, "DEC C", 4, 1},

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

    {0x18, "JR n", 0, 2},

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

    {0x30, "JR NC, n", 0, 2},

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

    {0x76, "HALT", 4, 1},

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

    {0xc4, "CALL NZ, nn", 0, 3},

    {0xc5, "PUSH BC", 16, 1},

    {0xc6, "ADD A, n", 8, 2},

    {0xc7, "RST 0x00", 16, 1},

    {0xc8, "RET Z", 0, 1},

    {0xc9, "RET", 16, 1},  // !!!

    {0xca, "JP Z, nn", 0, 3},

    {0xcb, "CB", 4, 2},

    {0xcc, "CALL Z, nn", 0, 3},

    {0xcd, "CALL nn", 0, 3},  // !!!

    {0xce, "ADC A, n", 8, 2},

    {0xcf, "RST 0x08", 16, 1},

    {0xd0, "RET NC", 0, 1},

    {0xd1, "POP DE", 12, 1},

    {0xd2, "JP NC, nn", 0, 3},

    {},  // 0xd3

    {0xd4, "CALL NC, nn", 0, 3},

    {0xd5, "PUSH DE", 16, 1},

    {0xd6, "SUB A, n", 8, 2},

    {0xd7, "RST 0x10", 16, 1},

    {0xd8, "RET C", 0, 1},

    {0xd9, "RETI", 16, 1},

    {0xda, "JP C, nn", 0, 3},

    {},  // 0xdb

    {0xdc, "CALL C, nn", 0, 3},

    {},  // 0xdd

    {0xde, "SBC A, n", 8, 2},

    {0xdf, "RST 0x18", 16, 1},

    {0xe0, "LD (N+0xff00), A", 12, 2},

    {0xe1, "POP HL", 12, 1},

    {0xe2, "LD (C+0xff00), A", 8, 1},

    {},  // 0xe3

    {},  // 0xe4

    {0xe5, "PUSH HL", 16, 1},

    {0xe6, "AND A, n", 8, 2},

    {0xe7, "RST 0x20", 16, 1},

    {0xe8, "ADD SP, n", 16, 2},

    {0xe9, "JP HL", 4, 1},  // !!! docs say (HL) but this is wrong (and

    // makes little sense)
    {0xea, "LD (nn), A", 16, 3},

    {},  // 0xeb

    {},  // 0xec

    {},  // 0xed

    {0xee, "XOR A, n", 8, 2},

    {0xef, "RST 0x28", 16, 1},

    {0xf0, "LD A, (N+0xff00)", 12, 2},

    {0xf1, "POP AF", 12, 1},

    {0xf2, "LD A, (C+0xff00)", 8, 1},

    {0xf3, "DI", 4, 1},

    {},  // 0xf4

    {0xf5, "PUSH AF", 16, 1},

    {0xf6, "OR A, n", 8, 2},

    {0xf7, "RST 0x30", 16, 1},

    {0xf8, "LD HL, SP+n", 12, 2},

    {},  // 0xf9

    {0xfa, "LD A, (nn)", 16, 3},

    {0xfb, "EI", 4, 1},

    {},  // 0xfc

    {},  // 0xfd

    {0xfe, "CP A, n", 8, 2},

    {0xff, "RST 0x38", 16, 1},
}};

const std::array<Instruction, 256> CPU::s_extended = {{
    {0x00, "RLC B", 8, 0},

    {0x01, "RLC C", 8, 0},

    {0x02, "RLC D", 8, 0},

    {0x03, "RLC E", 8, 0},

    {0x04, "RLC H", 8, 0},

    {0x05, "RLC L", 8, 0},

    {0x06, "RLC (HL)", 16, 0},

    {0x07, "RLC A", 8, 0},

    {0x08, "RRC B", 8, 0},

    {0x09, "RRC C", 8, 0},

    {0x0a, "RRC D", 8, 0},

    {0x0b, "RRC E", 8, 0},

    {0x0c, "RRC H", 8, 0},

    {0x0d, "RRC L", 8, 0},

    {0x0e, "RRC (HL)", 16, 0},

    {0x0f, "RRC A", 8, 0},

    {0x10, "RL B", 8, 0},

    {0x11, "RL C", 8, 0},

    {0x12, "RL D", 8, 0},

    {0x13, "RL E", 8, 0},

    {0x14, "RL H", 8, 0},

    {0x15, "RL L", 8, 0},

    {0x16, "RL (HL)", 16, 0},

    {0x17, "RL A", 8, 0},

    {0x18, "RR B", 8, 0},

    {0x19, "RR C", 8, 0},

    {0x1a, "RR D", 8, 0},

    {0x1b, "RR E", 8, 0},

    {0x1c, "RR H", 8, 0},

    {0x1d, "RR L", 8, 0},

    {0x1e, "RR (HL)", 16, 0},

    {0x1f, "RR A", 8, 0},

    {0x20, "SLA B", 8, 0},

    {0x21, "SLA C", 8, 0},

    {0x22, "SLA D", 8, 0},

    {0x23, "SLA E", 8, 0},

    {0x24, "SLA H", 8, 0},

    {0x25, "SLA L", 8, 0},

    {0x26, "SLA (HL)", 16, 0},

    {0x27, "SLA A", 8, 0},

    {0x28, "SRA B", 8, 0},

    {0x29, "SRA C", 8, 0},

    {0x2a, "SRA D", 8, 0},

    {0x2b, "SRA E", 8, 0},

    {0x2c, "SRA H", 8, 0},

    {0x2d, "SRA L", 8, 0},

    {0x2e, "SRA (HL)", 16, 0},

    {0x2f, "SRA A", 8, 0},

    {0x30, "SWAP B", 8, 0},

    {0x31, "SWAP C", 8, 0},

    {0x32, "SWAP D", 8, 0},

    {0x33, "SWAP E", 8, 0},

    {0x34, "SWAP H", 8, 0},

    {0x35, "SWAP L", 8, 0},

    {0x36, "SWAP (HL)", 16, 0},

    {0x37, "SWAP A", 8, 0},

    {0x38, "SRL B", 8, 0},

    {0x39, "SRL C", 8, 0},

    {0x3a, "SRL D", 8, 0},

    {0x3b, "SRL E", 8, 0},

    {0x3c, "SRL H", 8, 0},

    {0x3d, "SRL L", 8, 0},

    {0x3e, "SRL (HL)", 16, 0},

    {0x3f, "SRL A", 8, 0},

    {0x40, "BIT 0, B", 8, 0},

    {0x41, "BIT 0, C", 8, 0},

    {0x42, "BIT 0, D", 8, 0},

    {0x43, "BIT 0, E", 8, 0},

    {0x44, "BIT 0, H", 8, 0},

    {0x45, "BIT 0, L", 8, 0},

    {0x46, "BIT 0, (HL)", 16, 0},

    {0x47, "BIT 0, A", 8, 0},

    {0x48, "BIT 1, B", 8, 0},

    {0x49, "BIT 1, C", 8, 0},

    {0x4a, "BIT 1, D", 8, 0},

    {0x4b, "BIT 1, E", 8, 0},

    {0x4c, "BIT 1, H", 8, 0},

    {0x4d, "BIT 1, L", 8, 0},

    {0x4e, "BIT 1, (HL)", 16, 0},

    {0x4f, "BIT 1, A", 8, 0},

    {0x50, "BIT 2, B", 8, 0},

    {0x51, "BIT 2, C", 8, 0},

    {0x52, "BIT 2, D", 8, 0},

    {0x53, "BIT 2, E", 8, 0},

    {0x54, "BIT 2, H", 8, 0},

    {0x55, "BIT 2, L", 8, 0},

    {0x56, "BIT 2, (HL)", 16, 0},

    {0x57, "BIT 2, A", 8, 0},

    {0x58, "BIT 3, B", 8, 0},

    {0x59, "BIT 3, C", 8, 0},

    {0x5a, "BIT 3, D", 8, 0},

    {0x5b, "BIT 3, E", 8, 0},

    {0x5c, "BIT 3, H", 8, 0},

    {0x5d, "BIT 3, L", 8, 0},

    {0x5e, "BIT 3, (HL)", 16, 0},

    {0x5f, "BIT 3, A", 8, 0},

    {0x60, "BIT 4, B", 8, 0},

    {0x61, "BIT 4, C", 8, 0},

    {0x62, "BIT 4, D", 8, 0},

    {0x63, "BIT 4, E", 8, 0},

    {0x64, "BIT 4, H", 8, 0},

    {0x65, "BIT 4, L", 8, 0},

    {0x66, "BIT 4, (HL)", 16, 0},

    {0x67, "BIT 4, A", 8, 0},

    {0x68, "BIT 5, B", 8, 0},

    {0x69, "BIT 5, C", 8, 0},

    {0x6a, "BIT 5, D", 8, 0},

    {0x6b, "BIT 5, E", 8, 0},

    {0x6c, "BIT 5, H", 8, 0},

    {0x6d, "BIT 5, L", 8, 0},

    {0x6e, "BIT 5, (HL)", 16, 0},

    {0x6f, "BIT 5, A", 8, 0},

    {0x70, "BIT 6, B", 8, 0},

    {0x71, "BIT 6, C", 8, 0},

    {0x72, "BIT 6, D", 8, 0},

    {0x73, "BIT 6, E", 8, 0},

    {0x74, "BIT 6, H", 8, 0},

    {0x75, "BIT 6, L", 8, 0},

    {0x76, "BIT 6, (HL)", 16, 0},

    {0x77, "BIT 6, A", 8, 0},

    {0x78, "BIT 7, B", 8, 0},

    {0x79, "BIT 7, C", 8, 0},

    {0x7a, "BIT 7, D", 8, 0},

    {0x7b, "BIT 7, E", 8, 0},

    {0x7c, "BIT 7, H", 8, 0},

    {0x7d, "BIT 7, L", 8, 0},

    {0x7e, "BIT 7, (HL)", 16, 0},

    {0x7f, "BIT 7, A", 8, 0},

    {0x80, "RES 0, B", 8, 0},

    {0x81, "RES 0, C", 8, 0},

    {0x82, "RES 0, D", 8, 0},

    {0x83, "RES 0, E", 8, 0},

    {0x84, "RES 0, H", 8, 0},

    {0x85, "RES 0, L", 8, 0},

    {0x86, "RES 0, (HL)", 16, 0},

    {0x87, "RES 0, A", 8, 0},

    {0x88, "RES 1, B", 8, 0},

    {0x89, "RES 1, C", 8, 0},

    {0x8a, "RES 1, D", 8, 0},

    {0x8b, "RES 1, E", 8, 0},

    {0x8c, "RES 1, H", 8, 0},

    {0x8d, "RES 1, L", 8, 0},

    {0x8e, "RES 1, (HL)", 16, 0},

    {0x8f, "RES 1, A", 8, 0},

    {0x90, "RES 2, B", 8, 0},

    {0x91, "RES 2, C", 8, 0},

    {0x92, "RES 2, D", 8, 0},

    {0x93, "RES 2, E", 8, 0},

    {0x94, "RES 2, H", 8, 0},

    {0x95, "RES 2, L", 8, 0},

    {0x96, "RES 2, (HL)", 16, 0},

    {0x97, "RES 2, A", 8, 0},

    {0x98, "RES 3, B", 8, 0},

    {0x99, "RES 3, C", 8, 0},

    {0x9a, "RES 3, D", 8, 0},

    {0x9b, "RES 3, E", 8, 0},

    {0x9c, "RES 3, H", 8, 0},

    {0x9d, "RES 3, L", 8, 0},

    {0x9e, "RES 3, (HL)", 16, 0},

    {0x9f, "RES 3, A", 8, 0},

    {0xa0, "RES 4, B", 8, 0},

    {0xa1, "RES 4, C", 8, 0},

    {0xa2, "RES 4, D", 8, 0},

    {0xa3, "RES 4, E", 8, 0},

    {0xa4, "RES 4, H", 8, 0},

    {0xa5, "RES 4, L", 8, 0},

    {0xa6, "RES 4, (HL)", 16, 0},

    {0xa7, "RES 4, A", 8, 0},

    {0xa8, "RES 5, B", 8, 0},

    {0xa9, "RES 5, C", 8, 0},

    {0xaa, "RES 5, D", 8, 0},

    {0xab, "RES 5, E", 8, 0},

    {0xac, "RES 5, H", 8, 0},

    {0xad, "RES 5, L", 8, 0},

    {0xae, "RES 5, (HL)", 16, 0},

    {0xaf, "RES 5, A", 8, 0},

    {0xb0, "RES 6, B", 8, 0},

    {0xb1, "RES 6, C", 8, 0},

    {0xb2, "RES 6, D", 8, 0},

    {0xb3, "RES 6, E", 8, 0},

    {0xb4, "RES 6, H", 8, 0},

    {0xb5, "RES 6, L", 8, 0},

    {0xb6, "RES 6, (HL)", 16, 0},

    {0xb7, "RES 6, A", 8, 0},

    {0xb8, "RES 7, B", 8, 0},

    {0xb9, "RES 7, C", 8, 0},

    {0xba, "RES 7, D", 8, 0},

    {0xbb, "RES 7, E", 8, 0},

    {0xbc, "RES 7, H", 8, 0},

    {0xbd, "RES 7, L", 8, 0},

    {0xbe, "RES 7, (HL)", 16, 0},

    {0xbf, "RES 7, A", 8, 0},

    {0xc0, "SET 0, B", 8, 0},

    {0xc1, "SET 0, C", 8, 0},

    {0xc2, "SET 0, D", 8, 0},

    {0xc3, "SET 0, E", 8, 0},

    {0xc4, "SET 0, H", 8, 0},

    {0xc5, "SET 0, L", 8, 0},

    {0xc6, "SET 0, (HL)", 16, 0},

    {0xc7, "SET 0, A", 8, 0},

    {0xc8, "SET 1, B", 8, 0},

    {0xc9, "SET 1, C", 8, 0},

    {0xca, "SET 1, D", 8, 0},

    {0xcb, "SET 1, E", 8, 0},

    {0xcc, "SET 1, H", 8, 0},

    {0xcd, "SET 1, L", 8, 0},

    {0xce, "SET 1, (HL)", 16, 0},

    {0xcf, "SET 1, A", 8, 0},

    {0xd0, "SET 2, B", 8, 0},

    {0xd1, "SET 2, C", 8, 0},

    {0xd2, "SET 2, D", 8, 0},

    {0xd3, "SET 2, E", 8, 0},

    {0xd4, "SET 2, H", 8, 0},

    {0xd5, "SET 2, L", 8, 0},

    {0xd6, "SET 2, (HL)", 16, 0},

    {0xd7, "SET 2, A", 8, 0},

    {0xd8, "SET 3, B", 8, 0},

    {0xd9, "SET 3, C", 8, 0},

    {0xda, "SET 3, D", 8, 0},

    {0xdb, "SET 3, E", 8, 0},

    {0xdc, "SET 3, H", 8, 0},

    {0xdd, "SET 3, L", 8, 0},

    {0xde, "SET 3, (HL)", 16, 0},

    {0xdf, "SET 3, A", 8, 0},

    {0xe0, "SET 4, B", 8, 0},

    {0xe1, "SET 4, C", 8, 0},

    {0xe2, "SET 4, D", 8, 0},

    {0xe3, "SET 4, E", 8, 0},

    {0xe4, "SET 4, H", 8, 0},

    {0xe5, "SET 4, L", 8, 0},

    {0xe6, "SET 4, (HL)", 16, 0},

    {0xe7, "SET 4, A", 8, 0},

    {0xe8, "SET 5, B", 8, 0},

    {0xe9, "SET 5, C", 8, 0},

    {0xea, "SET 5, D", 8, 0},

    {0xeb, "SET 5, E", 8, 0},

    {0xec, "SET 5, H", 8, 0},

    {0xed, "SET 5, L", 8, 0},

    {0xee, "SET 5, (HL)", 16, 0},

    {0xef, "SET 5, A", 8, 0},

    {0xf0, "SET 6, B", 8, 0},

    {0xf1, "SET 6, C", 8, 0},

    {0xf2, "SET 6, D", 8, 0},

    {0xf3, "SET 6, E", 8, 0},

    {0xf4, "SET 6, H", 8, 0},

    {0xf5, "SET 6, L", 8, 0},

    {0xf6, "SET 6, (HL)", 16, 0},

    {0xf7, "SET 6, A", 8, 0},

    {0xf8, "SET 7, B", 8, 0},

    {0xf9, "SET 7, C", 8, 0},

    {0xfa, "SET 7, D", 8, 0},

    {0xfb, "SET 7, E", 8, 0},

    {0xfc, "SET 7, H", 8, 0},

    {0xfd, "SET 7, L", 8, 0},

    {0xfe, "SET 7, (HL)", 16, 0},

    {0xff, "SET 7, A", 8, 0},
}};
