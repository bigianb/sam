#pragma once

#include <cstdint>

class AddressBus
{
public:
	virtual std::uint8_t readByte(std::uint32_t address) = 0;

};