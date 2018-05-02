#pragma once

#include <string>
#include <vector>

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

  private:
    std::vector<Range> ranges;
};
