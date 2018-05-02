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

static void formatZPageXInstruction(std::ostringstream& os, const char* opcode, int val)
{
	formatZPageInstruction(os, opcode, val);
	os << ", X";
}

static void formatRelativeInstruction(std::ostringstream& os, const char* opcode, int relVal, int pcVal)
{
	int offset = (int)((char)relVal);
	os << opcode << " *" << offset << "   -> 0x" << std::setw(2) << std::setfill('0') << std::hex << pcVal+offset+2;
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

static void formatAbsoluteYInstruction(std::ostringstream& os, const char* opcode, int loVal, int hiVal)
{
	formatAbsoluteInstruction(os, opcode, loVal, hiVal);
	os << ", Y";
}

static void formatIndirectXInstruction(std::ostringstream& os, const char* opcode, int val)
{
	os << opcode << " ($" << std::setw(2) << std::setfill('0') << std::hex << val << ", X)";
}

static void formatIndirectYInstruction(std::ostringstream& os, const char* opcode, int val)
{
	os << opcode << " ($" << std::setw(2) << std::setfill('0') << std::hex << val << "), Y";
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
		case 0x01:
			formatIndirectXInstruction(stringStream, "ORA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x05:
			formatZPageInstruction(stringStream, "ORA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x06:
			formatZPageInstruction(stringStream, "ASL", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0x09:
			formatImmediateInstruction(stringStream, "ORA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x08:
			stringStream << "PHP";
			break;
		case 0x0A:
			stringStream << "ASL A";
			break;
		case 0x0e:
			formatAbsoluteInstruction(stringStream, "ASL", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x10:
			formatRelativeInstruction(stringStream, "BPL", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0x18:
			stringStream << "CLC";
			break;
		case 0x20:
			formatAbsoluteInstruction(stringStream, "JSR", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x24:
			formatZPageInstruction(stringStream, "BIT", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x25:
			formatZPageInstruction(stringStream, "AND", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0x26:
			formatZPageInstruction(stringStream, "ROL", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x28:
			stringStream << "PLP";
			break;
		case 0x29:
			formatImmediateInstruction(stringStream, "AND", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x30:
			formatRelativeInstruction(stringStream, "BMI", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0x38:
			stringStream << "SEC";
			break;
		case 0x40:
			stringStream << "RTI";
			break;
		case 0x46:
			formatZPageInstruction(stringStream, "LSR", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0x48:
			stringStream << "PHA";
			break;
		case 0x49:
			formatImmediateInstruction(stringStream, "EOR", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x4A:
			stringStream << "LSR A";
			break;
		case 0x4c:
			formatAbsoluteInstruction(stringStream, "JMP", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x50:
			formatRelativeInstruction(stringStream, "BVC", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0x58:
			stringStream << "CLI";
			break;
		case 0x5d:
			formatAbsoluteXInstruction(stringStream, "EOR", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x60:
			stringStream << "RTS";
			break;
		case 0x65:
			formatZPageInstruction(stringStream, "ADC", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0x68:
			stringStream << "PLA";
			break;
		case 0x69:
			formatImmediateInstruction(stringStream, "ADC", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0x6A:
			stringStream << "ROR A";
			break;
		case 0x78:
			stringStream << "SEI";
			break;
		case 0x81:
			formatIndirectXInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x85:
			formatZPageInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x86:
			formatZPageInstruction(stringStream, "STX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x88:
			stringStream << "DEY";
			break;
		case 0x8a:
			stringStream << "TXA";
			break;
		case 0x8d:
			formatAbsoluteInstruction(stringStream, "STA", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0x90:
			formatRelativeInstruction(stringStream, "BCC", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0x91:
			formatIndirectYInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x95:
			formatZPageXInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x98:
			stringStream << "TYA";
			break;
		case 0x99:
			formatAbsoluteYInstruction(stringStream, "STA", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2));
			desc.numBytes = 3;
			break;
		case 0x9A:
			stringStream << "TXS";
			break;
		case 0xa0:
			formatImmediateInstruction(stringStream, "LDY", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xA2:
			formatImmediateInstruction(stringStream, "LDX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xa4:
			formatZPageInstruction(stringStream, "LDY", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0xa5:
			formatZPageInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xa6:
			formatZPageInstruction(stringStream, "LDX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xa8:
			stringStream << "TAY";
			break;
		case 0xA9:
			formatImmediateInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xAA:
			stringStream << "TAX";
			break;
		case 0xad:
			formatAbsoluteInstruction(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0xb0:
			formatRelativeInstruction(stringStream, "BCS", addressBus.readByte(pc + 1), pc);
			desc.numBytes = 2;
			break;
		case 0xb1:
			formatIndirectYInstruction(stringStream, "LDA", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0xb5:
			formatZPageXInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xbd:
			formatAbsoluteXInstruction(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0xbc:
			formatAbsoluteXInstruction(stringStream, "LDY", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2));
			desc.numBytes = 3;
			break;
		case 0xc5:
			formatZPageInstruction(stringStream, "CMP", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
			break;
		case 0xc6:
			formatZPageInstruction(stringStream, "DEC", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xc8:
			stringStream << "INY";
			break;
		case 0xc9:
			formatImmediateInstruction(stringStream, "CMP", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xca:
			stringStream << "DEX";
			break;
		case 0xcc:
			formatAbsoluteInstruction(stringStream, "CPY", addressBus.readByte(pc+1), addressBus.readByte(pc+2));
			desc.numBytes = 3;
			break;
		case 0xd0:
			formatRelativeInstruction(stringStream, "BNE", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0xd8:
			stringStream << "CLD";
			break;
		case 0xe0:
			formatImmediateInstruction(stringStream, "CPX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xe6:
			formatZPageInstruction(stringStream, "INC", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xe8:
			stringStream << "INX";
			break;
		case 0xe9:
			formatImmediateInstruction(stringStream, "SBC", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xea:
			stringStream << "NOP";
			break;
		case 0xed:
			formatAbsoluteInstruction(stringStream, "SBC", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2));
			desc.numBytes = 3;
			break;
		case 0xf0:
			formatRelativeInstruction(stringStream, "BEQ", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0xf8:
			stringStream << "SED";
			break;
		default:
			stringStream << "unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << opcode;
		break;
	}
	desc.line = stringStream.str();
	return desc;
}