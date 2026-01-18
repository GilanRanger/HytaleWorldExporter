#pragma once
#include "../data/Prefab.h"
#include <string>
#include <memory>

class PrefabLoader {
public:
	static std::unique_ptr<Prefab> loadFromFile(const std::string& filepath);
	static std::unique_ptr<Prefab> loadFromJson(const std::string& jsonData);
};