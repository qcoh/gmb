#include "catch.hpp"
#include "testgpu.h"

SCENARIO("Verify read and write locations (GPU)", "[GPU]") {
	GIVEN("GPU, IGPU::Data") {
		TestGPU gpu{};

		WHEN("Writing to registers") {
			for (u16 i = 0xff40; i < 0xff4c; i++) {
				if (i == 0xff46 || i == 0xff44) {
					continue;
				}
				gpu.write8(i, i & 0x1f);
			}
			THEN("value at 0xff4n == n") {
				REQUIRE(gpu.lcdControl() == 0);
				// lower 3 bits untouched
				REQUIRE(gpu.lcdStat() == 0);
				REQUIRE(gpu.scrollY() == 2);
				REQUIRE(gpu.scrollX() == 3);
				REQUIRE(gpu.lYC() == 5);
				REQUIRE(gpu.bgp() == 7);
				REQUIRE(gpu.obp0() == 8);
				REQUIRE(gpu.obp1() == 9);
				REQUIRE(gpu.wY() == 10);
				REQUIRE(gpu.wX() == 11);
			}
		}
		WHEN("Writing to LCD Stat") {
			gpu.write8(0xff41, 0xff);

			THEN("lcdStat == 0b11111000") {
				REQUIRE(gpu.lcdStat() == 0b11111000);
			}
		}
		WHEN("Writing to OAM") {
			gpu.write8(0xfe00, 0x12);
			gpu.write8(0xfe50, 0x34);
			gpu.write8(0xfe9f, 0x56);

			THEN(
			    "oam[0] == 0x12, oam[0x50] == 0x34, oam[0x9f] == "
			    "0x56") {
				REQUIRE(gpu.oam()[0] == 0x12);
				REQUIRE(gpu.oam()[0x50] == 0x34);
				REQUIRE(gpu.oam()[0x9f] == 0x56);
			}
		}
	}
}
