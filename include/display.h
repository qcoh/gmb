#pragma once

#include <SDL2/SDL.h>
#include <array>

#include "types.h"

class Display {
public:
	static const u32 WIDTH = 160;
	static const u32 HEIGHT = 144;
	static const u32 PITCH = 4 * WIDTH;
	static const char* NAME;
	using PixelArray = std::array<u32, WIDTH * HEIGHT>;

	Display();
	void render(const PixelArray&);
	~Display();

private:
	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;
};
