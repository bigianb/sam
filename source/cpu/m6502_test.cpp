#define BOOST_TEST_MODULE boost_test_attotime
#include <boost/test/included/unit_test.hpp>

#include "m6502.h"
#include "../ram.h"
#include "../direct_address_bus.h"

BOOST_AUTO_TEST_CASE(test_reset)
{
    Ram ram(64 * 1024);
    DirectAddressBus bus(ram);

    ram.bytes[0xFFFD] = 0x01;
    ram.bytes[0xFFFC] = 0x23;

	m6502 cpu(bus);
    cpu.reset();

    BOOST_CHECK_EQUAL(cpu.regPC, 0x0123);
}

struct M6502TestCase
{
	std::string flagsIn;
	std::uint8_t opcode;
	std::uint8_t value1;
	std::uint8_t value2;
	std::uint8_t memval;
	int regA;
	int regX;
	int regY;
	std::string expectedDesc;
	int expectedCycles;
	int expectedRegA;
	int expectedRegX;
	int expectedRegY;
	std::string expectedFlags;

	M6502TestCase(
		std::string flagsIn_in,
		std::uint8_t opcode_in,
		std::uint8_t value1_in,
		int regA_in,
		std::string expectedDesc_in,
		int expectedCycles_in,
		int expectedRegA_in,
		std::string expectedFlags_in) : flagsIn(flagsIn_in), opcode(opcode_in), value1(value1_in),
										regA(regA_in), expectedDesc(expectedDesc_in), expectedCycles(expectedCycles_in),
										expectedRegA(expectedRegA_in), expectedFlags(expectedFlags_in)
	{
	}

	M6502TestCase(
		std::string flagsIn_in,
		std::uint8_t opcode_in,
		std::uint8_t value1_in,
		std::uint8_t value2_in,
		std::uint8_t memval_in,
		int regA_in,
		int regX_in,
		int regY_in,
		std::string expectedDesc_in,
		int expectedCycles_in,
		int expectedRegA_in,
		int expectedRegX_in,
		int expectedRegY_in,
		std::string expectedFlags_in) : flagsIn(flagsIn_in), opcode(opcode_in), value1(value1_in), value2(value2_in), memval(memval_in),
		regA(regA_in), regX(regX_in), regY(regY_in), expectedDesc(expectedDesc_in), expectedCycles(expectedCycles_in),
		expectedRegA(expectedRegA_in), expectedRegX(expectedRegX_in), expectedRegY(expectedRegY_in), expectedFlags(expectedFlags_in)
	{
	}
};

BOOST_AUTO_TEST_CASE(test_absx_instructions)
{
	// Single opcode instructions
	M6502TestCase tests[]{
		//flags, opcode, lo, hi, memval, A, X, Y, desc, cycles, AOut, XOut, YOut, flagsOut
		// 0x70 + 0x10 + carry. Extensive tests for ADC are in the zval case. 
		M6502TestCase("c---z", 0x7d, 0x34, 0x12, 0x70, 0x10, 0x22, 0x33, "ADC $1234, X", 4, 0x81, 0x22, 0x33, "---v-")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	for (const auto& testcase : tests) {
		ram.bytes[0x0123] = testcase.opcode;
		ram.bytes[0x0124] = testcase.value1;
		ram.bytes[0x0125] = testcase.value2;

		int absval = testcase.value1 + (testcase.value2 << 8);
		ram.bytes[absval + testcase.expectedRegX] = testcase.memval;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regX = testcase.regX;
		cpu.regY = testcase.regY;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regX, testcase.expectedRegX);
		BOOST_CHECK_EQUAL(cpu.regY, testcase.expectedRegY);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x126);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_zpage_instructions)
{
	M6502TestCase zeroPageTests[] {
		//flags, opcode, zval, A, desc, cycles, AOut, flagsOut
		M6502TestCase("c---z", 0x65, 0x15, 0x11, "ADC $10", 3, 0x11 + 0x15 + 0x01, "-----"),
		// two positives overflow to a negative
		M6502TestCase("----z", 0x65, 0x70, 0x10, "ADC $10", 3, 0x80, "---v-"),
		// two negatives overflow to a positive
		M6502TestCase("----z", 0x65, 0x80, 0xE0, "ADC $10", 3, 0x60, "c--v-"),
		M6502TestCase("----z", 0x65, 0x70, 0x90, "ADC $10", 3, 0x00, "c---z"),

		// Decimal mode
		M6502TestCase("-d--z", 0x65, 0x16, 0x75, "ADC $10", 3, 0x91, "-d-v-"),
		M6502TestCase("-d--z", 0x65, 0x16, 0x85, "ADC $10", 3, 0x01, "cd---"),
		M6502TestCase("cd--z", 0x65, 0x16, 0x85, "ADC $10", 3, 0x02, "cd---")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	for (const auto& testcase : zeroPageTests){
		ram.bytes[0x0123] = testcase.opcode;
		ram.bytes[0x0124] = 0x10;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		ram.bytes[0x10] = testcase.value1;

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_imm_instructions)
{
	M6502TestCase tests[]{
		//flags, opcode, immval, A, desc, cycles, AOut, flagsOut
		M6502TestCase("c---z", 0x69, 0x15, 0x11, "ADC #15", 2, 0x11 + 0x15 + 0x01, "-----")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	for (const auto& testcase : tests) {
		ram.bytes[0x0123] = testcase.opcode;
		ram.bytes[0x0124] = testcase.value1;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_acc_instructions)
{
	// Single opcode instructions
	M6502TestCase tests[]{
		//flags, opcode, immval, A, desc, cycles, AOut, flagsOut
		M6502TestCase("c---z", 0x6A, 0x00, 0x11, "ROR A", 2, 0x88, "c-n--"),
		M6502TestCase("----z", 0x6A, 0x00, 0x11, "ROR A", 2, 0x08, "c----"),
		M6502TestCase("----z", 0x6A, 0x00, 0x01, "ROR A", 2, 0x00, "c---z"),
		M6502TestCase("cd--z", 0xd8, 0x00, 0x11, "CLD", 2, 0x11, "c---z"),
		M6502TestCase("c---z", 0xf8, 0x00, 0x11, "SED", 2, 0x11, "cd--z"),
		M6502TestCase("c---z", 0xea, 0x00, 0x11, "NOP", 2, 0x11, "c---z")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	for (const auto& testcase : tests) {
		ram.bytes[0x0123] = testcase.opcode;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x124);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_reg_instructions)
{
	// Single opcode instructions
	M6502TestCase tests[]{
		//flags, opcode, lo, hi, memval, A, X, Y, desc, cycles, AOut, XOut, YOut, flagsOut
		M6502TestCase("c---z", 0xe8, 0x00, 0x00, 0x00, 0x10, 0x22, 0x33, "INX", 2, 0x10, 0x23, 0x33, "c----"),
		M6502TestCase("c---z", 0xe8, 0x00, 0x00, 0x00, 0x10, 0xFF, 0x33, "INX", 2, 0x10, 0x00, 0x33, "c---z"),
		M6502TestCase("c---z", 0xe8, 0x00, 0x00, 0x00, 0x10, 0x92, 0x33, "INX", 2, 0x10, 0x93, 0x33, "c-n--")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	for (const auto& testcase : tests) {
		ram.bytes[0x0123] = testcase.opcode;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regX = testcase.regX;
		cpu.regY = testcase.regY;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regX, testcase.expectedRegX);
		BOOST_CHECK_EQUAL(cpu.regY, testcase.expectedRegY);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x124);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_indexed_indirect_instructions)
{
	// Single opcode instructions
	M6502TestCase tests[]{
		// hijack hi to be memval_out
		//            flags, opcode, lo,   mem_out, memval, A,    X,    Y,    desc,          cycles, AOut, XOut, YOut, flagsOut
		M6502TestCase("c---z", 0x81, 0x10, 0x10,    0x00,   0x10, 0x02, 0x33, "STA ($10, X)", 6,    0x10,  0x02, 0x33, "c---z")
	};

	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);

	for (const auto& testcase : tests) {
		cpu.reset();
		ram.bytes[0x0123] = testcase.opcode;
		ram.bytes[0x0124] = testcase.value1;

		int targetzPage = (testcase.regX + testcase.value1) & 0xFF;
		ram.bytes[targetzPage] = 0x45;
		ram.bytes[targetzPage+1] = 0x23;
		ram.bytes[0x2345] = testcase.memval;

		const auto desc = cpu.disassemble(0x123, debugInfo);
		BOOST_CHECK_EQUAL(desc.line, testcase.expectedDesc);

		cpu.cycleCount = 0;
		cpu.regA = testcase.regA;
		cpu.regX = testcase.regX;
		cpu.regY = testcase.regY;
		cpu.regPC = 0x0123;
		cpu.cFlag = testcase.flagsIn.find('c') != std::string::npos;
		cpu.decimalMode = testcase.flagsIn.find('d') != std::string::npos;
		cpu.nFlag = testcase.flagsIn.find('n') != std::string::npos;
		cpu.vFlag = testcase.flagsIn.find('v') != std::string::npos;
		cpu.zFlag = testcase.flagsIn.find('z') != std::string::npos;
		cpu.step();

		BOOST_CHECK_EQUAL(cpu.regA, testcase.expectedRegA);
		BOOST_CHECK_EQUAL(cpu.regX, testcase.expectedRegX);
		BOOST_CHECK_EQUAL(cpu.regY, testcase.expectedRegY);
		BOOST_CHECK_EQUAL(ram.bytes[0x2345], testcase.value2);
		BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
		BOOST_CHECK_EQUAL(cpu.cycleCount, testcase.expectedCycles);
		BOOST_CHECK_EQUAL(cpu.cFlag, testcase.expectedFlags.find('c') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.decimalMode, testcase.expectedFlags.find('d') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.nFlag, testcase.expectedFlags.find('n') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.vFlag, testcase.expectedFlags.find('v') != std::string::npos);
		BOOST_CHECK_EQUAL(cpu.zFlag, testcase.expectedFlags.find('z') != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(test_asl_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x06;		// ASL $nn
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ASL $10");

	ram.bytes[0x10] = 0xC3;

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.cFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x11);
	BOOST_CHECK_EQUAL(ram.bytes[0x10], 0x86);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 5);
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);
	BOOST_CHECK(cpu.cFlag);

	ram.bytes[0x0123] = 0x0A;		// ASL A

	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ASL A");

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x22);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);

	ram.bytes[0x0123] = 0x0E;		// ASL $nnnn
	ram.bytes[0x0124] = 0x10;
	ram.bytes[0x0125] = 0x03;

	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ASL $0310");

	ram.bytes[0x310] = 0xC3;

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x11);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x126);
	BOOST_CHECK_EQUAL(ram.bytes[0x310], 0x86);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 6);
}

BOOST_AUTO_TEST_CASE(test_lsr_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x46;		// LSR $nn
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "LSR $10");

	ram.bytes[0x10] = 0xC3;

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.cFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x11);
	BOOST_CHECK_EQUAL(ram.bytes[0x10], 0x61);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 5);
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);
	BOOST_CHECK(cpu.cFlag);

	ram.bytes[0x0123] = 0x4A;		// LSR A

	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "LSR A");

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x08);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);
}

BOOST_AUTO_TEST_CASE(test_rol_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x26;		// ROL $nn
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ROL $10");

	ram.bytes[0x10] = 0xC3;
	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.cFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x11);
	BOOST_CHECK_EQUAL(ram.bytes[0x10], 0x87);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 5);
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);
	BOOST_CHECK(cpu.cFlag);

	ram.bytes[0x10] = 0xC3;
	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.cFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x11);
	BOOST_CHECK_EQUAL(ram.bytes[0x10], 0x86);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 5);
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);
	BOOST_CHECK(cpu.cFlag);
}

BOOST_AUTO_TEST_CASE(test_and_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x21;		// AND ($nn,X)
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "AND ($10, X)");

	ram.bytes[0x11] = 0x11;
	ram.bytes[0x12] = 0x12;
	ram.bytes[0x1211] = 0x13;

	cpu.cycleCount = 0;
	cpu.regA = 0x33;
	cpu.regX = 1;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x13);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 6);
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is negative
	cpu.regPC = 0x0123;
	cpu.regA = 0x80;
	ram.bytes[0x1211] = 0x83;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is zero
	cpu.regPC = 0x0123;
	cpu.regA = 0xFF;
	ram.bytes[0x1211] = 0x0;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);

	// Zero page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x25;		// AND $nn
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "AND $10");

	cpu.regPC = 0x0123;
	cpu.regA = 0x21;
	ram.bytes[0x10] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x20);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	// immediate page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x29;		// AND #nn
	ram.bytes[0x0124] = 0x44;
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "AND #44");

	cpu.regPC = 0x0123;
	cpu.regA = 0x51;
	ram.bytes[0x10] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x40);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);
	
}

BOOST_AUTO_TEST_CASE(test_or_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x01;		// ORA ($nn,X)
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ORA ($10, X)");

	ram.bytes[0x11] = 0x11;
	ram.bytes[0x12] = 0x12;
	ram.bytes[0x1211] = 0x13;

	cpu.cycleCount = 0;
	cpu.regA = 0x20;
	cpu.regX = 1;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x33);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 6);
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is negative
	cpu.regPC = 0x0123;
	cpu.regA = 0x20;
	ram.bytes[0x1211] = 0x83;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is zero
	cpu.regPC = 0x0123;
	cpu.regA = 0x0;
	ram.bytes[0x1211] = 0x0;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);

	// Zero page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x05;		// ORA $nn
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ORA $10");

	cpu.regPC = 0x0123;
	cpu.regA = 0x11;
	ram.bytes[0x10] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x33);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	// immediate page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x09;		// ORA #nn
	ram.bytes[0x0124] = 0x44;
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "ORA #44");

	cpu.regPC = 0x0123;
	cpu.regA = 0x11;
	ram.bytes[0x10] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x55);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);
}

BOOST_AUTO_TEST_CASE(test_eor_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x41;		// EOR ($nn,X)
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "EOR ($10, X)");

	ram.bytes[0x11] = 0x11;
	ram.bytes[0x12] = 0x12;
	ram.bytes[0x1211] = 0x53;

	cpu.cycleCount = 0;
	cpu.regA = 0x60;
	cpu.regX = 1;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x33);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 6);
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is negative
	cpu.regPC = 0x0123;
	cpu.regA = 0x20;
	ram.bytes[0x1211] = 0x83;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is zero
	cpu.regPC = 0x0123;
	cpu.regA = 0x12;
	ram.bytes[0x1211] = 0x12;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);

	// Zero page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x45;		// EOR $nn
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "EOR $10");

	cpu.regPC = 0x0123;
	cpu.regA = 0x51;
	ram.bytes[0x10] = 0x62;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x33);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	// immediate page variant. Flag setting is common, so just test the value.
	ram.bytes[0x0123] = 0x49;		// EOR #nn
	ram.bytes[0x0124] = 0x44;
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "EOR #44");

	cpu.regPC = 0x0123;
	cpu.regA = 0x41;
	ram.bytes[0x10] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x05);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);

	ram.bytes[0x0123] = 0x5D;		// EOR nnnn, X
	ram.bytes[0x0124] = 0x02;
	ram.bytes[0x0125] = 0x01;
	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "EOR $0102, X");

	cpu.regPC = 0x0123;
	cpu.regX = 0x05;
	cpu.regA = 0x41;
	ram.bytes[0x0107] = 0x22;
	cpu.cycleCount = 0;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x63);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 4);
}

BOOST_AUTO_TEST_CASE(test_pla_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x68;		// PLA

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "PLA");

	cpu.cycleCount = 0;
	cpu.regSP = 0xEF;
	cpu.regPC = 0x0123;
	cpu.regA = 0xAB;
	cpu.zFlag = true;
	cpu.nFlag = false;
	ram.bytes[0x1F0] = 0xB3;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0xB3);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 4);
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	cpu.cycleCount = 0;
	cpu.regSP = 0xEF;
	cpu.regPC = 0x0123;
	cpu.regA = 0xAB;
	cpu.zFlag = false;
	cpu.nFlag = true;
	ram.bytes[0x1F0] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x00);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 4);
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);
}

BOOST_AUTO_TEST_CASE(test_pha_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x48;		// PHA

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "PHA");

	cpu.cycleCount = 0;
	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.regA = 0xAB;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xAB);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);
}

BOOST_AUTO_TEST_CASE(test_php_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x08;		// PHP

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "PHP");

	cpu.cycleCount = 0;
	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;

	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.vFlag = true;
	cpu.cFlag = true;
	cpu.decimalMode = true;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xFF);
	BOOST_CHECK_EQUAL(cpu.regSP, 0xEF);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.vFlag = true;
	cpu.cFlag = true;
	cpu.decimalMode = true;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0x7F);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = false;
	cpu.vFlag = true;
	cpu.cFlag = true;
	cpu.decimalMode = true;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xFD);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.vFlag = false;
	cpu.cFlag = true;
	cpu.decimalMode = true;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xBF);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.vFlag = true;
	cpu.cFlag = false;
	cpu.decimalMode = true;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xFE);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.vFlag = true;
	cpu.cFlag = true;
	cpu.decimalMode = false;
	cpu.interruptDisable = true;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xF7);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = true;
	cpu.vFlag = true;
	cpu.cFlag = true;
	cpu.decimalMode = true;
	cpu.interruptDisable = false;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0xFB);
}

BOOST_AUTO_TEST_CASE(test_plp_instructions)
{
	// Given that PHP works, verifying round trip behaviour of PLP is sufficient.
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x28;		// PLP
	ram.bytes[0x0124] = 0x08;		// PHP

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "PLP");

	cpu.cycleCount = 0;
	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;

	ram.bytes[0x1F1] = 0xFF;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regSP, 0xF1);
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 4);
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xFF);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0x7F;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0x7F);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0xBF;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xBF);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0x30);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0xF7;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xF7);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0xFB;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xFB);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0xFD;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xFD);

	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;
	ram.bytes[0x1F1] = 0xFE;
	cpu.step();
	ram.bytes[0x1F1] = 0x00;
	cpu.step();
	BOOST_CHECK_EQUAL(ram.bytes[0x1F1], 0xFE);
}

BOOST_AUTO_TEST_CASE(test_clc_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x18;		// CLC

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "CLC");

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.cFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);
	BOOST_CHECK_EQUAL(cpu.cFlag, false);
}

BOOST_AUTO_TEST_CASE(test_sec_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x38;		// SEC

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "SEC");

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.cFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0124);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);
	BOOST_CHECK_EQUAL(cpu.cFlag, true);
}

BOOST_AUTO_TEST_CASE(test_jsr_rts_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x20;		// JSR $0201
	ram.bytes[0x0124] = 0x01;
	ram.bytes[0x0125] = 0x02;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "JSR $0201");

	cpu.cycleCount = 0;
	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;

	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0201);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 6);
	BOOST_CHECK_EQUAL(ram.bytes[0x1F0], 0x01);
	BOOST_CHECK_EQUAL(ram.bytes[0x1EF], 0x25);

	ram.bytes[0x0201] = 0x60;
	desc = cpu.disassemble(cpu.regPC, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "RTS");
	cpu.step();
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0126);
}

BOOST_AUTO_TEST_CASE(test_jmp_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x4C;		// JMP $0201
	ram.bytes[0x0124] = 0x01;
	ram.bytes[0x0125] = 0x02;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "JMP $0201");

	cpu.cycleCount = 0;
	cpu.regSP = 0xF0;
	cpu.regPC = 0x0123;

	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0201);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);
	BOOST_CHECK_EQUAL(cpu.regSP, 0xF0);
}

BOOST_AUTO_TEST_CASE(test_bit_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x24;		// BIT $nn
	ram.bytes[0x0124] = 0x10;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "BIT $10");

	ram.bytes[0x10] = 0xC3;

	cpu.cycleCount = 0;
	cpu.regA = 0x11;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.vFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0125);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);
	BOOST_CHECK(cpu.vFlag);

	ram.bytes[0x10] = 0x43;
	cpu.cycleCount = 0;
	cpu.regA = 0x10;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = false;
	cpu.vFlag = false;
	cpu.step();
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);
	BOOST_CHECK(cpu.vFlag);

	ram.bytes[0x10] = 0x23;
	cpu.cycleCount = 0;
	cpu.regA = 0x10;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.zFlag = false;
	cpu.vFlag = true;
	cpu.step();
	BOOST_CHECK(!cpu.nFlag);
	BOOST_CHECK(cpu.zFlag);
	BOOST_CHECK(!cpu.vFlag);
}

BOOST_AUTO_TEST_CASE(test_branch_instructions)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);
	DebugInfo debugInfo;

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x10;		// BPL #FC
	ram.bytes[0x0124] = 0xFC;

	auto desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "BPL *-4   -> 0x121");

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x121);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	ram.bytes[0x0124] = -0x30;
	cpu.nFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x0F5);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 4);

	ram.bytes[0x0123] = 0x30;		// BMI #FC
	ram.bytes[0x0124] = 0xFC;

	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "BMI *-4   -> 0x121");

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.nFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.nFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x121);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);

	ram.bytes[0x0123] = 0x50;		// BVC #FC
	ram.bytes[0x0124] = 0xFC;

	desc = cpu.disassemble(0x123, debugInfo);
	BOOST_CHECK_EQUAL(desc.line, "BVC *-4   -> 0x121");

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.vFlag = true;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x125);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 2);

	cpu.cycleCount = 0;
	cpu.regPC = 0x0123;
	cpu.vFlag = false;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regPC, 0x121);
	BOOST_CHECK_EQUAL(cpu.cycleCount, 3);
}
