#pragma once
#include "address_bus.h"
#include "ram.h"

class DirectAddressBus : public AddressBus
{
public:
	DirectAddressBus(Ram r) : ram(r) {}
	std::uint8_t readByte(std::uint32_t address);

	void setRam(Ram& ramIn) { ram = ramIn; }

private:
	Ram & ram;
};
