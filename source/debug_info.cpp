#include "debug_info.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

void DebugInfo::read(std::string filename)
{
    namespace pt = boost::property_tree;
    pt::ptree tree;
    pt::read_json(filename, tree);

    pt::ptree range_node = tree.get_child("ranges");
	readRanges(range_node);
	pt::ptree functions_node = tree.get_child("functions");
	readFunctions(functions_node);
}

void DebugInfo::readRanges(pt::ptree range_node)
{
	for (const std::pair<std::string, pt::ptree> &range_item : range_node) {
		Range rangeItem;
		for (const std::pair<std::string, pt::ptree> &kv : range_item.second) {
			std::string value = kv.second.get_value<std::string>();
			if (kv.first == "start") {
				std::sscanf(value.c_str(), "0x%x", &rangeItem.start);
			}
			else if (kv.first == "end") {
				std::sscanf(value.c_str(), "0x%x", &rangeItem.end);
			}
			else if (kv.first == "type") {
				if (value == "code") {
					rangeItem.type = RangeType::eCODE;
				}
				else if (value == "data") {
					rangeItem.type = RangeType::eDATA;
				}
				else {
					// TODO: throw exception
					std::cerr << "unknown type: " << value << std::endl;
				}
			}
			else {
				// TODO: throw exception
				std::cerr << "unknown key: " << kv.first << std::endl;
			}
		}
		ranges.push_back(rangeItem);
	}
}

void DebugInfo::readFunctions(pt::ptree function_node)
{
	for (const std::pair<std::string, pt::ptree> &functionItem : function_node) {
		for (const std::pair<std::string, pt::ptree> &kv : functionItem.second) {
			std::string value = kv.second.get_value<std::string>();
			if (kv.first == "name") {
				unsigned int addr;
				std::sscanf(functionItem.first.c_str(), "0x%x", &addr);
				functionNameMap[addr] = value;
			}
		}
	}
}

DebugInfo::RangeType DebugInfo::getType(unsigned int address)
{
	for (const Range& range: ranges) {
		if (range.start <= address && address <= range.end) {
			return range.type;
		}
	}
	return RangeType::eUNKNOWN;
}

std::string DebugInfo::getFunctionName(unsigned int address)
{
	auto it = functionNameMap.find(address);
	std::string name;
	if (it != functionNameMap.end())
	{
		name = it->second;
	}
	return name;
}
