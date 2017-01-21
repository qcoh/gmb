#include "mmu.h"
#include "catch.hpp"

SCENARIO("Verify read and write locations", "[MMU]") {
	GIVEN("MMU, IMMU::Data") {
		IMMU::Data data{};
		MMU mmu{data};

		WHEN("Writing to hram") {
			u8 v = 0;
			for (u16 i = 0xff80; i < 0xffff; i++) {
				mmu.write8(i, v);
				v++;
			}

			THEN("hram[w] == w") {
				for (u8 w = 0; w < data.hram.size(); w++) {
					REQUIRE(data.hram[w] == w);
				}
			}
		}
	}
}
