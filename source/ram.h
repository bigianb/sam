#pragma once

#include <memory>

class Ram
{
public:
	Ram(int size);

	std::shared_ptr<unsigned char> bytes;
};
