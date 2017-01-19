#include "memref.h"
#include "bitref.h"
#include "catch.hpp"
#include "testmmu.h"

SCENARIO("Testing MemRef", "[MemRef]") {
	GIVEN("BitRef to MemRef") {
		std::array<u8, 0x10000> arr = {{0}};
		TestMMU mmu{arr};

		u16 addr = 0x100;
		MemRef mr{addr, &mmu};

		BitRef<MemRef> br{mr, 7};

		WHEN("mr = 0x99") {
			mr = 0x99;

			THEN("arr[0x100] == 0x99") {
				REQUIRE(arr[0x100] == 0x99);
			}
		}
		WHEN("br = true") {
			br = true;

			THEN("arr[0x100] == 0b10000000") {
				REQUIRE(arr[0x100] == 0b10000000);
			}
		}
	}
}
