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
	}
}
