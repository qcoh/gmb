#pragma once

#include "icpu.h"

class CPU : public ICPU {
public:
	CPU(ICPU::Data&);
	virtual ~CPU() = default;

	virtual void Step() override;

private:
	ICPU::Data& m_data;
};

extern "C" std::unique_ptr<ICPU> loadCPU(ICPU::Data&);
