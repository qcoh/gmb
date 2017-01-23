#include <SDL2/SDL.h>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "debugboy.h"
#include "defer.h"
#include "romonly.h"

int main() {
	const char* rom = "tetris.gb";
	const char* bios = "bios.bin";
	try {
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			throw std::runtime_error{SDL_GetError()};
		}
		defer([]() { SDL_Quit(); });
		RomOnly ro{"tetris.gb"};
		(void)ro;
		DebugBoy db{rom, bios};
		db.Run();
	} catch (std::exception& e) {
		std::cout << e.what() << '\n';
	}
}
