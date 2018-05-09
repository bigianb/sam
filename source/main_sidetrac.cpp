#include "cpu/m6502.h"
#include "ram.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "boost/program_options.hpp"

using namespace std;

#include "rom_loader.h"
#include "direct_address_bus.h"
#include "debug_info.h"

struct CLOptions
{
	CLOptions() : errorReported(false), allDone(false) {}

	bool errorReported;
	bool allDone;
	std::string rombase;
	std::string configbase;
};

static CLOptions parseCommandLineOptions(int argc, char *argv[])
{
	CLOptions retval;
#ifdef _WIN32
	std::string romBase("C:\\emu\\sam\\roms");
	std::string configBase("C:\\emu\\sam\\config");
#elif __APPLE__
	std::string romBase("/Users/ian/roms/mame");
	std::string configBase("/Users/ian/dev/sam/config");
#else
	string romBase("/home/ian/roms/mame");
#endif
	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
	("help,h", "Print help messages")
	("rombase", po::value<string>()->default_value(romBase), "Rom Base Directory")
	("configbase", po::value<string>()->default_value(configBase), "Config Base Directory");
	po::variables_map vm;

	try
	{
		// throws on error
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			retval.allDone = true;
			return retval;
		}
		retval.rombase = vm["rombase"].as<std::string>();
		retval.configbase = vm["configbase"].as<std::string>();
		po::notify(vm);
	}
	catch (boost::program_options::required_option &e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl
				  << std::endl;
		std::cout << desc << std::endl;
		retval.errorReported = true;
		return retval;
	}
	catch (boost::program_options::error &e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl
				  << std::endl;
		std::cout << desc << std::endl;
		retval.errorReported = true;
		return retval;
	}

	return retval;
}

int main(int argc, char *argv[])
{
	Ram ram(64 * 1024);

	CLOptions options = parseCommandLineOptions(argc, argv);
	if (options.allDone)
	{
		return 0;
	}
	if (options.errorReported)
	{
		return -1;
	}

	RomLoader romLoader(options.rombase + "/sidetrac");
	romLoader.load("stl8a-1", 0x2800, 0x0800, ram);
	romLoader.load("stl7a-2", 0x3000, 0x0800, ram);
	romLoader.load("stl6a-2", 0x3800, 0x0800, ram);
	romLoader.load("stl9c-1", 0x4800, 0x0400, ram);

	DirectAddressBus bus(ram);

	m6502 cpu(bus);

	DebugInfo debugInfo;
	debugInfo.read(options.configbase + "/sidetrac_symbols.json");

	// 0x3f00 is mirrored to 0xFF00
	auto pc = bus.readByte(0x3FFD) * 256 + bus.readByte(0x3FFC);
	auto irq_vec = bus.readByte(0x3FFF) * 256 + bus.readByte(0x3FFE);

	std::cout << "Initial PC: 0x" << std::setw(4) << std::setfill('0') << std::hex << pc << std::endl;
	std::cout << "IRQ vector: 0x" << std::setw(4) << std::setfill('0') << std::hex << irq_vec << std::endl;

	while(pc < 0x3aaa)
	{
		std::ostringstream stringStream;
		int pc_inc=1;
		if (debugInfo.getType(pc) == DebugInfo::RangeType::eCODE) {
			auto funcname = debugInfo.getFunctionName(pc);
			if (funcname != "") {
				stringStream << std::endl << funcname << std::endl;
			}
			stringStream << "    " << std::setw(4) << std::setfill('0') << std::hex << pc << ": ";
			auto desc = cpu.disassemble(pc, debugInfo);
			stringStream << desc.line;
			pc_inc = desc.numBytes;
		}
		else {
			stringStream << "    " << std::setw(4) << std::setfill('0') << std::hex << pc << ": ";
			uint8_t val = bus.readByte(pc);
			stringStream << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)val;
			if (val >= 0x20 && val < 0x80) {
				stringStream << "  " << val;
			}
		}
		auto comment = debugInfo.getComment(pc);
		if (comment != "") {
			auto pos = stringStream.str().length();
			while (pos < 45){
				stringStream << " ";
				++pos;
			}
			stringStream << "; " << comment;
		}
		std::cout << stringStream.str() << std::endl;
		pc += pc_inc;
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
