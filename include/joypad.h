#pragma once

#include "bitref.h"

class Joypad {
public:
	Joypad() = default;
	~Joypad() = default;

	Joypad(const Joypad&) = delete;
	Joypad& operator=(const Joypad&) = delete;
	Joypad(Joypad&&) = delete;
	Joypad& operator=(Joypad&&) = delete;

	void write(u8);
	u8 read();

private:
	enum class Mode {
		BUTTON,
		DIRECTION,
	};
	Mode m_mode = Mode::BUTTON;

	u8 m_buttonKeys = 0x0f;
	BitRef<u8> m_buttonStart{m_buttonKeys, 3};
	BitRef<u8> m_buttonSelect{m_buttonKeys, 2};
	BitRef<u8> m_buttonB{m_buttonKeys, 1};
	BitRef<u8> m_buttonA{m_buttonKeys, 0};

	u8 m_directionKeys = 0x0f;
	BitRef<u8> m_directionDown{m_directionKeys, 3};
	BitRef<u8> m_directionUp{m_directionKeys, 2};
	BitRef<u8> m_directionLeft{m_directionKeys, 1};
	BitRef<u8> m_directionRight{m_directionKeys, 0};
};
