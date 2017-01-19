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

private:
	ICPU::Data& m_data;
	IMMU* m_mmu;

	template <typename T>
	void LD(T& target, const T& source) {
		target = source;
	}
	template <typename T>
	void XOR(const T& source) {
		m_data.a ^= source;
		m_data.zeroFlag = (m_data.a == 0);
		m_data.negFlag = false;
		m_data.halfFlag = false;
		m_data.carryFlag = false;
	}
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&, IMMU*);
