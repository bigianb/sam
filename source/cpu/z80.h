#pragma once

#include<string>

class z80
{
public:
	void step();
	std::string disassemble();

	enum class RegNum
	{
		B=0, C=1, H=2, L=3, D=4, E=5, A = 6, F = 7,
		BC=0, HL=1, DE=2, AF=3
	};

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

};
