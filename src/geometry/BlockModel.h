#pragma once
#include "../data/MeshData.h"
#include <string>
#include <vector>
#include <unordered_map>

struct BlockModel {
	std::string modelName;

	Mesh mesh;

	// Key: face name ("north", "south", "east", "west", "top", "bottom")
	std::unordered_map < std::string, std::string> faceTextures;

	bool isFullCube;

	Vec3 boundsMin;
	Vec3 boundsMax;

	BlockModel() : isFullCube(true),
		boundsMin(0, 0, 0),
		boundsMax(1, 1, 1) {}
};

class BlockModelRegistry {
private:
	std::unordered_map<std::string, BlockModel> models;

public:
	BlockModel* loadModel(const std::string& modelName);

	BlockModel* getModel(const std::string& modelName);

	bool hasModel(const std::string& modelName) const;

	void createDefaultCubeModel();
};