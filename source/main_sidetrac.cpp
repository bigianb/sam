#include "cpu/z80.h"
#include "ram.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

#include "rom_loader.h"


int main(int argc, char* argv[])
{
	Ram ram(64 * 1024);

	string romBase("C:\\emu\\sam\\roms\\sidetrac");

	RomLoader romLoader(romBase);
	romLoader.load("stl8a-1", 0x2800, 0x0800, ram);

	z80 cpu;
	auto pc = 0;
	for (auto i = 0; i < 10; ++i) {
		auto desc = cpu.disassemble(pc);
		std::cout << desc.line << "\n";
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
