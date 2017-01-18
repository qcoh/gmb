#include "gameboy.h"
#include "cpu.h"

GameBoy::GameBoy() : m_cpu{std::make_unique<CPU>(m_cpuData)} {}
