#include "immu.h"
#include "romonly.h"

IMMU::Data::Data(const std::string& romPath, const std::string& biosPath)
    : bios{biosPath}, cart{std::make_unique<RomOnly>(romPath)} {}
