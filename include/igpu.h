#pragma once

#include <array>

#include "bitref.h"
#include "display.h"
#include "interruptdata.h"
#include "types.h"

class IGPU {
public:
	struct Data {
		u16 cycles = 0;
		std::array<u8, 8192> vram = {{0}};

		using Row = std::array<u8, 2>;
		using Tile = std::array<Row, 8>;
		std::array<Tile, 384> tiles;

		Display* display = nullptr;
		Display::PixelArray pixelArray = {{0}};

		// 0xff40 LCD Control
		u8 lcdControl = 0;
		// Display Enable (0=Off, 1=On)
		BitRef<u8> displayEnable{lcdControl, 7};
		// Window Tile Map Display Select (0=0x9800-0x9bff,
		// 1=0x9c00-0x9fff)
		BitRef<u8> windowTileMapDisplaySelect{lcdControl, 6};
		// Window Display Enable (0=Off, 1=On)
		BitRef<u8> windowDisplayEnable{lcdControl, 5};
		// BG & Window Tile Data Select (0=0x8800-0x97ff,
		// 1=0x8000-0x8fff)
		BitRef<u8> tileDataSelect{lcdControl, 4};
		// BG Tile Map Display Select (0=0x9800-0x9bff, 1=0x9c00-9xfff)
		BitRef<u8> bgTileMapDisplaySelect{lcdControl, 3};
		// Object Size (0=8x8, 1=8x16)
		BitRef<u8> objectSize{lcdControl, 2};
		// Object Display Enable (0=Off, 1=On)
		BitRef<u8> objectDisplayEnable{lcdControl, 1};
		// BG Display (0=Off, 1=On) -- GBC
		BitRef<u8> bgDisplay{lcdControl, 0};

		// 0xff41 LCD Stat
		u8 lcdStat = 0;
		// LYC == LY Coincidence interrupt (1=Enable, R/W)
		BitRef<u8> coincidenceInterrupt{lcdStat, 6};
		// Mode 2 OAM Interrupt (1=Enable, R/W)
		BitRef<u8> oamInterrupt{lcdStat, 5};
		// Mode 1 V-Blank Interrupt (1=Enable, R/W)
		BitRef<u8> vBlankInterrupt{lcdStat, 4};
		// Mode 0 H-Blank Interrupt (1=Enablem, R/W)
		BitRef<u8> hBlankInterrupt{lcdStat, 3};
		// Coincidence Flag (1 if LY == LYC, 0 if LY != LYC, R)
		BitRef<u8> coincidenceFlag{lcdStat, 2};

		// 0xff42 Scroll Y
		u8 scrollY = 0;
		// 0xff43 Scroll X
		u8 scrollX = 0;

		// 0xff44 LY (read only)
		u8 lY = 0;
		// 0xff45 LYC
		u8 lYC = 0;

		// 0xff47 BG Palette
		u8 bgp = 0;
		// 0xff48 Object Palette 0 Data
		u8 obp0 = 0;
		// 0xff49 Object Palette 1 Data
		u8 obp1 = 0;

		// 0xff4a Window Y position
		u8 wY = 0;
		// 0xff4b Window X position
		u8 wX = 0;

		InterruptData* intData = nullptr;
	};
	IGPU() = default;
	IGPU(const IGPU&) = delete;
	IGPU& operator=(const IGPU&) = delete;
	IGPU(IGPU&&) = delete;
	IGPU& operator=(IGPU&&) = delete;
	virtual ~IGPU() = default;

	virtual u8 read8(u16) const = 0;
	virtual void write8(u16, u8) = 0;
	virtual void step(u16) = 0;
};
