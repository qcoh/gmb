#pragma once

#include "gpu.h"

class TestGPU : public GPU {
public:
	TestGPU();
	virtual ~TestGPU() = default;

	u8 lcdControl();
	u8 lcdStat();
	u8 scrollX();
	u8 scrollY();
	u8 lYC();
	u8 bgp();
	u8 obp0();
	u8 obp1();
	u8 wX();
	u8 wY();

	const std::array<u8, 160>& oam();
};
