#pragma once

#include <string>

#include "types.h"

struct Instruction {
	u8 code = 0;
	std::string mnemonic;
	u8 cycles = 0;
	u8 offset = 0;
};
