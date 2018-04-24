#include "cpu/m6502.h"
#include "ram.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>

using namespace std;

#include "rom_loader.h"
#include "direct_address_bus.h"


int main(int argc, char* argv[])
{
	Ram ram(64 * 1024);

	//string romBase("C:\\emu\\sam\\roms\\sidetrac");
	//string romBase("/Users/ian/roms/mame/sidetrac");
	string romBase("/home/ian/roms/mame/sidetrac");

	RomLoader romLoader(romBase);
	romLoader.load("stl8a-1", 0x2800, 0x0800, ram);
	romLoader.load("stl7a-2", 0x3000, 0x0800, ram);
	romLoader.load("stl6a-2", 0x3800, 0x0800, ram);
	romLoader.load("stl9c-1", 0x4800, 0x0400, ram);

	DirectAddressBus bus(ram);

	m6502 cpu(bus);
	
	// 0x3f00 is mirrored to 0xFF00
	auto pc = bus.readByte(0x3FFD) * 256 + bus.readByte(0x3FFC);
	for (auto i = 0; i < 400; ++i) {
		auto desc = cpu.disassemble(pc);
		std::cout << std::setw(4) << std::setfill('0') << std::hex << pc << " : ";
		std::cout << desc.line << std::endl;
		pc += desc.numBytes;
	}

	return 0;
}
/*
ROM_START(sidetrac)
ROM_REGION(0x10000, "maincpu", 0)
ROM_LOAD("stl8a-1", 0x2800, 0x0800, CRC(e41750ff) SHA1(3868a0d7e34a5118b39b31cff9e4fc839df541ff))
ROM_LOAD("stl7a-2", 0x3000, 0x0800, CRC(57fb28dc) SHA1(6addd633d655d6a56b3e509d18e5f7c0ab2d0fbb))
ROM_LOAD("stl6a-2", 0x3800, 0x0800, CRC(4226d469) SHA1(fd18b732b66082988b01e04adc2b1e5dae410c98))
ROM_LOAD("stl9c-1", 0x4800, 0x0400, CRC(08710a84) SHA1(4bff254a14af7c968656ccc85277d31ab5a8f0c4)) 

ROM_REGION(0x0200, "gfx1", 0)
ROM_LOAD("stl11d", 0x0000, 0x0200, CRC(3bd1acc1) SHA1(06f900cb8f56cd4215c5fbf58a852426d390e0c1))
ROM_END
*/
