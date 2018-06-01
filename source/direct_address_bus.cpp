#include "direct_address_bus.h"

std::uint8_t DirectAddressBus::readByte(std::uint32_t address)
{
	std::uint32_t targetAddress = address;
	for (const auto& region : mirrorRegions) {
		if (address >= region.start && address <= region.end) {
			targetAddress = address + region.target - region.start;
			break;
		}
	}
	return ram.bytes[targetAddress];
}

void DirectAddressBus::writeByte(std::uint32_t address, std::uint8_t val)
{
	std::uint32_t targetAddress = address;
	for (const auto& region : mirrorRegions) {
		if (address >= region.start && address <= region.end) {
			targetAddress = address + region.target - region.start;
			break;
		}
	}
	ram.bytes[targetAddress] = val;
}

