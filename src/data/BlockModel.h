#pragma once
#include "MeshData.h"
#include "WorldData.h"
#include <string>
#include <vector>
#include <unordered_map>

struct BlockModel {
	std::string modelName;

	// Key: face name ("north", "south", "east", "west", "top", "bottom")
	std::unordered_map < std::string, std::string> faceTextures;

	bool isFullCube;
	Vec3 boundsMin;
	Vec3 boundsMax;

	enum class RenderPass {
		Opaque,
		Transparent,
		Cutout,
		Liquid
	};

	BlockModel() : isFullCube(true),
		boundsMin(0, 0, 0),
		boundsMax(1, 1, 1) {}
};

class BlockModelRegistry {
private:
	std::unordered_map<std::string, BlockModel> models;

public:
	BlockModelRegistry();

	BlockModel* loadModel(const std::string& modelName);

	BlockModel* getModel(const std::string& modelName);

	bool hasModel(const std::string& modelName) const;

	void createDefaultCubeModel();
};

// UV bounds for a texture within an atlas
struct AtlasRegion {
	Vec2 uvMin;     // Top-left UV coordinate (0.0 - 1.0)
	Vec2 uvMax;     // Bottom-right UV coordinate (0.0 - 1.0)
	uint32_t pixelWidth;
	uint32_t pixelHeight;
};

class TextureRegistry {
private:
	std::unordered_map<std::string, AtlasRegion> textureRegions;
	uint32_t atlasWidth, atlastHeight;
	uint32_t standardTileSize;

	std::vector<uint8_t> pixelData;
	bool isBuilding;
public:
	void beginBuild(uint32_t width, uint32_t height, uint32_t tileSize);
	void addTexture(const std::string& name, const std::string& filepath);
	void packTextures();
	void exportAtlas(const std::string& outputPath);
	void endBuild();

	bool getTextureUVs(const std::string& name, Vec2& uvMin, Vec2& uvMax) const;
	void loadMetadata(const std::string& metadataPath);
	void saveMetadata(const std::string& metadataPath) const;

	bool isBuilding() const { return isBuilding; }
};

class BlockIDMappings {
private:
	std::unordered_map<PackedBlock, std::string> blockNames;
public:
	BlockIDMappings();
	
	void parseBlockList(const std::string& data);

	std::string getBlockName(PackedBlock blockId);
};