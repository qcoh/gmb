#include "catch.hpp"
#include "testbios.h"

SCENARIO("Verify endianess of reading from BIOS", "[BIOS]") {
	GIVEN("TestBIOS") {
		TestBIOS tb{};

		WHEN("Reading u16 at 0x1") {
			u16 a = tb.read16(1);

			THEN("a == 0xfffe") { REQUIRE(a == 0xfffe); }
		}
	}
}
