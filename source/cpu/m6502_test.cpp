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

BOOST_AUTO_TEST_CASE(test_opcodes)
{
	Ram ram(64 * 1024);
	DirectAddressBus bus(ram);

	for (int i = 0; i < 0xFFFF; ++i) {
		ram.bytes[i] = 0;
	}

	m6502 cpu(bus);
	cpu.reset();

	ram.bytes[0x0123] = 0x01;		// ORA ($nn,X)
	ram.bytes[0x0124] = 0x10;

	ram.bytes[0x11] = 0x13;

	cpu.regA = 0x20;
	cpu.regX = 1;
	cpu.regPC = 0x0123;
	cpu.step();
	BOOST_CHECK_EQUAL(cpu.regA, 0x33);
}
