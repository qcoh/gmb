#include <fstream>
#include <iterator>
#include <memory>
#include <vector>

#include "icartridge.h"
#include "mbc1.h"
#include "romonly.h"

std::unique_ptr<ICartridge> fromFile(const std::string& path) {
	std::ifstream f{};
	f.exceptions(std::ifstream::failbit);
	f.open(path, std::ios::binary);
	std::vector<u8> data{std::istreambuf_iterator<char>{f}, {}};

	switch (data[0x147]) {
	case 0x00:  // ROM ONLY
		return std::make_unique<RomOnly>(std::move(data));
	case 0x03:  // MBC1+RAM+BATTERY
		return std::make_unique<MBC1>(std::move(data));
	default:
		throw std::runtime_error{"Unknown cartridge type"};
	}
}
