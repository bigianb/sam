#include "m6502.h"
#include <iostream>
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


static void formatAbsoluteInstruction(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo, bool isRead)
{
	int val = (hiVal << 8) + loVal;
	os << opcode << " $" << std::setw(4) << std::setfill('0') << std::hex << val;
	auto funcname = debugInfo.getFunctionName(val);
	if (funcname != "") {
		os << "  " << funcname;
	}
	auto portname = isRead ? debugInfo.getReadPortName(val) : debugInfo.getWritePortName(val);
	if (portname != "") {
		os << "  " << portname;
	}
}

static void formatAbsoluteInstructionR(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstruction(os, opcode, loVal, hiVal, debugInfo, true);
}

static void formatAbsoluteInstructionW(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstruction(os, opcode, loVal, hiVal, debugInfo, false);
}

static void formatAbsoluteXInstructionR(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstructionR(os, opcode, loVal, hiVal, debugInfo);
	os << ", X";
}

static void formatAbsoluteXInstructionW(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstructionW(os, opcode, loVal, hiVal, debugInfo);
	os << ", X";
}

static void formatAbsoluteYInstructionR(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstructionR(os, opcode, loVal, hiVal, debugInfo);
	os << ", Y";
}

static void formatAbsoluteYInstructionW(std::ostringstream& os, const char* opcode, int loVal, int hiVal, DebugInfo& debugInfo)
{
	formatAbsoluteInstructionW(os, opcode, loVal, hiVal, debugInfo);
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


m6502::OpcodeDesc m6502::disassemble(unsigned short pc, DebugInfo& debugInfo)
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
			formatAbsoluteInstructionR(stringStream, "ASL", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
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
			formatAbsoluteInstructionR(stringStream, "JSR", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0x21:
			formatIndirectXInstruction(stringStream, "AND", addressBus.readByte(pc + 1));
			desc.numBytes = 2;
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
			formatAbsoluteInstructionR(stringStream, "JMP", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
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
			formatAbsoluteXInstructionR(stringStream, "EOR", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
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
		case 0x7d:
			formatAbsoluteXInstructionR(stringStream, "ADC", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0x81:
			formatIndirectXInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0x84:
			formatZPageInstruction(stringStream, "STY", addressBus.readByte(pc+1));
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
			formatAbsoluteInstructionW(stringStream, "STA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
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
			formatAbsoluteYInstructionW(stringStream, "STA", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
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
			formatAbsoluteInstructionR(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
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
		case 0xb4:
			formatZPageXInstruction(stringStream, "LDY", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xb5:
			formatZPageXInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xb9:
			formatAbsoluteYInstructionR(stringStream, "LDA", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0xbd:
			formatAbsoluteXInstructionR(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0xbc:
			formatAbsoluteXInstructionR(stringStream, "LDY", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
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
			formatAbsoluteInstructionR(stringStream, "CPY", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0xd0:
			formatRelativeInstruction(stringStream, "BNE", addressBus.readByte(pc+1), pc);
			desc.numBytes = 2;
			break;
		case 0xd8:
			stringStream << "CLD";
			break;
		case 0xd9:
			formatAbsoluteYInstructionR(stringStream, "CMP", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0xdd:
			formatAbsoluteXInstructionR(stringStream, "CMP", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			desc.numBytes = 3;
			break;
		case 0xe0:
			formatImmediateInstruction(stringStream, "CPX", addressBus.readByte(pc+1));
			desc.numBytes = 2;
			break;
		case 0xe5:
			formatZPageInstruction(stringStream, "SBC", addressBus.readByte(pc+1));
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
			formatAbsoluteInstructionR(stringStream, "SBC", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
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

void m6502::reset()
{
	regPC = addressBus.readByte(0xFFFD) * 256 + addressBus.readByte(0xFFFC);
}

std::uint8_t m6502::readIndexedIndirect()
{
	// e.g. OPCODE ($nn,X)
	std::uint8_t nn = addressBus.readByte(regPC+1);
	std::uint8_t zPageAddr = nn + regX;
	return addressBus.readByte(zPageAddr);
}

std::uint8_t m6502::readZeroPageValue()
{
	// e.g. OPCODE $nn
	std::uint8_t zPageAddr = addressBus.readByte(regPC + 1);
	return addressBus.readByte(zPageAddr);
}

void m6502::pushByte(std::uint8_t val)
{
	std::uint16_t addr = regSP + 0x100;
	addressBus.writeByte(addr, val);
	regSP -= 1;
}

void m6502::pushShort(std::uint16_t val)
{
	pushByte((val >> 8) & 0xFF);
	pushByte(val & 0xFF);
}

std::uint8_t m6502::popByte()
{
	regSP += 1;
	std::uint16_t addr = regSP + 0x100;
	return addressBus.readByte(addr);
}

void m6502::doAND(std::uint8_t val)
{
	regA &= val;
	zFlag = regA == 0;
	nFlag = regA >= 0x80;
}

void m6502::doORA(std::uint8_t val)
{
	regA |= val;
	zFlag = regA == 0;
	nFlag = regA >= 0x80;
}

std::uint8_t m6502::doASL(std::uint8_t val)
{
	std::uint8_t outval = val << 1;
	zFlag = outval == 0;
	nFlag = outval >= 0x80;
	cFlag = val >= 0x80;
	return outval;
}

std::uint8_t m6502::doLSR(std::uint8_t val)
{
	std::uint8_t outval = val >> 1;
	zFlag = outval == 0;
	nFlag = false;
	cFlag = (val & 0x01) == 0x01;
	return outval;
}

std::uint8_t m6502::doROL(std::uint8_t val)
{
	std::uint8_t outval = val << 1;
	if (cFlag) {
		outval |= 0x01;
	}
	zFlag = outval == 0;
	nFlag = outval >= 0x80;
	cFlag = val >= 0x80;
	return outval;
}

void m6502::doBranch(bool predicate, std::int8_t offset)
{
	regPC += 2;
	cycleCount += 2;
	if (predicate) {
		int targetAddr = regPC + offset;
		if ((targetAddr & 0xFF00) != (regPC & 0xFF00)) {
			cycleCount += 2;
		}
		else {
			cycleCount += 1;
		}
		regPC = targetAddr;
	}
}

void m6502::step()
{
	int opcode = addressBus.readByte(regPC);
	int cycles = 0;
	switch (opcode)
	{
		case 0x01:
			// ORA ($nn,X)
			{
				doORA(readIndexedIndirect());
				regPC += 2;
				cycleCount += 6;
			}
			break;
		case 0x05:
			// ORA $nn
			{
				doORA(readZeroPageValue());
				regPC += 2;
				cycleCount += 3;
			}
			break;
		case 0x06:
			{
				// ASL $nn
				std::uint8_t zPageAddr = addressBus.readByte(regPC + 1);
				std::uint8_t newVal = doASL(addressBus.readByte(zPageAddr));
				addressBus.writeByte(zPageAddr, newVal);
				regPC += 2;
				cycleCount += 5;
			}
			break;
		case 0x09:
			// ORA #nn
			{
				doORA(addressBus.readByte(regPC + 1));
				regPC += 2;
				cycleCount += 2;
			}
			break;
		case 0x08:
			// PHP
			{
			/*
				The flags are copied to memory in this arrangement:

				Bit 7 : Negative
				Bit 6 : Overflow
				Bit 5 : Always set
				Bit 4 : Clear if interrupt vectoring, set if BRK or PHP
				Bit 3 : Decimal mode
				Bit 2 : Interrupt disable
				Bit 1 : Zero
				Bit 0 : Carry
			*/
				std::uint8_t flags = 0x30;
				if (nFlag) { flags |= 0x80; }
				if (vFlag) { flags |= 0x40; }
				if (decimalMode) { flags |= 0x08; }
				if (interruptDisable) { flags |= 0x04; }
				if (zFlag) { flags |= 0x02; }
				if (cFlag) { flags |= 0x01; }

				pushByte(flags);

				regPC += 1;
				cycleCount += 3;
			}
			break;
		case 0x0A:
			{
				// ASL A
				regA = doASL(regA);
				regPC += 1;
				cycleCount += 2;
			}
			break;
		case 0x0e:
			{
				// ASL $nnnn
				std::uint16_t addr = addressBus.readByte(regPC + 1) + (addressBus.readByte(regPC + 2) << 8);
				std::uint8_t newVal = doASL(addressBus.readByte(addr));
				addressBus.writeByte(addr, newVal);
				regPC += 3;
				cycleCount += 6;
			}
			break;
		case 0x10:
			// BPL $nn
			{
				doBranch(!nFlag, addressBus.readByte(regPC + 1));
			}
			break;
		case 0x18:
			{
				// CLC
				cFlag = false;
				regPC += 1;
				cycleCount += 2;
			}
			break;
		case 0x20:
			{
				std::uint16_t retAddr = regPC + 2;
				std::uint16_t targetAddr = (addressBus.readByte(regPC + 2) << 8) | addressBus.readByte(regPC + 1);
				pushShort(retAddr);
				regPC = targetAddr;
				cycleCount += 6;
			}
			break;
		case 0x21:
			{
				// AND ($nn,X)
				doAND(readIndexedIndirect());
				regPC += 2;
				cycleCount += 6;
			}
			break;
		case 0x24:
			{
				// BIT $nn
				std::uint8_t bitVal = readZeroPageValue();
				zFlag = (bitVal & regA) == 0;
				nFlag = (bitVal & 0x80) == 0x80;
				vFlag = (bitVal & 0x40) == 0x40;
				regPC += 2;
				cycleCount += 3;
			}
			break;
		case 0x25:
			{
				doAND(readZeroPageValue());
				regPC += 2;
				cycleCount += 3;
			}
			break;
		case 0x26:
			{
				// ROL $nn
				std::uint8_t zPageAddr = addressBus.readByte(regPC + 1);
				std::uint8_t newVal = doROL(addressBus.readByte(zPageAddr));
				addressBus.writeByte(zPageAddr, newVal);
				regPC += 2;
				cycleCount += 5;
			}
			break;
		case 0x28:
			{
				// PLP
				std::uint8_t flags = popByte();
				nFlag = (flags & 0x80) == 0x80;
				vFlag = (flags & 0x40) == 0x40;
				decimalMode = (flags & 0x08) == 0x08;
				interruptDisable = (flags & 0x04) == 0x04;
				zFlag = (flags & 0x02) == 0x02;
				cFlag = (flags & 0x01) == 0x01;

				regPC += 1;
				cycleCount += 4;
			}
			break;
		case 0x29:
			// AND #nn
			{
				doAND(addressBus.readByte(regPC + 1));
				regPC += 2;
				cycleCount += 2;
			}
			break;
		case 0x30:
			// BMI $nn
			{
				doBranch(nFlag, addressBus.readByte(regPC + 1));
			}
			break;
		case 0x38:
			{
				// SEC
				cFlag = true;
				regPC += 1;
				cycleCount += 2;
			}
			break;
		case 0x40:
			//stringStream << "RTI";
			break;
		case 0x46:
			{
				// LSR $nn
				std::uint8_t zPageAddr = addressBus.readByte(regPC + 1);
				std::uint8_t newVal = doLSR(addressBus.readByte(zPageAddr));
				addressBus.writeByte(zPageAddr, newVal);
				regPC += 2;
				cycleCount += 5;
			}
			break;
		case 0x48:
			//stringStream << "PHA";
			break;
		case 0x49:
			//formatImmediateInstruction(stringStream, "EOR", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x4A:
			//stringStream << "LSR A";
			break;
		case 0x4c:
			//formatAbsoluteInstructionR(stringStream, "JMP", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0x50:
			//formatRelativeInstruction(stringStream, "BVC", addressBus.readByte(pc+1), pc);
			//desc.numBytes = 2;
			break;
		case 0x58:
			//stringStream << "CLI";
			break;
		case 0x5d:
			//formatAbsoluteXInstructionR(stringStream, "EOR", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0x60:
			//stringStream << "RTS";
			break;
		case 0x65:
			//formatZPageInstruction(stringStream, "ADC", addressBus.readByte(pc + 1));
			//desc.numBytes = 2;
			break;
		case 0x68:
			//stringStream << "PLA";
			break;
		case 0x69:
			//formatImmediateInstruction(stringStream, "ADC", addressBus.readByte(pc + 1));
			//desc.numBytes = 2;
			break;
		case 0x6A:
			//stringStream << "ROR A";
			break;
		case 0x78:
			//stringStream << "SEI";
			break;
		case 0x7d:
			//formatAbsoluteXInstructionR(stringStream, "ADC", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0x81:
			//formatIndirectXInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x84:
			//formatZPageInstruction(stringStream, "STY", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x85:
			//formatZPageInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x86:
			//formatZPageInstruction(stringStream, "STX", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x88:
			//stringStream << "DEY";
			break;
		case 0x8a:
			//stringStream << "TXA";
			break;
		case 0x8d:
			//formatAbsoluteInstructionW(stringStream, "STA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0x90:
			//formatRelativeInstruction(stringStream, "BCC", addressBus.readByte(pc+1), pc);
			//desc.numBytes = 2;
			break;
		case 0x91:
			//formatIndirectYInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x95:
			//formatZPageXInstruction(stringStream, "STA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0x98:
			//stringStream << "TYA";
			break;
		case 0x99:
			//formatAbsoluteYInstructionW(stringStream, "STA", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0x9A:
			//stringStream << "TXS";
			break;
		case 0xa0:
			//formatImmediateInstruction(stringStream, "LDY", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xA2:
			//formatImmediateInstruction(stringStream, "LDX", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xa4:
			//formatZPageInstruction(stringStream, "LDY", addressBus.readByte(pc + 1));
			//desc.numBytes = 2;
			break;
		case 0xa5:
			//formatZPageInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xa6:
			//formatZPageInstruction(stringStream, "LDX", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xa8:
			//stringStream << "TAY";
			break;
		case 0xA9:
			//formatImmediateInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xAA:
			//stringStream << "TAX";
			break;
		case 0xad:
			//formatAbsoluteInstructionR(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xb0:
			//formatRelativeInstruction(stringStream, "BCS", addressBus.readByte(pc + 1), pc);
			//desc.numBytes = 2;
			break;
		case 0xb1:
			//formatIndirectYInstruction(stringStream, "LDA", addressBus.readByte(pc + 1));
			//desc.numBytes = 2;
			break;
		case 0xb4:
			//formatZPageXInstruction(stringStream, "LDY", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xb5:
			//formatZPageXInstruction(stringStream, "LDA", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xb9:
			//formatAbsoluteYInstructionR(stringStream, "LDA", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xbd:
			//formatAbsoluteXInstructionR(stringStream, "LDA", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xbc:
			//formatAbsoluteXInstructionR(stringStream, "LDY", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xc5:
			//formatZPageInstruction(stringStream, "CMP", addressBus.readByte(pc + 1));
			//desc.numBytes = 2;
			break;
		case 0xc6:
			//formatZPageInstruction(stringStream, "DEC", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xc8:
			//stringStream << "INY";
			break;
		case 0xc9:
			//formatImmediateInstruction(stringStream, "CMP", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xca:
			//stringStream << "DEX";
			break;
		case 0xcc:
			//formatAbsoluteInstructionR(stringStream, "CPY", addressBus.readByte(pc+1), addressBus.readByte(pc+2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xd0:
			//formatRelativeInstruction(stringStream, "BNE", addressBus.readByte(pc+1), pc);
			//desc.numBytes = 2;
			break;
		case 0xd8:
			//stringStream << "CLD";
			break;
		case 0xd9:
			//formatAbsoluteYInstructionR(stringStream, "CMP", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xdd:
			//formatAbsoluteXInstructionR(stringStream, "CMP", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xe0:
			//formatImmediateInstruction(stringStream, "CPX", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xe5:
			//formatZPageInstruction(stringStream, "SBC", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xe6:
			//formatZPageInstruction(stringStream, "INC", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xe8:
			//stringStream << "INX";
			break;
		case 0xe9:
			//formatImmediateInstruction(stringStream, "SBC", addressBus.readByte(pc+1));
			//desc.numBytes = 2;
			break;
		case 0xea:
			//stringStream << "NOP";
			break;
		case 0xed:
			//formatAbsoluteInstructionR(stringStream, "SBC", addressBus.readByte(pc + 1), addressBus.readByte(pc + 2), debugInfo);
			//desc.numBytes = 3;
			break;
		case 0xf0:
			//formatRelativeInstruction(stringStream, "BEQ", addressBus.readByte(pc+1), pc);
			//desc.numBytes = 2;
			break;
		case 0xf8:
			//stringStream << "SED";
			break;
		default:
			std::cerr << "unknown opcode: 0x" << std::setw(2) << std::setfill('0') << std::hex << opcode;
			break;
	}
}
