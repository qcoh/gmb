#include <memory>
#include <string>

#include "icartridge.h"
#include "immu.h"

class MMU : public IMMU {
public:
	MMU(IMMU::Data&);
	virtual ~MMU() = default;

	virtual u8 read(u16) override;

private:
	IMMU::Data& m_data;
	std::unique_ptr<ICartridge> m_cart;
};

extern "C" std::unique_ptr<IMMU> loadMMU(IMMU::Data&,
					 std::shared_ptr<ICartridge>);
