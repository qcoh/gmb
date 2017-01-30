#include <stdexcept>

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
			m_data.attributes[addr >> 2][addr & 0x3] = v;
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
	if (m_data.bgDisplay) {
		renderTiles();
	}
}

void GPU::renderTiles() {
	// References:
	// http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	// http://bgb.bircd.org/pandocs.htm
	// http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Graphics

	// TODO: window

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

		// The GameBoy has two tile maps: 0x9800 - 0x9bff and 0x9c00 -
		// 0x9fff, which are selected by

		u16 tileMapAddr =
		    m_data.bgTileMapDisplaySelect ? 0x9c00 : 0x9800;

		// The leftmost visible tile on the current scanline is then
		// indexed by

		u16 currentTileIndex = read8(tileMapAddr + tileX + tileY * 32);

		// Similarly, the tiles themselves also consist of two
		// (intersecting!) blocks: 0x8000 - 0x0x8fff and 0x8800 -
		// 0x97ff, which are selected by m_data.tileDataSelect. If the
		// latter is is true, the current tile is vram[16 *
		// currentTileIndex .. 16 * currentTileIndex + 15], respectively

		IGPU::Data::Tile currentTile = m_data.tiles[currentTileIndex];

		// If m_data.tileDataSelect is false, the indexing starts at
		// 0x9000 BUT currentTileIndex is a SIGNED byte:

		if (m_data.tileDataSelect == false) {
			currentTileIndex = static_cast<u16>(
			    256 + static_cast<int8_t>(currentTileIndex));
			currentTile = m_data.tiles[currentTileIndex];
		}

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

		u32 color = 0;
		switch ((m_data.bgp >> (colorIndex << 1)) & 0x3) {
		case 0b00:
			color = 0xffffffff;
			break;
		case 0b01:
			color = 0xffc0c0c0;
			break;
		case 0b10:
			color = 0xff606060;
			break;
		default:
			color = 0xff000000;
			break;
		}

		// Finally, we draw the pixel.
		m_data.pixelArray[i + 160 * m_data.lY] = color;
	}
}
