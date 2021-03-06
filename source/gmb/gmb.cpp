#include <SDL2/SDL.h>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "debugboy.h"
#include "defer.h"

int main(int argc, char* argv[]) {
	(void)argc;
	const char* bios = "bios.bin";
	try {
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			throw std::runtime_error{SDL_GetError()};
		}
		defer([]() { SDL_Quit(); });
		DebugBoy db{argv[1], bios};
		db.run();
	} catch (std::exception& e) {
		std::cout << e.what() << '\n';
	}
}
