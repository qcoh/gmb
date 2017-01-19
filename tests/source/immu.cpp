#include "catch.hpp"
#include "testmmu.h"

SCENARIO("Verify endianess of reading from MMU", "[IMMU]") {
	GIVEN("TestMMU") {
		std::array<u8, 0x10000> arr = {{0x31, 0xfe, 0xff}};
		TestMMU mmu{arr};

		WHEN("Reading u16 at 0x1") {
			u16 a = mmu.read16(1);

			THEN("a == 0xfffe") { REQUIRE(a == 0xfffe); }
		}
	}
}
