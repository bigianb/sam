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
	std::uint16_t getIndexedIndirectAddress();
	std::uint16_t getIndirectIndexedYAddress();
	std::uint8_t readIndexedIndirect();
	std::uint8_t readIndirectIndexedY();
	void writeIndirectIndexedY(std::uint8_t val);
	void writeIndexedIndirect(std::uint8_t val);
	std::uint8_t readZeroPageValue();
	std::uint8_t readZeroPageXValue();
	void writeZeroPageValue(std::uint8_t val);
	void writeZeroPageXValue(std::uint8_t val);
	void writeAbsolute(std::uint8_t val);
	void writeAbsoluteY(std::uint8_t val);
	std::uint8_t readAbsolute();
	std::uint8_t readAbsoluteX();
	std::uint8_t readAbsoluteY();
	void doADC(std::uint8_t val);
	void doAND(std::uint8_t val);
	void doEOR(std::uint8_t val);
	void doORA(std::uint8_t val);
	void doCMP(std::uint8_t val);
	void doCPY(std::uint8_t val);
	std::uint8_t doROL(std::uint8_t val);
	std::uint8_t doROR(std::uint8_t val);
	std::uint8_t doASL(std::uint8_t val);
	std::uint8_t doLSR(std::uint8_t val);

	void pushByte(std::uint8_t val);
	std::uint8_t popByte();
	std::uint16_t popShort();
	void pushShort(std::uint16_t val);
	void doBranch(bool predicate, std::int8_t offset);

	void setNZFlags(std::uint8_t val){
				zFlag = val == 0;
				nFlag = val >= 0x80;
		}

private:
	AddressBus & addressBus;

};
