#include "catch.hpp"

#include "bitref.h"

SCENARIO("Verify BitRef", "[BitRef]") {
	GIVEN("u8") {
		u8 foo = 0b00010000;

		WHEN("BitRef{foo, 4}") {
			BitRef<u8> br{foo, 4};

			THEN("implicit conversion operator yields true") {
				REQUIRE(br == true);
			}
		}
	}
	GIVEN("BitRefs to u8") {
		u8 foo = 0b10101010;
		BitRef<u8> br0{foo, 0};
		BitRef<u8> br1{foo, 1};
		BitRef<u8> br2{foo, 2};
		BitRef<u8> br3{foo, 3};
		BitRef<u8> br4{foo, 4};
		BitRef<u8> br5{foo, 5};
		BitRef<u8> br6{foo, 6};
		BitRef<u8> br7{foo, 7};

		WHEN("bitwise negating foo") {
			foo = ~foo;

			THEN("BitRefs flip") {
				REQUIRE(br0 == true);
				REQUIRE(br1 == false);
				REQUIRE(br2 == true);
				REQUIRE(br3 == false);
				REQUIRE(br4 == true);
				REQUIRE(br5 == false);
				REQUIRE(br6 == true);
				REQUIRE(br7 == false);
			}
		}
	}
	GIVEN("BitRef to u8 (1)") {
		u8 foo = 0b11110000;
		BitRef<u8> br{foo, 0};

		WHEN("setting br = true") {
			br = true;

			THEN("foo == 0b11110001") {
				REQUIRE(foo == 0b11110001);
			}
		}
	}
	GIVEN("BitRef to u8 (2)") {
		u8 foo = 0b11111111;
		BitRef<u8> br{foo, 7};

		WHEN("setting br = false") {
			br = false;

			THEN("foo == 0b01111111") {
				REQUIRE(foo == 0b01111111);
			}
		}
	}
}
