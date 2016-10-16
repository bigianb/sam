#include "ram.h"

Ram::Ram(int size)
{
	bytes = std::make_shared<unsigned char>(size);
}
