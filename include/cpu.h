#pragma once

#include "icpu.h"
#include "immu.h"
#include "instruction.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&, IMMU*);
	virtual ~CPU() = default;

	virtual void Step() override;

private:
	ICPU::Data& m_data;
	IMMU* m_mmu;
	static const std::array<Instruction, 256> s_instructions;
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&, IMMU*);
