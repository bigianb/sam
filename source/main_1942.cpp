#include "cpu/z80.h"
#include "ram.h"

int main(int argc, char* argv[])
{
	Ram ram(64 * 1024);

	std::string romBase("C:\\emu\\sam\\roms\\1942");

	// ROM_LOAD("srb-03.m3", 0x00000, 0x4000, CRC(d9dafcc3) SHA1(a089a9bc55fb7d6d0ac53f91b258396d5d62677a))
}
