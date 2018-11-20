#pragma once

#include <cstdint>

class AddressBus
{
public:
	virtual std::uint8_t readByte(std::uint32_t address) = 0;
	virtual void writeByte(std::uint32_t address, std::uint8_t val) = 0;

	/**
	 * sets a value, bypassing any write handlers. Useful for setting a read value when a write handler is set to the same address.
	*/
	virtual void setByte(std::uint32_t address, std::uint8_t val) = 0;
};