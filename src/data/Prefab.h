#pragma once
#include "Vec.h"
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

struct PrefabBlock {
    int x, y, z;
    std::string name;
    uint16_t rotation;
    uint16_t filler;
    std::unordered_map<std::string, std::string> components;

    PrefabBlock() : x(0), y(0), z(0), rotation(0), filler(0) {}
};

struct PrefabFluid {
    int x, y, z;
    std::string name;
    uint8_t level;

    PrefabFluid() : x(0), y(0), z(0), level(0) {}
};

struct Prefab {
    int version;
    int blockIdVersion;
    Vec3 anchor;
    std::vector<PrefabBlock> blocks;
    std::vector<PrefabFluid> fluids;
    std::string name;

    Prefab() : version(0), blockIdVersion(0), anchor(0, 0, 0) {}

    Vec3 getMinBounds() const {
        if (blocks.empty()) return Vec3(0, 0, 0);

        Vec3 min(blocks[0].x, blocks[0].y, blocks[0].z);
        for (const auto& block : blocks) {
            if (block.x < min.x) min.x = static_cast<float>(block.x);
            if (block.y < min.y) min.y = static_cast<float>(block.y);
            if (block.z < min.z) min.z = static_cast<float>(block.z);
        }
        return min;
    }

    Vec3 getMaxBounds() const {
        if (blocks.empty()) return Vec3(0, 0, 0);

        Vec3 max(blocks[0].x, blocks[0].y, blocks[0].z);
        for (const auto& block : blocks) {
            if (block.x > max.x) max.x = static_cast<float>(block.x);
            if (block.y > max.y) max.y = static_cast<float>(block.y);
            if (block.z > max.z) max.z = static_cast<float>(block.z);
        }
        return max;
    }

    Vec3 getSize() const {
        Vec3 min = getMinBounds();
        Vec3 max = getMaxBounds();
        return Vec3(max.x - min.x + 1, max.y - min.y + 1, max.z - min.z + 1);
    }

    std::unordered_set<std::string> getUniqueBlockTypes() const {
        std::unordered_set<std::string> uniqueTypes;
        for (const auto& block : blocks) {
            uniqueTypes.insert(block.name);
        }
        return uniqueTypes;
    }
};