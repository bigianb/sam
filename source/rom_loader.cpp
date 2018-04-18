#include "rom_loader.h"
#include "ram.h"
#include <fstream>
#include <stdexcept>

void RomLoader::load(std::string filename, int startAddr, int bytesToLoad, Ram& ram)
{
	if (startAddr + bytesToLoad > (int)ram.bytes.size()) {
		throw std::length_error("loading too many bytes");
	}

	std::ifstream infile;
	std::string path = base + '/' + filename;
	infile.open(path, std::ios::binary | std::ios::in);
	if (infile.is_open()) {
		unsigned char* dest = ram.bytes.data() + startAddr;
		infile.read((char*)dest, bytesToLoad);
	}
	else {
		throw std::runtime_error("failed to open ROM file");
	}
}
