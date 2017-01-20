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
		WHEN("Calling JR n (1)") {
			data.pc = 0x1234;
			data.n = static_cast<u8>(-17);
			data.op = 0x18;
			cpu.exec();

			THEN("data.pc == 0x1234 -17") {
				REQUIRE(data.pc == 0x1234 - 17);
			}
		}
		WHEN("Calling JR n (2)") {
			data.pc = 0xbbbb;
			data.n = static_cast<u8>(127);
			data.op = 0x18;
			cpu.exec();

			THEN("data.pc == 0xbbbb + 127") {
				REQUIRE(data.pc == 0xbbbb + 127);
			}
		}
		WHEN("Calling JR C, n") {
			data.pc = 0xaaaa;
			data.n = static_cast<u8>(-128);
			data.carryFlag = true;
			data.op = 0x38;
			cpu.exec();

			THEN("data.pc == 0xaaaa - 128") {
				REQUIRE(data.pc == 0xaaaa - 128);
			}
		}
		WHEN("Calling JR Z, n") {
			data.pc = 0x1111;
			data.n = 113;
			data.zeroFlag = false;
			data.op = 0x28;
			cpu.exec();

			THEN("data.pc == 0x1111") {
				REQUIRE(data.pc == 0x1111);
			}
		}
		WHEN("Calling JR NC, n") {
			data.pc = 0xffff;
			data.n = static_cast<u8>(-85);
			data.carryFlag = false;
			data.op = 0x30;
			cpu.exec();

			THEN("data.pc == 0xffff - 85") {
				REQUIRE(data.pc == 0xffff - 85);
			}
		}
		WHEN("Calling JR NZ, n") {
			data.pc = 0;
			data.n = 12;
			data.zeroFlag = false;
			data.op = 0x20;
			cpu.exec();

			THEN("data.pc == 12") { REQUIRE(data.pc == 12); }
		}
		WHEN("Calling LD _, n on register (1)") {
			data.n = 0x25;
			data.op = 0x6;
			cpu.exec();

			THEN("b == 0x25") { REQUIRE(data.b == 0x25); }
		}
		WHEN("Calling LD _, n on register (2)") {
			data.n = 0x26;
			data.op = 0x2e;
			cpu.exec();

			THEN("l == 0x26") { REQUIRE(data.l == 0x26); }
		}
		WHEN("Calling LD (HL), n") {
			data.hl = 0x1234;
			data.op = 0x36;
			data.n = 0xff;
			cpu.exec();

			THEN("arr[data.hl] == 0xff") {
				REQUIRE(arr[data.hl] == 0xff);
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
		WHEN("Calling RR on register (1)") {
			data.b = 0b11110000;
			data.carryFlag = true;
			data.n = 0x18;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 0b11111000, carryFlag == false, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 0b11111000);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RR on register (2)") {
			data.a = 1;
			data.n = 0x1f;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == true, zeroFlag == true, "
			    "halfFlag == false, negFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RR on (HL)") {
			data.hl = 0x234;
			arr[data.hl] = 0xfe;
			data.carryFlag = true;
			data.n = 0x1e;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0xff, carryFlag == false, "
			    "zeroFlag == false, negFlag == false, halfFlag == "
			    "false") {
				REQUIRE(arr[data.hl] == 0xff);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SLA on register (1)") {
			data.b = 0b11111111;
			data.n = 0x20;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 0b11111110, carryFlag == true, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 0b11111110);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SLA on register (2)") {
			data.a = 0b10000000;
			data.n = 0x27;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == true, zeroFlag == true, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SLA on (HL)") {
			data.hl = 0x826a;
			arr[data.hl] = 0x0f;
			data.n = 0x26;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0b00011110, carryFlag == false, "
			    "zeroFlag == false, negFlag == false, halfFlag == "
			    "false") {
				REQUIRE(arr[data.hl] == 0b00011110);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SRA on register (1)") {
			data.b = 0b10000000;
			data.n = 0x28;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 0b11000000, carryFlag == false, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 0b11000000);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SRA on register (2)") {
			data.a = 1;
			data.n = 0x2f;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == true, zeroFlag == true, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling SRA on (HL)") {
			data.hl = 0x53fa;
			arr[data.hl] = 0xff;
			data.n = 0x2e;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0xff, carryFlag == true, zeroFlag "
			    "== false, negFlag == false, halfFlag == false") {
				REQUIRE(arr[data.hl] == 0xff);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SWAP on register (1)") {
			data.b = 0xf0;
			data.n = 0x30;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 0x0f, carryFlag == false, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 0x0f);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling SWAP on register (2)") {
			data.a = 0x00;
			data.n = 0x37;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "a == 0, zeroFlag == true, carryFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SWAP on (HL)") {
			data.hl = 0xbf11;
			arr[data.hl] = 0b10010000;
			data.n = 0x36;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0b00001001, carryFlag == false, "
			    "zeroFlag == false, negFlag == false, halfFlag == "
			    "false") {
				REQUIRE(arr[data.hl] == 0b00001001);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SRL on register") {
			data.b = 1;
			data.n = 0x38;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "b == 0, carryFlag == true, zeroFlag == true, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.b == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling SRL on (HL)") {
			data.hl = 0xbcf0;
			arr[data.hl] = 0b10000000;
			data.carryFlag = true;
			data.n = 0x3e;
			data.op = 0xcb;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0b01000000, carryFlag == false, "
			    "zeroFlag == false, negFlag == false, halfFlag == "
			    "false") {
				REQUIRE(arr[data.hl] == 0b01000000);
				REQUIRE(data.carryFlag == false);
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
