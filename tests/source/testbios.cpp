#include "testbios.h"

TestBIOS::TestBIOS() : BIOS{} { m_data = {{0x31, 0xfe, 0xff}}; }
