#pragma once

#include<string>
#include "../address_bus.h"
#include "../debug_info.h"

class m6502
{
public:
	m6502(AddressBus& ab) : addressBus(ab) {}
	void reset();
	void step();
	
	class OpcodeDesc
	{
	public:
		std::string line;
		unsigned short pc;
		int numBytes;
	};

	OpcodeDesc disassemble(unsigned short pc, DebugInfo& info);

	std::uint8_t	regA;
	std::uint8_t	regX;
	std::uint8_t	regY;

	std::uint8_t	regSP;
	std::uint16_t	regPC;

	int cycleCount;

	// Status flags
	bool zFlag;
	bool vFlag;
	bool nFlag;
	bool cFlag;
	bool decimalMode;
	bool interruptDisable;

private:
	std::uint8_t readIndexedIndirect();
	std::uint8_t readZeroPageValue();
	void doORA(std::uint8_t val);
	std::uint8_t doASL(std::uint8_t val);
	void pushByte(std::uint8_t val);
	std::uint8_t popByte();

private:
	AddressBus & addressBus;

};
