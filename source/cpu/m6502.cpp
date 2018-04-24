#include "m6502.h"
#include <sstream>
#include <iomanip>

static void formatImmediateInstruction(std::ostringstream& os, const char* opcode, int immVal)
{
	os << opcode << " #" << std::setw(2) << std::setfill('0') << std::hex << immVal;
}

static void formatZPageInstruction(std::ostringstream& os, const char* opcode, int immVal)
{
	os << opcode << " $" << std::setw(2) << std::setfill('0') << std::hex << immVal;
}

static void formatRelativeInstruction(std::ostringstream& os, const char* opcode, int relVal, int pcVal)
{
	int offset = (int)((char)relVal);
	os << opcode << " *" << relVal << "   -> 0x" << std::setw(2) << std::setfill('0') << std::hex << pcVal+offset;
}

static void formatAbsoluteInstruction(std::ostringstream& os, const char* opcode, int loVal, int hiVal)
{
	os << opcode << " $" << std::setw(2) << std::setfill('0') << std::hex << hiVal << loVal;
}

static void formatAbsoluteXInstruction(std::ostringstream& os, const char* opcode, int loVal, int hiVal)
{
	formatAbsoluteInstruction(os, opcode, loVal, hiVal);
	os << ", X";
}

m6502::OpcodeDesc m6502::disassemble(unsigned short pc)
{
	m6502::OpcodeDesc desc;
	desc.numBytes = 1;

	int opcode = addressBus.readByte(pc);
	int byte2 = addressBus.readByte(pc+1);
	std::ostringstream stringStream;

	switch (opcode)
	{
		case 0x05:
			formatZPageInstruction(stringStream, "ORA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x0e:
			formatAbsoluteInstruction(stringStream, "ASL", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x20:
			formatAbsoluteInstruction(stringStream, "JSR", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x29:
			formatImmediateInstruction(stringStream, "AND", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x58:
			stringStream << "CLI";
			break;
		case 0x85:
			formatZPageInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x86:
			formatZPageInstruction(stringStream, "STX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x9A:
			stringStream << "TXS";
			break;
		case 0xA2:
			formatImmediateInstruction(stringStream, "LDX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xA9:
			formatImmediateInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xAA:
			stringStream << "TAX";
			break;
			break;
		case 0xad:
			formatAbsoluteXInstruction(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0xbd:
			formatImmediateInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xd8:
			stringStream << "CLD";
			break;
		case 0xf0:
			formatRelativeInstruction(stringStream, "BEQ", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		default:
			stringStream << "unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << opcode;
		break;
	}
	desc.line = stringStream.str();
	return desc;
}