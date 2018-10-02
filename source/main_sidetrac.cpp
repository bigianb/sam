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
	bool dump;
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
	std::string romBase("/home/ian/roms/mame");
	std::string configBase("/home/ian/dev/sam/config");
#endif
	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
	("help,h", "Print help messages")
	("dump,d", "Dump disassembly")
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
		retval.dump = vm.count("dump");
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

void dump(std::ostream& os, int fromPC, int toPC, DebugInfo& debugInfo, m6502& cpu, AddressBus& bus)
{
	auto irq_vec = bus.readByte(0x3FFF) * 256 + bus.readByte(0x3FFE);

	os << "Initial PC: 0x" << std::setw(4) << std::setfill('0') << std::hex << fromPC << std::endl;
	os << "IRQ vector: 0x" << std::setw(4) << std::setfill('0') << std::hex << irq_vec << std::endl;

	int pc = fromPC;
	while (pc < toPC)
	{
		std::ostringstream stringStream;
		int pc_inc = 1;
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
			while (pos < 45) {
				stringStream << " ";
				++pos;
			}
			stringStream << "; " << comment;
		}
		os << stringStream.str() << std::endl;
		pc += pc_inc;
	}
}

/*
// https://www.arcade-museum.com/game_detail.php?game_id=9541
#define EXIDY_MASTER_CLOCK              (XTAL_11_289MHz)
#define EXIDY_CPU_CLOCK                 (EXIDY_MASTER_CLOCK / 16)
#define EXIDY_PIXEL_CLOCK               (EXIDY_MASTER_CLOCK / 2)

Total pixels = 0x150 * 0x118 = 16f80 = 94080 dec
pixel clock = 5644500 Hz
Rate = 59 Frames per second

#define EXIDY_HTOTAL                    (0x150)
#define EXIDY_HBEND                     (0x000)
#define EXIDY_HBSTART                   (0x100)
#define EXIDY_HSEND                     (0x140)
#define EXIDY_HSSTART                   (0x120)

#define EXIDY_VTOTAL                    (0x118)
#define EXIDY_VBEND                     (0x000)
#define EXIDY_VBSTART                   (0x100)
#define EXIDY_VSEND                     (0x108)
#define EXIDY_VSSTART                   (0x100)
*/

int runCpu(DebugInfo& debugInfo, m6502& cpu, AddressBus& bus)
{
	// There are 8 pixels per cpu cycle
	// There are 94080 pixels per frame and so there are 11,760 cycles per frame
	const int cyclesPerFrame = 11760;

	int framesToGo = 10;

	while (framesToGo > 0) {
		cpu.cycleCount = 0;
		while (cpu.cycleCount < cyclesPerFrame) {
			cpu.step();
			cpu.setIrqHigh();
		}
		cpu.setIrqLow();
		framesToGo -= 1;
	}
	return 0;
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
	bus.setMirror(0xFF00, 0xFFFF, 0x3F00);
	cpu.reset();

	auto pc = cpu.regPC;
	if (options.dump) {
		dump(std::cout, pc, 0x3aaa, debugInfo, cpu, bus);
	}
	return runCpu(debugInfo, cpu, bus);
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
