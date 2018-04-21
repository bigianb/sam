#include "m6502.h"
#include <sstream>

m6502::OpcodeDesc m6502::disassemble(unsigned short pc)
{
	m6502::OpcodeDesc desc;

	std::uint8_t val = addressBus.readByte(pc);

	std::ostringstream stringStream;
	stringStream << val;
	desc.line = stringStream.str();

	return desc;
}