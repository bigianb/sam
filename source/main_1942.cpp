#include "cpu/z80.h"
#include "ram.h"
#include <fstream>
#include <stdexcept>

using namespace std;

/*
	Loads ROM images into memory.
*/
class RomLoader
{
public:
	RomLoader(string baseIn) : base(baseIn) {}

	void load(string filename, int startAddr, int bytesToLoad, Ram& ram);

private:
	string base;
};

void RomLoader::load(string filename, int startAddr, int bytesToLoad, Ram& ram)
{
	if (startAddr + bytesToLoad > (int)ram.bytes.size()) {
		throw length_error("loading too many bytes");
	}

	ifstream infile;
	string path = base + '/' + filename;
	infile.open(path, ios::binary | ios::in);
	if (infile.is_open()) {
		unsigned char* dest = ram.bytes.data() + startAddr;
		infile.read((char*)dest, bytesToLoad);
	} else {
		throw runtime_error("failed to open ROM file");
	}
}

int main(int argc, char* argv[])
{
	Ram ram(64 * 1024);

	string romBase("C:\\emu\\sam\\roms\\1942");

	RomLoader romLoader(romBase);
	romLoader.load("srb-03.m3", 0x00000, 0x4000, ram);
	// ROM_LOAD("srb-03.m3", 0x00000, 0x4000, CRC(d9dafcc3) SHA1(a089a9bc55fb7d6d0ac53f91b258396d5d62677a))

	z80 cpu;
	auto pc = 0;
	for (auto i = 0; i < 10; +i) {
		auto desc = cpu.disassemble(pc);
		pc += desc.numBytes;
	}
}
