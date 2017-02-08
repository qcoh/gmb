#include <SDL2/SDL.h>

#include "joypad.h"

void Joypad::write(u8 v) {
	if ((v & 0b00100000) == 0) {
		m_mode = Mode::BUTTON;
	} else if ((v & 0b00010000) == 0) {
		m_mode = Mode::DIRECTION;
	}
}

u8 Joypad::read() {
	m_buttonKeys = 0xf;
	m_directionKeys = 0xf;
	const Uint8* state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_DOWN]) {
		m_directionDown = false;
	}
	if (state[SDL_SCANCODE_UP]) {
		m_directionUp = false;
	}
	if (state[SDL_SCANCODE_LEFT]) {
		m_directionLeft = false;
	}
	if (state[SDL_SCANCODE_RIGHT]) {
		m_directionRight = false;
	}
	if (state[SDL_SCANCODE_A]) {
		m_buttonStart = false;
	}
	if (state[SDL_SCANCODE_S]) {
		m_buttonSelect = false;
	}
	if (state[SDL_SCANCODE_Y]) {
		m_buttonB = false;
	}
	if (state[SDL_SCANCODE_X]) {
		m_buttonA = false;
	}

	return m_mode == Mode::BUTTON ? m_buttonKeys : m_directionKeys;
}
