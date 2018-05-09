#pragma once

#include<string>
#include "../address_bus.h"
#include "../debug_info.h"

class m6502
{
public:
	m6502(AddressBus& ab) : addressBus(ab) {}
	void step();
	

	enum class RegNum
	{
		B=0, C=1, H=2, L=3, D=4, E=5, A = 6, F = 7,
		BC=0, HL=1, DE=2, AF=3
	};

	class OpcodeDesc
	{
	public:
		std::string line;
		unsigned short pc;
		int numBytes;
	};

	OpcodeDesc disassemble(unsigned short pc, DebugInfo& info);

	class Registers
	{
	public:
		union {
			unsigned char eightBit[8];
			unsigned short sixteenBit[4];
		};
	};

	Registers registers;
	Registers registersDash;
	unsigned short IXReg;
	unsigned short IYReg;
	unsigned short SPReg;
	unsigned short PCReg;

private:
	AddressBus & addressBus;

};
