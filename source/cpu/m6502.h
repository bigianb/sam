#pragma once

#include<string>
#include "../address_bus.h"
#include "../debug_info.h"

class m6502
{
public:
	m6502(AddressBus& ab) : addressBus(ab) {}
	void step();
	
	class OpcodeDesc
	{
	public:
		std::string line;
		unsigned short pc;
		int numBytes;
	};

	OpcodeDesc disassemble(unsigned short pc, DebugInfo& info);

	unsigned char	regA;
	unsigned char	regX;
	unsigned char	regY;

	unsigned short SPReg;
	unsigned short PCReg;

private:
	AddressBus & addressBus;

};
