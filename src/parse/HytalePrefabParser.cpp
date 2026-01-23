#include "HytalePrefabParser.h"
#include "json/json.hpp"
#include <fstream>
#include <sstream>

using json = nlohmann::json;

std::unique_ptr<Prefab> PrefabLoader::loadFromFile(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		return nullptr;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return loadFromJson(buffer.str());
}

std::unique_ptr<Prefab> PrefabLoader::loadFromJson(const std::string& jsonData) {
    try {
        json j = json::parse(jsonData);

        auto prefab = std::make_unique<Prefab>();

        if (j.contains("version")) {
            prefab->version = j["version"].get<int>();
        }

        if (j.contains("blockIdVersion")) {
            prefab->blockIdVersion = j["blockIdVersion"].get<int>();
        }

        if (j.contains("anchorX") && j.contains("anchorY") && j.contains("anchorZ")) {
            prefab->anchor.x = j["anchorX"].get<float>();
            prefab->anchor.y = j["anchorY"].get<float>();
            prefab->anchor.z = j["anchorZ"].get<float>();
        }

        if (j.contains("blocks") && j["blocks"].is_array()) {
            for (const auto& blockJson : j["blocks"]) {

                // Skip filler blocks (other blocks in multi-block models)
                if (blockJson.contains("filler")) {
                    continue;
                }

                PrefabBlock block;

                if (blockJson.contains("x")) block.x = blockJson["x"].get<int>();
                if (blockJson.contains("y")) block.y = blockJson["y"].get<int>();
                if (blockJson.contains("z")) block.z = blockJson["z"].get<int>();
                if (blockJson.contains("name")) block.name = blockJson["name"].get<std::string>();

                if (blockJson.contains("rotation")) {
                    block.rotation = blockJson["rotation"].get<uint16_t>();
                }

                if (blockJson.contains("components") && blockJson["components"].is_object()) {
                    for (auto it = blockJson["components"].begin(); it != blockJson["components"].end(); ++it) {
                        block.components[it.key()] = it.value().dump();
                    }
                }

                prefab->blocks.push_back(block);
            }
        }

        if (j.contains("fluids") && j["fluids"].is_array()) {
            for (const auto& fluidJson : j["fluids"]) {
                PrefabFluid fluid;

                if (fluidJson.contains("x")) fluid.x = fluidJson["x"].get<int>();
                if (fluidJson.contains("y")) fluid.y = fluidJson["y"].get<int>();
                if (fluidJson.contains("z")) fluid.z = fluidJson["z"].get<int>();
                if (fluidJson.contains("name")) fluid.name = fluidJson["name"].get<std::string>();
                if (fluidJson.contains("level")) fluid.level = fluidJson["level"].get<uint8_t>();

                prefab->fluids.push_back(fluid);
            }
        }

        return prefab;

    }
    catch (const json::exception& e) {
        return nullptr;
    }
}