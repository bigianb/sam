#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>

class DebugInfo
{
  public:
    void read(std::string filename);

    enum class RangeType
    {
        eCODE,
        eDATA,
		eUNKNOWN
    };

    class Range
    {
      public:
        unsigned int start;
        unsigned int end;
        RangeType type;
    };

    const std::vector<Range> &getRanges() const { return ranges; }

	RangeType getType(unsigned int address);

	std::string getFunctionName(unsigned int address);

  private:
	void readRanges(boost::property_tree::ptree range_node);
	void readFunctions(boost::property_tree::ptree functions_node);

    std::vector<Range> ranges;

	std::map<unsigned int, std::string> functionNameMap;
};
