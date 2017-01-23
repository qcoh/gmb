#include "gpu.h"
#include "catch.hpp"

SCENARIO("Verify read and write locations (GPU)", "[GPU]") {
	GIVEN("GPU, IGPU::Data") {
		IGPU::Data data{};
		GPU gpu{data};

		WHEN("Writing to registers") {
			for (u16 i = 0xff40; i < 0xff4c; i++) {
				if (i == 0xff46 || i == 0xff44) {
					continue;
				}
				gpu.write8(i, i & 0x1f);
			}
			THEN("value at 0xff4n == n") {
				REQUIRE(data.lcdControl == 0);
				// lower 3 bits untouched
				REQUIRE(data.lcdStat == 0);
				REQUIRE(data.scrollY == 2);
				REQUIRE(data.scrollX == 3);
				REQUIRE(data.lYC == 5);
				REQUIRE(data.bgp == 7);
				REQUIRE(data.obp0 == 8);
				REQUIRE(data.obp1 == 9);
				REQUIRE(data.wY == 10);
				REQUIRE(data.wX == 11);
			}
		}
		WHEN("Writing to LCD Stat") {
			gpu.write8(0xff41, 0xff);

			THEN("lcdStat == 0b11111000") {
				REQUIRE(data.lcdStat == 0b11111000);
			}
		}
		WHEN("Writing to OAM") {
			gpu.write8(0xfe00, 0x12);
			gpu.write8(0xfe50, 0x34);
			gpu.write8(0xfe9f, 0x56);

			THEN(
			    "oam[0] == 0x12, oam[0x50] == 0x34, oam[0x9f] == "
			    "0x56") {
				REQUIRE(data.oam[0] == 0x12);
				REQUIRE(data.oam[0x50] == 0x34);
				REQUIRE(data.oam[0x9f] == 0x56);
			}
		}
	}
}
