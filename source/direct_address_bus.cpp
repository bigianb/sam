#include "direct_address_bus.h"

std::uint8_t DirectAddressBus::readByte(std::uint32_t address)
{
	return ram.bytes[address];
}
