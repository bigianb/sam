#pragma once
#include "address_bus.h"
#include "ram.h"

class MirrorRegion
{
public:
	MirrorRegion(std::uint32_t s, std::uint32_t e, std::uint32_t t) : start(s), end(e), target(t) {}
	std::uint32_t start;
	std::uint32_t end;
	std::uint32_t target;
};

class DirectAddressBus : public AddressBus
{
public:
	DirectAddressBus(Ram& r) : ram(r) {}
	std::uint8_t readByte(std::uint32_t address);

	void setRam(Ram& ramIn) { ram = ramIn; }

	/**
	 * Causes the memory region at start -> end to be mapped to target.
	*/
	void setMirror(std::uint32_t start, std::uint32_t end, std::uint32_t target) {
		mirrorRegions.push_back(MirrorRegion(start, end, target));
	}

private:
	Ram & ram;

	std::vector<MirrorRegion> mirrorRegions;
};
