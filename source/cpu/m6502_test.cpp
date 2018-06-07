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

	ram.bytes[0x11] = 0x13;

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
	ram.bytes[0x11] = 0x83;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is zero
	cpu.regPC = 0x0123;
	cpu.regA = 0xFF;
	ram.bytes[0x11] = 0x0;
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

	ram.bytes[0x11] = 0x13;

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
	ram.bytes[0x11] = 0x83;
	cpu.nFlag = false;
	cpu.zFlag = true;
	cpu.step();
	BOOST_CHECK(cpu.nFlag);
	BOOST_CHECK(!cpu.zFlag);

	// result is zero
	cpu.regPC = 0x0123;
	cpu.regA = 0x0;
	ram.bytes[0x11] = 0x0;
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

BOOST_AUTO_TEST_CASE(test_jsr_instructions)
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
}
