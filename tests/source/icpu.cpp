#include "icpu.h"
#include "catch.hpp"

SCENARIO("Verify Endianess of registers", "[ICPU::Data]") {
	GIVEN("ICPU::Data") {
		ICPU::Data data;
		WHEN("data.af = 0xfffe") {
			data.af = 0xfffe;

			THEN("data.a = 0xff, data.f = 0xfe") {
				REQUIRE(data.a == 0xff);
				REQUIRE(data.f == 0xfe);
			}
		}
		WHEN("data.nn = 0x1234") {
			data.nn = 0x1234;

			THEN("data.n == 0x34") { REQUIRE(data.n == 0x34); }
		}
	}
}
