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
		WHEN("Writing to wram*") {
			mmu.write8(0xc000, 0x12);
			mmu.write8(0xcfff, 0x34);

			mmu.write8(0xd000, 0x78);
			mmu.write8(0xdfff, 0x9a);

			THEN(
			    "wram0[0] == 0x12, wram0[0xfff] == 0x34, wram1[0] "
			    "== 0x78, wram1[0xfff] == 0x9a") {
				REQUIRE(data.wram0[0] == 0x12);
				REQUIRE(data.wram0[0xfff] == 0x34);
				REQUIRE(data.wram1[0] == 0x78);
				REQUIRE(data.wram1[0xfff] == 0x9a);
			}
		}
	}
}
