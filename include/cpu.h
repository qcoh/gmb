#pragma once

#include "icpu.h"
#include "immu.h"
#include "instruction.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&, IMMU*);
	virtual ~CPU() = default;

	virtual void Step() override;

	void fetch();
	void exec();

	static const std::array<Instruction, 256> s_instructions;
	static const std::array<Instruction, 256> s_extended;

private:
	ICPU::Data& m_data;
	IMMU* m_mmu;

	// LD
	template <typename T>
	void LD(T& target, const T& source) {
		target = source;
	}

	template <typename T, typename S>
	void LDD(T&& target, const S& source) {
		target = source;
		m_data.hl--;
	}

	template <typename T, typename S>
	void LDI(T&& target, const S& source) {
		target = source;
		m_data.hl++;
	}

	// 8bit arithmetic and logic
	template <typename T>
	void XOR(const T& source) {
		m_data.a ^= source;
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = false;
		m_data.halfFlag = false;
		m_data.carryFlag = false;
	}

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

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&, IMMU*);
