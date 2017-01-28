#include "mbc1.h"
#include "catch.hpp"

SCENARIO("Writing to and reading from MBC1", "[MBC1]") {
	GIVEN("MBC1 with 2mb rom, 8kb ram") {
		std::vector<u8> data(0x200000);
		data[0x148] = 0x07;
		data[0x149] = 0x00;

		for (u8 i = 1; i < 128; i++) {
			for (auto it = std::begin(data) + i * 0x4000;
			     it != std::begin(data) + (i + 1) * 0x4000; ++it) {
				*it = i;
			}
		}

		MBC1 mbc{std::move(data)};

		WHEN("Reading from 0th bank") {
			THEN("Correct results") {
				REQUIRE(mbc.read8(0x148) == 0x07);
				REQUIRE(mbc.read8(0x149) == 0x00);
			}
		}
		WHEN("Reading from 1st bank") {
			WHEN("Correct results") {
				REQUIRE(mbc.read8(0x4000) == 0x1);
				REQUIRE(mbc.read8(0x4567) == 0x1);
				REQUIRE(mbc.read8(0x7fff) == 0x1);
			}
		}
		WHEN("Switching to 2nd bank") {
			mbc.write8(0x2100, 0x2);

			THEN("Reading yields correct values") {
				REQUIRE(mbc.read8(0x4000) == 0x2);
				REQUIRE(mbc.read8(0x4567) == 0x2);
				REQUIRE(mbc.read8(0x7fff) == 0x2);
			}
		}
		WHEN("Reading from the remaining rom banks") {
			for (u8 i = 0x3; i < 128; i++) {
				mbc.write8(0x2100, i);

				THEN("Reading yields correct values") {
					REQUIRE(mbc.read8(0x4000) == i);
					REQUIRE(mbc.read8(0x4567) == i);
					REQUIRE(mbc.read8(0x7fff) == i);
				}
			}
		}
		WHEN("Writing to and reading from RAM") {
			mbc.write8(0x1000, 0xa);

			mbc.write8(0xa000, 0x45);
			mbc.write8(0xb000, 0x46);
			mbc.write8(0xbfff, 0x47);

			THEN("Reading yields the correct values") {
				REQUIRE(mbc.read8(0xa000) == 0x45);
				REQUIRE(mbc.read8(0xb000) == 0x46);
				REQUIRE(mbc.read8(0xbfff) == 0x47);
			}
		}
	}
	GIVEN("MBC1 with 512kb rom, 32kb ram") {
		std::vector<u8> data(0x80000);
		data[0x148] = 0x05;
		data[0x149] = 0x03;

		for (u8 i = 1; i < 32; i++) {
			for (auto it = std::begin(data) + i * 0x4000;
			     it != std::begin(data) + (i + 1) * 0x4000; ++it) {
				*it = i;
			}
		}

		MBC1 mbc{std::move(data)};

		WHEN("Reading from 0th rom bank") {
			THEN("Reading yields correct values") {
				REQUIRE(mbc.read8(0x148) == 0x05);
				REQUIRE(mbc.read8(0x149) == 0x03);
			}
		}
		WHEN("Reading from the remaining rom banks") {
			for (u8 i = 0x1; i < 32; i++) {
				mbc.write8(0x2100, i);

				THEN("Reading yields correct values") {
					REQUIRE(mbc.read8(0x4000) == i);
					REQUIRE(mbc.read8(0x4567) == i);
					REQUIRE(mbc.read8(0x7fff) == i);
				}
			}
		}
		WHEN("Switching ram bank while in bank mode 0") {
			mbc.write8(0x1000, 0xa);

			mbc.write8(0xa000, 0x12);
			mbc.write8(0xb000, 0x34);

			mbc.write8(0x4100, 0b11100000);

			THEN("ram bank has not changed") {
				REQUIRE(mbc.read8(0xa000) == 0x12);
				REQUIRE(mbc.read8(0xb000) == 0x34);
			}
		}
		WHEN("Switching ram bank while in bank mode 1") {
			mbc.write8(0x1000, 0xa);

			for (u8 i = 0; i < 4; i++) {
				mbc.write8(0x6100, i);

				mbc.write8(0xa000, i);
				mbc.write8(0xb000, i);
				mbc.write8(0xbfff, i);

				THEN("Reading yields correct values") {
					REQUIRE(mbc.read8(0xa000) == i);
					REQUIRE(mbc.read8(0xb000) == i);
					REQUIRE(mbc.read8(0xbfff) == i);
				}
			}
		}
	}
}
