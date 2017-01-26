#pragma once

#include <unordered_set>

#include "mmu.h"

class DebugMMU : public MMU {
public:
	DebugMMU(IMMU::Data&);
	virtual ~DebugMMU() = default;

	virtual void write8(u16, u8) override;

	void watch(u16);
	void clear();
	bool& watchMode();

	struct WatchEvent {
		u16 addr;
		u8 oldVal;
		u8 newVal;
	};

private:
	std::unordered_set<u16> m_watchPoints{};

	bool m_watchMode = false;
};
