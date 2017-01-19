#include "cpu.h"
#include "catch.hpp"
#include "testmmu.h"

SCENARIO("Testing instructions", "[CPU]") {
	GIVEN("TestCPU, ICPU::Data, TestMMU") {
		std::array<u8, 0x10000> arr = {{0}};
		TestMMU mmu{arr};
		ICPU::Data data;
		CPU cpu{data, &mmu};

		WHEN("LD __, nn") {
			data.nn = 0x1234;
			data.op = 0x31;
			cpu.exec();

			THEN("data.sp == 0x1234") {
				REQUIRE(data.sp == 0x1234);
			}
		}
		WHEN("XOR A, _ (1)") {
			data.a = 0x78;
			data.op = 0xaf;
			cpu.exec();

			THEN(
			    "data.a == 0, data.zeroFlag == true, data.*Flag == "
			    "false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.carryFlag == false);
			}
		}
		WHEN("XOR A, _ (2)") {
			data.a = 0b10101010;
			data.b = 0b01010101;
			data.op = 0xa8;
			cpu.exec();

			THEN("data.a == 0b11111111, data.*Flag == false") {
				REQUIRE(data.a == 0b11111111);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.carryFlag == false);
			}
		}
		WHEN("LDD (HL-), A") {
			data.a = 0x77;
			data.hl = 0x100;
			data.op = 0x32;
			cpu.exec();

			THEN("arr[0x100] == 0x77, data.hl == 0xff") {
				REQUIRE(arr[0x100] == 0x77);
				REQUIRE(data.hl == 0xff);
			}
		}
		WHEN("LDD A, (HL-)") {
			arr[0x100] = 0x25;
			data.hl = 0x100;
			data.op = 0x3a;
			cpu.exec();

			THEN("a == 0x25, data.hl == 0xff") {
				REQUIRE(data.a == 0x25);
				REQUIRE(data.hl == 0xff);
			}
		}
		WHEN("LDI (HL+), A") {
			data.a = 0x77;
			data.hl = 0x100;
			data.op = 0x22;
			cpu.exec();

			THEN("arr[0x100] == 0x77, data.hl == 0x101") {
				REQUIRE(arr[0x100] == 0x77);
				REQUIRE(data.hl == 0x101);
			}
		}
		WHEN("LDI A, (HL+)") {
			arr[0x100] = 0x25;
			data.hl = 0x100;
			data.op = 0x2a;
			cpu.exec();

			THEN("a == 0x25, data.hl == 0x101") {
				REQUIRE(data.a == 0x25);
				REQUIRE(data.hl == 0x101);
			}
		}
	}
}
