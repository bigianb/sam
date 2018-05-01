#pragma once

#include <string>
#include <vector>

class DebugSymbols
{
  public:
    void read(std::string filename);

    enum class RangeType
    {
        eCODE,
        eDATA
    };

    class Range
    {
      public:
        unsigned int start;
        unsigned int end;
        RangeType type;
    };

    const std::vector<Range> &getRanges() const { return ranges; }

  private:
    std::vector<Range> ranges;
};
