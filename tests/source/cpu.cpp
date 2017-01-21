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
		WHEN("Calling XOR (HL)") {
			data.hl = 0x1234;
			arr[data.hl] = 0b10101010;
			data.a = 0b01010101;
			data.op = 0xae;
			cpu.exec();

			THEN(
			    "a == 0xff, zeroFlag == false, negFlag == false, "
			    "halfFlag == false, carryFlag == false") {
				REQUIRE(data.a == 0xff);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
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
		WHEN("Calling LD (C + 0xff00), A") {
			data.c = 0x25;
			data.a = 0x11;
			data.op = 0xe2;
			cpu.exec();

			THEN("arr[data.c + 0xff00] == 0x11") {
				REQUIRE(arr[data.c + 0xff00] == 0x11);
			}
		}
		WHEN("Calling LD A, (C + 0xff00)") {
			data.c = 0xbb;
			arr[data.c + 0xff00] = 0x43;
			data.op = 0xf2;
			cpu.exec();

			THEN("a == 0x43") { REQUIRE(data.a == 0x43); }
		}
		WHEN("Calling INC B") {
			data.b = 0x45;
			data.op = 0x04;
			cpu.exec();

			THEN(
			    "b == 0x46, halfFlag == false, zeroFlag == false, "
			    "negFlag == false") {
				REQUIRE(data.b == 0x46);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling INC A") {
			data.a = 0xff;
			data.op = 0x3c;
			cpu.exec();

			THEN(
			    "a == 0, halfFlag == true, zeroFlag == true, "
			    "negFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.halfFlag == true);
				REQUIRE(data.zeroFlag == true);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling INC (HL)") {
			data.hl = 0x45;
			arr[data.hl] = 0x10;
			data.op = 0x34;
			cpu.exec();

			THEN(
			    "arr[data.hl] == 0x11, halfFlag == false, zeroFlag "
			    "== false, negFlag == false") {
				REQUIRE(arr[data.hl] == 0x11);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling LD register, register (1)") {
			data.c = 0x25;
			data.op = 0x69;  // LD L, C
			cpu.exec();

			THEN("l == 0x25") { REQUIRE(data.l == 0x25); }
		}
		WHEN("Calling LD register, register (2)") {
			data.h = 0x11;
			data.op = 0x54;  // LD D, H
			cpu.exec();

			THEN("d == 0x11") { REQUIRE(data.d == 0x11); }
		}
		WHEN("Calling LD register, (HL)") {
			data.hl = 0x100;
			arr[data.hl] = 0xee;
			data.op = 0x7e;  // LD A, (HL)
			cpu.exec();

			THEN("a == 0xee") { REQUIRE(data.a == 0xee); }
		}
		WHEN("Calling LD (HL), register") {
			data.b = 0x77;
			data.hl = 0x99;
			data.op = 0x70;
			cpu.exec();

			THEN("arr[data.hl] == 0x77") {
				REQUIRE(arr[data.hl] == 0x77);
			}
		}
		WHEN("Calling LD (n+0xff00), A") {
			data.n = 0x47;
			data.a = 0x66;
			data.op = 0xe0;
			cpu.exec();

			THEN("arr[data.n + 0xff00] == 0x66") {
				REQUIRE(arr[data.n + 0xff00] == 0x66);
			}
		}
		WHEN("Calling LD A, (n + 0xff00)") {
			data.n = 0x26;
			arr[data.n + 0xff00] = 0x99;
			data.op = 0xf0;
			cpu.exec();

			THEN("a == 0x99") { REQUIRE(data.a == 0x99); }
		}
		WHEN("Calling LD (BC), A") {
			data.bc = 0x25;
			data.a = 0x12;
			data.op = 0x02;
			cpu.exec();

			THEN("arr[data.bc] == 0x12") {
				REQUIRE(arr[data.bc] == 0x12);
			}
		}
		WHEN("Calling LD (DE), A") {
			data.de = 0x100;
			data.a = 0x88;
			data.op = 0x12;
			cpu.exec();

			THEN("arr[data.de] == 0x88") {
				REQUIRE(arr[data.de] == 0x88);
			}
		}
		WHEN("Calling LD A, (BC)") {
			data.bc = 0x600;
			arr[data.bc] = 0x06;
			data.op = 0x0a;
			cpu.exec();

			THEN("a == 0x06") { REQUIRE(data.a == 0x06); }
		}
		WHEN("Calling LD A, (DE)") {
			data.de = 0x55;
			arr[data.de] = 0x89;
			data.op = 0x1a;
			cpu.exec();

			THEN("a == 0x89") { REQUIRE(data.a == 0x89); }
		}
		WHEN("Calling CALL nn (1)") {
			data.sp = 0xffff;
			data.pc = 0x1234;
			data.nn = 0x150;
			data.op = 0xcd;
			cpu.exec();

			THEN(
			    "data.sp == 0xfffd, data.pc == 0x150, arr[0xfffe] "
			    "== 0x12, arr[0xfffd] == 0x34, cycles == 24") {
				REQUIRE(data.sp == 0xfffd);
				REQUIRE(data.pc == 0x150);
				REQUIRE(arr[0xfffe] == 0x12);
				REQUIRE(arr[0xfffd] == 0x34);
				REQUIRE(data.cycles == 24);
			}
		}
		WHEN("Calling CALL nn (2)") {
			data.sp = 0x1000;
			data.pc = 0x96;
			data.nn = 0x7799;
			data.op = 0xcd;
			cpu.exec();

			THEN(
			    "sp == 0xffe, pc == 0x7799, arr[0xfff] == 0 "
			    "arr[0xffe] == 0x96, cycles == 24") {
				REQUIRE(data.sp == 0xffe);
				REQUIRE(data.pc == 0x7799);
				REQUIRE(arr[0xfff] == 0);
				REQUIRE(arr[0xffe] == 0x96);
				REQUIRE(data.cycles == 24);
			}
		}
		WHEN("Calling PUSH BC") {
			data.sp = 0xffff;
			data.bc = 0x1234;
			data.op = 0xc5;
			cpu.exec();

			THEN(
			    "arr[0xfffe] == 0x12, arr[0xfffd] == 0x34, sp == "
			    "0xfffd") {
				REQUIRE(arr[0xfffe] == 0x12);
				REQUIRE(arr[0xfffd] == 0x34);
				REQUIRE(data.sp == 0xfffd);
			}
		}
		WHEN("Calling PUSH DE") {
			data.sp = 0xffff;
			data.de = 0x1234;
			data.op = 0xd5;
			cpu.exec();

			THEN(
			    "arr[0xfffe] == 0x12, arr[0xfffd] == 0x34, sp == "
			    "0xfffd") {
				REQUIRE(arr[0xfffe] == 0x12);
				REQUIRE(arr[0xfffd] == 0x34);
				REQUIRE(data.sp == 0xfffd);
			}
		}
		WHEN("Calling PUSH HL") {
			data.sp = 0xffff;
			data.hl = 0x1234;
			data.op = 0xe5;
			cpu.exec();

			THEN(
			    "arr[0xfffe] == 0x12, arr[0xfffd] == 0x34, sp == "
			    "0xfffd") {
				REQUIRE(arr[0xfffe] == 0x12);
				REQUIRE(arr[0xfffd] == 0x34);
				REQUIRE(data.sp == 0xfffd);
			}
		}
		WHEN("Calling PUSH AF") {
			data.sp = 0xffff;
			data.af = 0x1234;
			data.op = 0xf5;
			cpu.exec();

			THEN(
			    "arr[0xfffe] == 0x12, arr[0xfffd] == 0x34, sp == "
			    "0xfffd") {
				REQUIRE(arr[0xfffe] == 0x12);
				REQUIRE(arr[0xfffd] == 0x34);
				REQUIRE(data.sp == 0xfffd);
			}
		}
		WHEN("Calling RLCA (1)") {
			data.a = 0xf0;
			data.op = 0x07;
			cpu.exec();

			THEN(
			    "a == 0b11100001, carryFlag == true, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0b11100001);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RLCA (2)") {
			data.a = 0;
			data.op = 0x07;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == false, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RLA (1)") {
			data.a = 0b10000000;
			data.carryFlag = true;
			data.op = 0x17;
			cpu.exec();

			THEN(
			    "a == 1, carryFlag == true, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 1);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RLA (2)") {
			data.a = 0b10000000;
			data.carryFlag = false;
			data.op = 0x17;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == true, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RRCA (1)") {
			data.a = 1;
			data.op = 0x0f;
			cpu.exec();

			THEN(
			    "a == 0b10000000, carryFlag == true, halfFlag == "
			    "false, negFlag == false, zeroFlag == false") {
				REQUIRE(data.a == 0b10000000);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.zeroFlag == false);
			}
		}
		WHEN("Calling RRCA (2)") {
			data.a = 0;
			data.op = 0x0f;
			cpu.exec();

			THEN(
			    "a == 0, carryFlag == false, zeroFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.halfFlag == false);
				REQUIRE(data.negFlag == false);
			}
		}
		WHEN("Calling RRA (1)") {
			data.a = 1;
			data.carryFlag = true;
			data.op = 0x1f;
			cpu.exec();

			THEN(
			    "a == 0b10000000, carryFlag == true, zeroFlag == "
			    "false, negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0b10000000);
				REQUIRE(data.carryFlag == true);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
			}
		}
		WHEN("Calling RRA (2)") {
			data.a = 0;
			data.op = 0x1f;
			cpu.exec();

			THEN(
			    "a == 0, zeroFlag == false, carryFlag == false, "
			    "negFlag == false, halfFlag == false") {
				REQUIRE(data.a == 0);
				REQUIRE(data.carryFlag == false);
				REQUIRE(data.zeroFlag == false);
				REQUIRE(data.negFlag == false);
				REQUIRE(data.halfFlag == false);
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
			    "data.a == 1, carryFlag == true, zeroFlag "
			    "== "
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
			    "data.b == 0b10, carryFlag == false, "
			    "zeroFlag == "
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
			    "arr[data.hl] == 0b11100001, carryFlag == "
			    "true, "
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
			    "arr[data.hl] == 0, carryFlag == false, "
			    "zeroFlag "
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
			    "h == 0b01010101, carryFlag == false, "
			    "zeroFlag == "
			    "false, negFlag == false, halfFlag == "
			    "false") {
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
			    "e == 0b10101010, carryFlag == true, "
			    "zeroFlag == "
			    "false, negFlag == false, halfFlag == "
			    "false") {
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
			    "arr[data.hl] == 0, carryFlag == false, "
			    "zeroFlag "
			    "== true, negFlag == false, halfFlag == "
			    "false") {
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
			    "arr[data.hl] == 0b10000111, carryFlag == "
			    "true, "
			    "zeroFlag == false, negFlag == false, "
			    "halfFlag == "
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
			    "b == 1, carryFlag == false, zeroFlag == "
			    "false, "
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
			    "data.zeroFlag == true, data.halfFlag == "
			    "false, "
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
			    "arr[data.hl] == 0xfe, data.CarryFlag == "
			    "true, "
			    "data.zeroFlag == false, data.negFlag == "
			    "false, "
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
			    "b == 0b11111000, carryFlag == false, "
			    "zeroFlag == "
			    "false, negFlag == false, halfFlag == "
			    "false") {
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
			    "a == 0, carryFlag == true, zeroFlag == "
			    "true, "
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
			    "zeroFlag == false, negFlag == false, "
			    "halfFlag == "
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
			    "b == 0b11111110, carryFlag == true, "
			    "zeroFlag == "
			    "false, negFlag == false, halfFlag == "
			    "false") {
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
			    "a == 0, carryFlag == true, zeroFlag == "
			    "true, "
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
			    "arr[data.hl] == 0b00011110, carryFlag == "
			    "false, "
			    "zeroFlag == false, negFlag == false, "
			    "halfFlag == "
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
			    "b == 0b11000000, carryFlag == false, "
			    "zeroFlag == "
			    "false, negFlag == false, halfFlag == "
			    "false") {
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
			    "a == 0, carryFlag == true, zeroFlag == "
			    "true, "
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
			    "arr[data.hl] == 0xff, carryFlag == true, "
			    "zeroFlag "
			    "== false, negFlag == false, halfFlag == "
			    "false") {
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
			    "b == 0x0f, carryFlag == false, zeroFlag "
			    "== false, "
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
			    "a == 0, zeroFlag == true, carryFlag == "
			    "false, "
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
			    "arr[data.hl] == 0b00001001, carryFlag == "
			    "false, "
			    "zeroFlag == false, negFlag == false, "
			    "halfFlag == "
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
			    "b == 0, carryFlag == true, zeroFlag == "
			    "true, "
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
			    "arr[data.hl] == 0b01000000, carryFlag == "
			    "false, "
			    "zeroFlag == false, negFlag == false, "
			    "halfFlag == "
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
			    "data.zeroFlag == false, data.negFlag == "
			    "false, "
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
			    "data.zeroFlag == false, data.negFlag == "
			    "false, "
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
			    "data.zeroFlag == true, data.negFlag == "
			    "false, "
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
