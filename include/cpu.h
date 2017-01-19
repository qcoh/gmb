#pragma once

#include "icpu.h"
#include "immu.h"
#include "instruction.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&, IMMU*);
	virtual ~CPU() = default;

	virtual void Step() override;

	static const std::array<Instruction, 256> s_instructions;

private:
	ICPU::Data& m_data;
	IMMU* m_mmu;
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&, IMMU*);
