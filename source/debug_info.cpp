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
	pt::ptree ports_node = tree.get_child("ports");
	readPorts(ports_node);
	pt::ptree comments_node = tree.get_child("comments");
	readComments(comments_node);
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

void DebugInfo::readPorts(pt::ptree ports_node)
{
	for (const std::pair<std::string, pt::ptree> &portItem : ports_node) {
		
		const auto& addrNode = portItem.second.get_child("addr");
		const auto& nameNode = portItem.second.get_child("name");
		const auto& dirNode = portItem.second.get_child("dir");

		unsigned int addr;
		std::sscanf(addrNode.get_value<std::string>().c_str(), "0x%x", &addr);

		std::string name = nameNode.get_value<std::string>();
		std::string dir = dirNode.get_value<std::string>();

		if (dir.find('r') != std::string::npos){
			portReadMap[addr] = name;
		}
		if (dir.find('w') != std::string::npos){
			portWriteMap[addr] = name;
		}
	}
}

void DebugInfo::readComments(pt::ptree comments_node)
{
	for (const std::pair<std::string, pt::ptree> &commentItem : comments_node) {
		unsigned int addr;
		std::sscanf(commentItem.first.c_str(), "0x%x", &addr);
		commentsMap[addr] = commentItem.second.get_value<std::string>();
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

std::string DebugInfo::getReadPortName(unsigned int address)
{
	auto it = portReadMap.find(address);
	std::string name;
	if (it != portReadMap.end())
	{
		name = it->second;
	}
	return name;
}

std::string DebugInfo::getWritePortName(unsigned int address)
{
	auto it = portWriteMap.find(address);
	std::string name;
	if (it != portWriteMap.end())
	{
		name = it->second;
	}
	return name;
}

std::string DebugInfo::getComment(unsigned int address)
{
	auto it = commentsMap.find(address);
	std::string name;
	if (it != commentsMap.end())
	{
		name = it->second;
	}
	return name;
}