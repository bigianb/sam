#pragma once

#include <cstdint>

class AddressBus
{
public:
	virtual std::uint8_t readByte(std::uint32_t address) = 0;
	virtual void writeByte(std::uint32_t address, std::uint8_t val) = 0;
};