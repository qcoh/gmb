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

SCENARIO("Testing extended instructions", "[CPU]") {
	GIVEN("TestCPU, ICPU::Data, TestMMU") {
		std::array<u8, 0x10000> arr = {{0}};
		TestMMU mmu{arr};
		ICPU::Data data;
		CPU cpu{data, &mmu};

		WHEN("Calling RLC on register (1)") {
			data.a = 0b10000000;
			data.carryFlag = false;
			data.n = 0x7;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.a == 1, carryFlag == true, zeroFlag == "
			    "false") {
				REQUIRE(data.a == 1);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
			}
		}
		WHEN("Calling RLC on register (2)") {
			data.b = 1;
			data.carryFlag = true;
			data.n = 0;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.b == 0b10, carryFlag == false, zeroFlag == "
			    "false") {
				REQUIRE(data.b == 2);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
			}
		}
		WHEN("Calling RLC on (HL) (1)") {
			data.hl = 0x300;
			arr[data.hl] = 0xf0;
			data.carryFlag = false;
			data.n = 0x6;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0b11100001, carryFlag == true, "
			    "zeroFlag == false") {
				REQUIRE(arr[data.hl] == 0b11100001);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
			}
		}
		WHEN("Calling RLC on (HL) (2)") {
			data.hl = 0x10;
			data.carryFlag = true;
			data.n = 0x6;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0, carryFlag == false, zeroFlag "
			    "== true") {
				REQUIRE(arr[data.hl] == 0);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == true);
			}
		}
		WHEN("Calling RRC on registers (1)") {
			data.h = 0b10101010;
			data.carryFlag = true;
			data.n = 0xc;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "h == 0b01010101, carryFlag == false, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.h == 0b01010101);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RRC on registers (2)") {
			data.e = 0b01010101;
			data.carryFlag = false;
			data.n = 0xb;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "e == 0b10101010, carryFlag == true, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.e == 0b10101010);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RRC on (HL) (1)") {
			data.hl = 0x123;
			arr[data.hl] = 0;
			data.carryFlag = true;
			data.n = 0xe;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0, carryFlag == false, zeroFlag "
			    "== true, negFlag == false, halfFlag == false") {
				REQUIRE(arr[data.hl] == 0);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RRC on (HL) (2)") {
			data.hl = 0x456;
			arr[data.hl] = 0b00001111;
			data.carryFlag = false;
			data.n = 0xe;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0b10000111, carryFlag == true, "
			    "zeroFlag == false, negFlag == false, halfFlag == "
			    "false") {
				REQUIRE(arr[data.hl] == 0b10000111);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RL on register (1)") {
			data.b = 0;
			data.carryFlag = true;
			data.n = 0x10;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 1, carryFlag == false, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 1);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RL on register (2)") {
			data.a = 0b10000000;
			data.carryFlag = false;
			data.n = 0x17;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.a == 0, data.carryFlag == true, "
			    "data.zeroFlag == true, data.halfFlag == false, "
			    "data.negFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RL on (HL)") {
			data.hl = 0x11;
			arr[data.hl] = 0xff;
			data.carryFlag = false;
			data.n = 0x16;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0xfe, data.CarryFlag == true, "
			    "data.zeroFlag == false, data.negFlag == false, "
			    "data.halfFlag") {
				REQUIRE(arr[data.hl] == 0xfe);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}

		WHEN("Calling BIT on registers") {
			data.b = 0b10000000;
			data.n = 0x78;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.zeroFlag == false, data.negFlag == false, "
			    "data.halfFlag == true") {
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == true);
			}
		}
		WHEN("Calling BIT on (hl) (1)") {
			data.hl = 0x100;
			arr[data.hl] = 0b00010000;
			data.n = 0x66;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.zeroFlag == false, data.negFlag == false, "
			    "data.halfFlag == true") {
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == true);
			}
		}
		WHEN("Calling BIT on (hl) (2)") {
			data.hl = 0x100;
			arr[data.hl] = 0b11101111;
			data.n = 0x66;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "data.zeroFlag == true, data.negFlag == false, "
			    "data.halfFlag == true") {
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == true);
			}
		}
		WHEN("Calling RES on register") {
			data.b = 0xff;
			data.n = 0x80;
			data.op = 0xcb;
			cpu.exec();

			THEN("data.b = 0xfe") { REQUIRE(data.b == 0xfe); }
		}
		WHEN("Calling RES on (hl)") {
			data.hl = 0x200;
			arr[data.hl] = 0xff;
			data.n = 0x86;
			data.op = 0xcb;
			cpu.exec();

			THEN("arr[data.hl] == 0xfe") {
				REQUIRE(arr[data.hl] == 0xfe);
			}
		}
		WHEN("Calling SET on register") {
			data.b = 0x00;
			data.n = 0xf0;
			data.op = 0xcb;
			cpu.exec();

			THEN("data.b = 0b01000000") {
				REQUIRE(data.b == 0b01000000);
			}
		}
		WHEN("Calling SET on (hl)") {
			data.hl = 0x200;
			arr[data.hl] = 0x0;
			data.n = 0xc6;
			data.op = 0xcb;
			cpu.exec();

			THEN("arr[data.hl] == 1") {
				REQUIRE(arr[data.hl] == 1);
			}
		}
	}
}
