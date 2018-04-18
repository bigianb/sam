#pragma once
#include <string>
class Ram;

/*
Loads ROM images into memory.
*/
class RomLoader
{
public:
	RomLoader(std::string baseIn) : base(baseIn) {}

	void load(std::string filename, int startAddr, int bytesToLoad, Ram& ram);

private:
	std::string base;
};
