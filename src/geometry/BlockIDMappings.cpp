#include "../data/Model.h"

BlockIDMappings::BlockIDMappings() {}

uint32_t readVarint(const std::string& data, size_t& pos) {
	uint32_t result = 0;
	int shift = 0;
	while (pos < data.size()) {
		uint8_t byte = static_cast<uint8_t>(data[pos]);
		pos++;
		result |= (byte & 0x7F) << shift;
		if ((byte & 0x80) == 0) break;
		shift += 7;
	}
	return result;
}

void BlockIDMappings::parseBlockList(const std::string& data) {
	std::unordered_map<PackedBlock, std::string> list;
	size_t pos = 0;

	while (pos < data.size()) {
		if (static_cast<uint8_t>(data[pos]) != 0x0A) {
			break;
		}
		pos++;

		uint32_t entryLen = readVarint(data, pos);
		size_t entryEnd = pos + entryLen;

		uint32_t blockId = 0;
		std::string blockName;

		while (pos < entryEnd) {
			uint8_t tag = static_cast<uint8_t>(data[pos]);
			pos++;

			if (tag == 0x08) {
				blockId = readVarint(data, pos);
			}
			else if (tag == 0x12) {
				uint32_t strLen = readVarint(data, pos);
				blockName = data.substr(pos, strLen);
				pos += strLen;
			}
		}

		list[blockId] = blockName;
	}

	blockNames = list;
}

std::string BlockIDMappings::getBlockName(PackedBlock blockId) {
	return "";
}