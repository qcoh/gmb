#pragma once

#include <memory>
#include "types.h"

class ICPU {
public:
	struct Data {
		u16 af = 0;
		u16 bc = 0;
		u16 de = 0;
		u16 hl = 0;
		u16 nn = 0;

		u8& a = {*(static_cast<u8*>(static_cast<void*>(&af)) + 1)};
		u8& f = {*(static_cast<u8*>(static_cast<void*>(&af)))};
		u8& b = {*(static_cast<u8*>(static_cast<void*>(&bc)) + 1)};
		u8& c = {*(static_cast<u8*>(static_cast<void*>(&bc)))};
		u8& d = {*(static_cast<u8*>(static_cast<void*>(&de)) + 1)};
		u8& e = {*(static_cast<u8*>(static_cast<void*>(&de)))};
		u8& h = {*(static_cast<u8*>(static_cast<void*>(&hl)) + 1)};
		u8& l = {*(static_cast<u8*>(static_cast<void*>(&hl)))};

		u8& n = {*(static_cast<u8*>(static_cast<void*>(&nn)))};
	};

	ICPU() = default;
	ICPU(const ICPU&) = delete;
	ICPU& operator=(const ICPU&) = delete;
	ICPU(ICPU&&) = delete;
	ICPU& operator=(ICPU&&) = delete;
	virtual ~ICPU() = default;

	virtual void Step() = 0;
};
