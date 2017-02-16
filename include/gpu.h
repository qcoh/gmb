#pragma once

#include "display.h"
#include "interruptdata.h"
#include "types.h"

class GPU {
public:
	GPU(Display*, InterruptData*);
	virtual ~GPU() = default;

	u8 read8(u16) const;
	void write8(u16, u8);
	void step(u16);

protected:
	static const u8 ACCESSING_OAM = 0b10;
	static const u8 ACCESSING_VRAM = 0b11;
	static const u8 HBLANK = 0b00;
	static const u8 VBLANK = 0b01;

	void renderScanline();
	void renderTiles();
	void renderWindow();
	void renderSprites();

	static u32 colorSelect(u8, u8);

	u16 m_cycles = 0;
	std::array<u8, 8192> m_vram = {{0}};

	using Row = std::array<u8, 2>;
	using Tile = std::array<Row, 8>;
	std::array<Tile, 384> m_tiles;

	std::array<u8, 160> m_oam = {{0}};

	using Attribute = std::array<u8, 4>;
	std::array<Attribute, 40> m_attributes;

	Display* m_display = nullptr;
	Display::PixelArray m_pixelArray = {{0}};

	// 0xff40 LCD Control
	u8 m_lcdControl = 0;
	// Display Enable (0=Off, 1=On)
	BitRef<u8> m_displayEnable{m_lcdControl, 7};
	// Window Tile Map Display Select (0=0x9800-0x9bff,
	// 1=0x9c00-0x9fff)
	BitRef<u8> m_windowTileMapDisplaySelect{m_lcdControl, 6};
	// Window Display Enable (0=Off, 1=On)
	BitRef<u8> m_windowDisplayEnable{m_lcdControl, 5};
	// BG & Window Tile Data Select (0=0x8800-0x97ff,
	// 1=0x8000-0x8fff)
	BitRef<u8> m_tileDataSelect{m_lcdControl, 4};
	// BG Tile Map Display Select (0=0x9800-0x9bff, 1=0x9c00-9xfff)
	BitRef<u8> m_bgTileMapDisplaySelect{m_lcdControl, 3};
	// Object Size (0=8x8, 1=8x16)
	BitRef<u8> m_objectSize{m_lcdControl, 2};
	// Object Display Enable (0=Off, 1=On)
	BitRef<u8> m_objectDisplayEnable{m_lcdControl, 1};
	// BG Display (0=Off, 1=On) -- GBC
	BitRef<u8> m_bgDisplayEnable{m_lcdControl, 0};

	// 0xff41 LCD Stat
	u8 m_lcdStat = 0;
	// LYC == LY Coincidence interrupt (1=Enable, R/W)
	BitRef<u8> m_coincidenceInterrupt{m_lcdStat, 6};
	// Mode 2 OAM Interrupt (1=Enable, R/W)
	BitRef<u8> m_oamInterrupt{m_lcdStat, 5};
	// Mode 1 V-Blank Interrupt (1=Enable, R/W)
	BitRef<u8> m_vBlankInterrupt{m_lcdStat, 4};
	// Mode 0 H-Blank Interrupt (1=Enablem, R/W)
	BitRef<u8> m_hBlankInterrupt{m_lcdStat, 3};
	// Coincidence Flag (1 if LY == LYC, 0 if LY != LYC, R)
	BitRef<u8> m_coincidenceFlag{m_lcdStat, 2};

	// 0xff42 Scroll Y
	u8 m_scrollY = 0;
	// 0xff43 Scroll X
	u8 m_scrollX = 0;

	// 0xff44 LY (read only)
	u8 m_lY = 0;
	// 0xff45 LYC
	u8 m_lYC = 0;

	// 0xff47 BG Palette
	u8 m_bgp = 0;
	// 0xff48 Object Palette 0 Data
	u8 m_obp0 = 0;
	// 0xff49 Object Palette 1 Data
	u8 m_obp1 = 0;

	// 0xff4a Window Y position
	u8 m_wY = 0;
	// 0xff4b Window X position
	u8 m_wX = 0;

	InterruptData* m_intData = nullptr;
};
