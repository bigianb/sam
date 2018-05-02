#include "debug_info.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

void DebugInfo::read(std::string filename)
{
    namespace pt = boost::property_tree;
    pt::ptree tree;
    pt::read_json(filename, tree);

    pt::ptree range_node = tree.get_child("ranges");
    
    for (const std::pair<std::string, pt::ptree> &range_item : range_node){
		Range rangeItem;
        for (const std::pair<std::string, pt::ptree> &kv : range_item.second){          
            std::string value = kv.second.get_value<std::string>();
            if (kv.first == "start"){
				std::sscanf(value.c_str(), "0x%x", &rangeItem.start);
            } else if (kv.first == "end"){
				std::sscanf(value.c_str(), "0x%x", &rangeItem.end);
            } else if (kv.first == "type"){
                if (value == "code"){
                    rangeItem.type = RangeType::eCODE;
                } else if (value == "data"){
                    rangeItem.type = RangeType::eDATA;
                } else {
                    // TODO: throw exception
                    std::cerr << "unknown type: " << value << std::endl;
                }
            } else {
                // TODO: throw exception
                std::cerr << "unknown key: " << kv.first << std::endl;
            }   
        }
		ranges.push_back(rangeItem);
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
