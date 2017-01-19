#pragma once

#include "icpu.h"
#include "instruction.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&);
	virtual ~CPU() = default;

	virtual void Step() override;

private:
	ICPU::Data& m_data;
	static const std::array<Instruction, 256> s_instructions;
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&);
