#include "cpu/z80.h"
#include "ram.h"


#include <iostream>

#include "rom_loader.h"

using namespace std;


int main(int argc, char* argv[])
{
	Ram ram(64 * 1024);

	string romBase("C:\\emu\\sam\\roms\\1942");

	RomLoader romLoader(romBase);
	romLoader.load("srb-03.m3", 0x00000, 0x4000, ram);
	// ROM_LOAD("srb-03.m3", 0x00000, 0x4000, CRC(d9dafcc3) SHA1(a089a9bc55fb7d6d0ac53f91b258396d5d62677a))

	z80 cpu;
	auto pc = 0;
	for (auto i = 0; i < 10; ++i) {
		auto desc = cpu.disassemble(pc);
		std::cout << desc.line << "\n";
		pc += desc.numBytes;
	}
}
