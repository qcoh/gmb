#include <algorithm>
#include <stdexcept>
#include <vector>

#include "gpu.h"

GPU::GPU(IGPU::Data& data) : m_data{data} {}

u8 GPU::read8(u16 addr) const {
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		return m_data.vram[addr & 0x1fff];
	case 0xf000:
		switch (addr & 0x0f00) {
		case 0x0e00:
			return m_data.oam[addr & 0xff];
		case 0x0f00:
			switch (addr) {
			case 0xff40:
				return m_data.lcdControl;
			case 0xff41:
				return m_data.lcdStat;
			case 0xff42:
				return m_data.scrollY;
			case 0xff43:
				return m_data.scrollX;
			case 0xff44:
				return m_data.lY;
			case 0xff45:
				return m_data.lYC;
			case 0xff46:
				// https://github.com/Gekkio/mooneye-gb/blob/master/docs/accuracy.markdown
				return 0xff;
			case 0xff47:
				return m_data.bgp;
			case 0xff48:
				return m_data.obp0;
			case 0xff49:
				return m_data.obp1;
			case 0xff4a:
				return m_data.wY;
			case 0xff4b:
				return m_data.wX;
			case 0xff4c:
			case 0xff4d:
			case 0xff4e:
			case 0xff4f:
				// unknown or CGB, ignore for now
				return 0xff;
			default:
				break;
			};
		default:
			break;
		};
	default:
		break;
	}
	std::cout << "Read: 0x" << std::hex << +addr << '\n';
	throw std::runtime_error{"GPU Address not implemented"};
}

void GPU::write8(u16 addr, u8 v) {
	m_data.vram[addr & 0x1fff] = v;

	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		m_data.vram[addr & 0x1fff] = v;
		if (addr < 0x9800) {
			// update tiles
			m_data.tiles[(addr & 0x1fff) >> 4][(addr >> 1) & 0x7]
				    [addr & 1] = v;
		}
		return;
	case 0xf000:
		switch (addr & 0x0f00) {
		case 0x0e00:
			m_data.oam[addr & 0xff] = v;
			m_data.attributes[(addr & 0xff) >> 2][addr & 0x3] = v;
			return;
		case 0x0f00:
			switch (addr) {
			case 0xff40:
				m_data.lcdControl = v;
			case 0xff41:
				// bit 0, 1, 2 are read only
				m_data.lcdStat =
				    (m_data.lcdStat & 0b111) | (v & 0b11111000);
				return;
			case 0xff42:
				m_data.scrollY = v;
				return;
			case 0xff43:
				m_data.scrollX = v;
				return;
			case 0xff44:
				// read only
				return;
			case 0xff45:
				m_data.lYC = v;
				return;
			case 0xff46:
				// DMA
				break;
			case 0xff47:
				m_data.bgp = v;
				return;
			case 0xff48:
				m_data.obp0 = v;
				return;
			case 0xff49:
				m_data.obp1 = v;
				return;
			case 0xff4a:
				m_data.wY = v;
				return;
			case 0xff4b:
				m_data.wX = v;
				return;
			case 0xff4c:
			case 0xff4d:
			case 0xff4e:
			case 0xff4f:
				// unknown or CGB, ignore for now
				return;
			default:
				break;
			};
		default:
			break;
		};
	default:
		break;
	}
	std::cout << "Write: 0x" << std::hex << +addr << '\n';
	throw std::runtime_error{"GPU Address not implemented"};
}

void GPU::step(u16 cycles) {
	// http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
	m_data.cycles += cycles;

	switch (m_data.lcdStat & 0b11) {
	case ACCESSING_OAM:
		if (m_data.cycles >= 80) {
			m_data.cycles = 0;
			m_data.lcdStat =
			    (m_data.lcdStat & 0b11111100) | ACCESSING_VRAM;
		}
		break;
	case ACCESSING_VRAM:
		if (m_data.cycles >= 172) {
			m_data.cycles = 0;
			m_data.lcdStat = (m_data.lcdStat & 0b11111100) | HBLANK;
			renderScanline();
		}
		break;
	case HBLANK:
		if (m_data.cycles >= 204) {
			m_data.cycles = 0;
			m_data.lY++;

			if (m_data.lY == 144) {
				m_data.lcdStat =
				    (m_data.lcdStat & 0b11111100) | VBLANK;
				m_data.intData->vBlankFlag = true;
				m_data.display->render(m_data.pixelArray);
			} else {
				m_data.lcdStat = (m_data.lcdStat & 0b11111100) |
						 ACCESSING_OAM;
			}
		}
		break;
	case VBLANK:
		if (m_data.cycles >= 456) {
			m_data.cycles = 0;
			m_data.lY++;

			if (m_data.lY > 153) {
				m_data.lcdStat = (m_data.lcdStat & 0b11111100) |
						 ACCESSING_OAM;
				m_data.lY = 0;
			}
		}
		break;
	}
}

void GPU::renderScanline() {
	if (m_data.bgDisplayEnable) {
		renderTiles();
	}

	if (m_data.windowDisplayEnable) {
		renderWindow();
	}

	if (m_data.objectDisplayEnable) {
		renderSprites();
	}
}

void GPU::renderTiles() {
	// References:
	// http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	// http://bgb.bircd.org/pandocs.htm
	// http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Graphics

	// The GameBoy has two tile maps: 0x9800 - 0x9bff and 0x9c00 -
	// 0x9fff, which are selected by

	u16 tileMapAddr = m_data.bgTileMapDisplaySelect ? 0x9c00 : 0x9800;

	for (u8 i = 0; i < 160; i++) {
		// The GameBoy renders a 160*144 slice of the 256*256
		// background, which consists of 32 * 32 tiles.
		//
		//	+---------------------------------------+
		//	|					|
		//	|					|
		//	|	+---------------+		|
		//	|	|		|		|
		//	|	|		|		|
		//	|	|		|		|
		//	|	+---------------+		|
		//	|					|
		//	|					|
		//	+---------------------------------------+
		//
		// The upper left corner of the inner rectangle is at
		// (m_data.scrollX, m_data.scrollY), the current row is at
		// m_data.scrollY + m_data.lY and the current pixel is
		// P = (m_data.scrollX + i, m_data.scrollY + m_data.lY).
		//
		// Since we only render a line at a time, we need to find the
		// leftmost tile at P:
		// (Note: We divide by 16 since a tile is 16 bytes)

		u8 tileX = static_cast<u8>(m_data.scrollX + i) >> 3;
		u8 tileY = static_cast<u8>(m_data.scrollY + m_data.lY) >> 3;

		// The leftmost visible tile on the current scanline is then
		// indexed by

		u16 currentTileIndex = read8(tileMapAddr + tileX + tileY * 32);

		// If m_data.tileDataSelect is false, the indexing starts at
		// 0x9000 BUT currentTileIndex is a SIGNED byte:
		if (m_data.tileDataSelect == false) {
			currentTileIndex = static_cast<u16>(
			    256 + static_cast<int8_t>(currentTileIndex));
		}

		IGPU::Data::Tile currentTile = m_data.tiles[currentTileIndex];

		// Before we render the scanline, we need to find the row inside
		// the tiles to render and the starting column:

		u8 pixelOffsetX = static_cast<u8>(m_data.scrollX + i) & 0x7;
		u8 pixelOffsetY =
		    static_cast<u8>((m_data.scrollY + m_data.lY) & 0x7);

		IGPU::Data::Row currentRow = currentTile[pixelOffsetY];

		// A Row consists of two bytes l and h, where the j-th pixel
		// color index (for j in 0, ..., 7) is determined by the
		// (7-j)-th bit of l and h:

		int mask = (7 - (pixelOffsetX & 0x7));
		u8 colorIndex =
		    static_cast<u8>(((currentRow[0] >> mask) & 0x1) |
				    (((currentRow[1] >> mask) & 0x1) << 1));

		// The actual color is determined using the palette m_data.bgp.
		// The eight bits of m_data.bgp are grouped into four two-bit
		// values, which give the colors:
		//
		// 	0b00 == 0xffffffff (white)
		// 	0b01 == 0xffc0c0c0 (light gray)
		// 	0b10 == 0xff606060 (dark gray)
		// 	0b11 == 0xff000000 (black)
		//
		// Since colorIndex is 0, 1, 2 or 3, we select the
		// corresponding color via:

		u32 color = colorSelect(m_data.bgp, colorIndex);

		// Finally, we draw the pixel.
		m_data.pixelArray[i + 160 * m_data.lY] = color;
	}
}

void GPU::renderWindow() {
	// References:
	// http://bgb.bircd.org/pandocs.htm#videodisplay

	// The window is an optional overlay over the background (tiles). It
	// shares the tile map location(s) and the tile data location(s) with
	// the ordinary background.
	//
	// The tile map for the window is selected via:

	const u16 tileMapAddr =
	    (m_data.windowTileMapDisplaySelect) ? 0x9c00 : 0x9800;

	// The window's location (top left corner) is given by (wX-7, wY),
	// so let's check if the window is intersecting the current scanline:

	if (m_data.wY > m_data.lY) {
		return;
	}
	// According to
	// https://www.reddit.com/r/EmuDev/comments/5qg6ek/gb_wrong_tile_set_displaying_in_games_like_volley/dczddcz/
	// if wX is less than 7, it is set to 7.
	if (m_data.wX < 7) {
		m_data.wX = 7;
	}

	// No need to draw the window if it is not on the screen.
	if (/*m_data.wX < 7 ||*/ m_data.wX >= 166) {
		return;
	}

	for (u8 i = m_data.wX - 7; i < 160; i++) {
		// Rendering the window works almost like rendering the
		// background tiles
		// except that it is moved by (wX-7, wY) and not affected by
		// scrolling.
		//
		//	+---------------------------------------+
		//	|					|
		//	|		+------------------------ - - - +
		//	|		|				|
		//	|		|
		//	|		|				|
		//	|		|
		//	|		|				|
		//	+---------------|
		//			|				|
		//			+ - - - - - - - - - - - - - - - +
		//
		// As before, we need to find the index of the tile intersecting
		// the scanline:

		const u8 tileX = static_cast<u8>(i - m_data.wX + 7) >> 3;
		const u8 tileY = static_cast<u8>(m_data.lY - m_data.wY) >> 3;

		// The tile below (i, lY) is then:

		u16 currentTileIndex = read8(tileMapAddr + tileX + tileY * 32);

		// As before, we may need to adjust currentTileIndex:

		if (m_data.tileDataSelect == false) {
			currentTileIndex = static_cast<u16>(
			    256 + static_cast<int8_t>(currentTileIndex));
		}

		IGPU::Data::Tile currentTile = m_data.tiles[currentTileIndex];

		// The row and column inside the tile:
		u8 pixelOffsetX = static_cast<u8>(i - m_data.wX + 7) & 0x7;
		u8 pixelOffsetY =
		    static_cast<u8>((m_data.lY - m_data.wY) & 0x7);

		IGPU::Data::Row currentRow = currentTile[pixelOffsetY];

		// We compute the color index and color and draw the pixel:
		int mask = (7 - (pixelOffsetX & 0x7));
		u8 colorIndex =
		    static_cast<u8>(((currentRow[0] >> mask) & 0x1) |
				    (((currentRow[1] >> mask) & 0x1) << 1));

		u32 color = colorSelect(m_data.bgp, colorIndex);

		m_data.pixelArray[i + 160 * m_data.lY] = color;
	}
}

void GPU::renderSprites() {
	// References:
	// http://bgb.bircd.org/pandocs.htm#videodisplay
	// http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	// http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Sprites

	// We now wish to render sprites, which consist of either 8*8 or
	// 8*16pixels.

	const u8 spriteHeight = m_data.objectSize ? 16 : 8;

	// Unlike tiles, their position is not fixed to the 32*32
	// background, but they can exist anywhere on the display (this also
	// means that they are unaffected by scrollX and scrollY).
	//
	// The position of a sprite is given by its attribute data, i.e.
	//
	//   u8 Y = y position *minus 16*. Y < 16 or Y >= 160 hides the sprite.
	//
	//   u8 X = x position *minus 8*. X < 8 or X >= 168 hides the sprite.
	//
	//   u8 n = selects the nth tile (in 0x8000-0x8fff). In 8*16 mode, n
	//   selects both the (n & 0xfe)-th and (n | 0x1)-th tile.
	//
	//   u8 attributes:
	//
	//   	bit 7: Specifies the priority, i.e. if 0, the sprite is always
	//   	drawn, if 1, it is drawn only if the background color is
	//   	white.
	//
	//   	bit 6: Y Flip (1=vertically mirrored)
	//
	//   	bit 5: X Flip (1=horizontally mirrored)
	//
	//   	bit 4: Palette number (0=obp0, 1=obp1)
	//
	//   	bit 0-3: CGB, ignore
	//
	// Each sprite is determined by these four bytes and the corresponding
	// tile. These object attributes are stored in the object attribute
	// memory (0xfe00 - 0xfe9f), which means there are 40 sprites.
	//
	// Our first objective is to find all visible sprites intersecting the
	// scanline:

	std::vector<u8> sprites{};
	sprites.reserve(40);
	for (u8 i = 0; i < 40; i++) {
		const IGPU::Data::Attribute& curAttr = m_data.attributes[i];

		// sprite is not visible
		if (curAttr[0] < 16 || curAttr[0] >= 160 || curAttr[1] < 8 ||
		    curAttr[1] >= 168) {
			continue;
		}

		u8 ypos = static_cast<u8>(curAttr[0] - 16);

		// sprite is not on scanline
		if (!(ypos <= m_data.lY && m_data.lY < ypos + spriteHeight)) {
			continue;
		}

		sprites.push_back(i);
	}

	// The GameBoy can only render only 10 sprites per scanline, so we sort
	// `sprites` by the sprites' X positions and resize it.
	//
	// http://stackoverflow.com/a/32440415
	std::sort(
	    std::begin(sprites), std::end(sprites),
	    [& attr = static_cast<const std::array<IGPU::Data::Attribute, 40>&>(
		 m_data.attributes)](u8 a, u8 b) {
		    return attr[a][1] < attr[b][1];
	    });
	sprites.resize(std::min(10ul, sprites.size()));

	// Next, we determine the sprites' priority, i.e. the order of drawing
	// overlapping sprites, as follows:
	//
	// * If their X positions are different, the leftmost has priority.
	// * If their X positions coincide, the sprite with the smaller index
	//   has priority.
	//
	// We sort by ascending priority:
	std::sort(
	    std::begin(sprites), std::end(sprites),
	    [& attr = static_cast<const std::array<IGPU::Data::Attribute, 40>&>(
		 m_data.attributes)](u8 a, u8 b) {
		    if (attr[a][1] != attr[b][1]) {
			    return attr[a][1] > attr[b][1];
		    }
		    return a > b;
	    });

	// Now let's render some sprites!
	for (u8 i = 0; i < 160; i++) {
		for (auto s : sprites) {
			const IGPU::Data::Attribute& curAttr =
			    m_data.attributes[s];

			u8 ypos = curAttr[0] - 16;
			u8 xpos = curAttr[1] - 8;

			// Current position (i, lY) does not lie on sprite.
			if (!((xpos <= i) && (i < xpos + 8) &&
			      (ypos <= m_data.lY) &&
			      (m_data.lY < ypos + spriteHeight))) {
				continue;
			}

			// Next, we must find the pixel inside the sprite which
			// lies below (i, lY):
			u8 pixelOffsetX = static_cast<u8>(i - xpos);
			u8 pixelOffsetY = static_cast<u8>(m_data.lY - ypos);

			// If the respective flags are set, we mirror the pixel
			// positions:
			if ((curAttr[3] & (1 << 6)) != 0) {
				pixelOffsetY = static_cast<u8>(
				    spriteHeight - 1 - pixelOffsetY);
			}
			if ((curAttr[3] & (1 << 5)) == 0) {
				pixelOffsetX =
				    static_cast<u8>(7 - pixelOffsetX);
			}

			// We obtain the palette:
			u8 palette =
			    (curAttr[3] & (1 << 4)) ? m_data.obp1 : m_data.obp0;

			// If the sprite lies behind the background we only draw
			// if the background color is white:
			if ((curAttr[3] & (1 << 7)) &&
			    (m_data.pixelArray[i + 160 * m_data.lY] !=
			     0xffffffff)) {
				continue;
			}

			// If we're in the 8*8 sprite mode, we select the tile
			// pointed at by the tile location attribute:
			u8 tileIndex = curAttr[2];

			// If we're in the 8*16 sprite mode, the sprite is
			//
			// +------------+
			// |		|
			// | n & 0xfe	|
			// |		|
			// +------------+
			// |		|
			// | n | 0x01	|
			// |		|
			// +------------+
			//
			// So, if pixelOffsetY is larger than 7, we select
			// sprite n | 0x01, otherwise we select sprite n & 0xfe.

			if (m_data.objectSize) {
				tileIndex = (pixelOffsetY > 7)
						? tileIndex | 0x1
						: tileIndex & 0xfe;
			}

			const IGPU::Data::Tile& curTile =
			    m_data.tiles[tileIndex];
			const IGPU::Data::Row& curRow =
			    curTile[pixelOffsetY & 0x7];

			// Like in `renderTiles` we need to determine the color
			// index:
			u8 colorIndex = static_cast<u8>(
			    ((curRow[0] >> pixelOffsetX) & 0x1) |
			    (((curRow[1] >> pixelOffsetX) & 0x1) << 1));

			// Unlike tiles, if colorIndex == 0b00, the pixel is
			// transparent:
			if (colorIndex == 0b00) {
				continue;
			}

			m_data.pixelArray[i + 160 * m_data.lY] =
			    colorSelect(palette, colorIndex);
		}
	}
}

u32 GPU::colorSelect(u8 palette, u8 index) {
	switch ((palette >> (index << 1)) & 0x3) {
	case 0b00:
		return 0xffffffff;
	case 0b01:
		return 0xffc0c0c0;
	case 0b10:
		return 0xff606060;
		break;
	default:
		return 0xff000000;
	}
}
