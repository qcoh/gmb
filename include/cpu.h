#pragma once

#include "icpu.h"
#include "immu.h"
#include "instruction.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&);
	virtual ~CPU() = default;

	virtual u16 step() override;

	void fetch();
	void exec();
	void processInterrupts();

	static const std::array<Instruction, 256> s_instructions;
	static const std::array<Instruction, 256> s_extended;

private:
	ICPU::Data& m_data;

	// misc
	void DI();
	void EI();

	// controlflow
	template <typename T>
	void JR(const T& cond) {
		m_data.cycles = 8;
		if (cond) {
			m_data.pc += static_cast<int8_t>(m_data.n);
			m_data.cycles += 4;
		}
	}

	template <typename T>
	void JRn(const T& cond) {
		JR(!cond);
	}

	void CALL(const bool&);
	void RET();
	void RET(const bool&);
	void RETI();

	void JP(const bool&, const u16&);

	void RST(const u16&);

	// LD
	template <typename T, typename S>
	void LD(T& target, const S& source) {
		target = source;
	}

	void PUSH(const u16&);
	void POP(u16&);

	// 8bit arithmetic and logic
	void CPL();

	template <typename T>
	void XOR(const T& source) {
		m_data.a ^= source;
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = false;
		m_data.halfFlag = false;
		m_data.carryFlag = false;
	}

	template <typename T>
	void INC(T& target) {
		m_data.halfFlag = ((target & 0x0f) == 0x0f);
		target = static_cast<u8>(target + 1);
		m_data.zeroFlag = (target == 0);
		m_data.negFlag = false;
	}

	template <typename T>
	void DEC(T& target) {
		m_data.halfFlag = ((target & 0xf) == 0);
		target = static_cast<u8>(target - 1);
		m_data.zeroFlag = (target == 0);
		m_data.negFlag = true;
	}

	template <typename T>
	void CP(const T& source) {
		m_data.halfFlag = ((m_data.a & 0xf) < (source & 0xf));
		m_data.carryFlag = (m_data.a < source);
		m_data.zeroFlag = ((m_data.a - source) == 0);
		m_data.negFlag = true;
	}

	template <typename T>
	void SUB(const T& source) {
		m_data.halfFlag = ((m_data.a & 0xf) < (source & 0xf));
		m_data.carryFlag = (m_data.a < source);
		m_data.a = static_cast<u8>(m_data.a - source);
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = true;
	}

	template <typename T>
	void ADD(const T& source) {
		m_data.halfFlag = ((m_data.a & 0xf) + (source & 0xf) > 0xf);
		m_data.carryFlag = (m_data.a + source > 0xff);
		m_data.a = static_cast<u8>(m_data.a + source);
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = false;
	}

	template <typename T>
	void OR(const T& source) {
		m_data.a |= source;
		m_data.zeroFlag = (m_data.a == 0);
		m_data.carryFlag = false;
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void AND(const T& source) {
		m_data.a &= source;
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = false;
		m_data.halfFlag = true;
		m_data.carryFlag = false;
	}

	// 16bit arithmetic

	// annoying: can't specialize in class scope
	void INC16(u16&);
	void DEC16(u16&);

	void ADD16(const u16&);

	void ADDmixed(u16&, u16, u8);

	// extended instruction set
	void CB();

	template <typename T>
	void RLC(T& target) {
		m_data.carryFlag = ((target & 0b10000000) != 0);
		target = static_cast<u8>(target << 1 | target >> 7);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void RRC(T& target) {
		m_data.carryFlag = ((target & 1) != 0);
		target = static_cast<u8>(target >> 1 | target << 7);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void RL(T& target) {
		bool temp = m_data.carryFlag;
		m_data.carryFlag = ((target & 0b10000000) != 0);
		target = static_cast<u8>(target << 1 | temp);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void RR(T& target) {
		bool temp = m_data.carryFlag;
		m_data.carryFlag = ((target & 1) != 0);
		target = static_cast<u8>(target >> 1 | temp << 7);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void SLA(T& target) {
		m_data.carryFlag = ((target & 0b10000000) != 0);
		target = static_cast<u8>(target << 1);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void SRA(T& target) {
		m_data.carryFlag = ((target & 1) != 0);
		target = static_cast<u8>(target >> 1 | (target & 0b10000000));
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void SWAP(T& target) {
		target = static_cast<u8>(target >> 4 | target << 4);
		m_data.zeroFlag = (target == 0);
		m_data.carryFlag = false;
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void SRL(T& target) {
		m_data.carryFlag = ((target & 1) != 0);
		target = static_cast<u8>(target >> 1);
		m_data.zeroFlag = (target == 0);
		m_data.halfFlag = false;
		m_data.negFlag = false;
	}

	template <typename T>
	void BIT(const T& source) {
		m_data.zeroFlag = !source;
		m_data.negFlag = false;
		m_data.halfFlag = true;
	}

	template <typename T>
	void RES(T&& target) {
		target = false;
	}

	template <typename T>
	void SET(T&& target) {
		target = true;
	}
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&);
