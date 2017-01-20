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
	void BIT(const T& source) {
		m_data.zeroFlag = !source;
		m_data.negFlag = false;
		m_data.halfFlag = true;
	}
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&, IMMU*);
