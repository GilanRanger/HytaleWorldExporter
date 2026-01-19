#pragma once
#include "MeshData.h"
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>

class NodeNameManager;

enum class ShadingMode {
	Standard = 0,
	Flat = 1,
	Fullbright = 2,
	Reflective = 3
};

struct ModelFaceTextureLayout {
	Vec2 offset;
	int angle;
	bool mirrorX;
	bool mirrorY;
	bool hidden;

	ModelFaceTextureLayout()
		: offset(0, 0), angle(0), mirrorX(false), mirrorY(false), hidden(false) {}
};

struct ModelNode {
	enum class ShapeType {
		None = 0,
		Box = 1,
		Quad = 2
	};

	enum class QuadNormal {
		PlusZ = 0,
		MinusZ = 1,
		PlusX = 2,
		MinusX = 3,
		PlusY = 4,
		MinusY = 5
	};

	int nameId;
	std::vector<int> children;

	// Transform
	Vec3 position;
	Vec4 orientation;
	Vec3 offset;
	Vec3 stretch;

	Vec3 proceduralOffset;
	Vec3 proceduralRotation;

	// Shape
	ShapeType type;
	Vec3 size;
	QuadNormal quadNormalDirection;

	// Texture
	ModelFaceTextureLayout* textureLayout;
	int textureLayoutSize;
	uint8_t atlasIndex;
	uint8_t gradientId;
	ShadingMode shadingMode;

	// Flags
	bool visible;
	bool doubleSided;
	bool isPiece;

	ModelNode()
		: nameId(-1),
		position(0, 0, 0), orientation(Vec4::Identity()),
		offset(0, 0, 0), stretch(1, 1, 1),
		proceduralOffset(0, 0, 0), proceduralRotation(0, 0, 0),
		type(ShapeType::None), size(0, 0, 0),
		quadNormalDirection(QuadNormal::PlusZ),
		textureLayout(nullptr), textureLayoutSize(0),
		atlasIndex(0), gradientId(0), shadingMode(ShadingMode::Standard),
		visible(true), doubleSided(false), isPiece(false) {
	}

	~ModelNode() {
		delete[] textureLayout;
	}

	ModelNode clone() const;
};

struct Model {
	static constexpr int EmptyNodeNameId = -1;
	static constexpr int NodeGrowthAmount = 5;
	static int MaxNodeCount;

	std::vector<int> rootNodes;
	ModelNode* allNodes;
	std::unordered_map<int, int> nodeIndicesByNameId;
	int* parentNodes;
	uint8_t gradientId;
	int nodeCount;

private:
	int allocatedNodeCount;

public:
	Model(int preAllocatedNodeCount = 256);
	~Model();

	void addNode(ModelNode& node, int parentNodeIndex = -1);


	Model clone() const;

	void attach(Model* attachment, NodeNameManager* nodeNameManager,
		uint8_t* atlasIndex = nullptr, Vec2* uvOffset = nullptr,
		int forcedTargetNodeNameId = -1);

	void setAtlasIndex(uint8_t atlasIndex);
	void setGradientId(uint8_t gradientId);
	void offsetUVs(Vec2 offset);

private:
	void ensureNodeCountAllocated(int required, int growth = 0);
	void recurseAttach(Model* attachment, ModelNode& attachmentNode,
		int parentNodeIndex, NodeNameManager* nodeNameManager,
		uint8_t* atlasIndex, Vec2* uvOffset, bool forcedAttachment);
};

struct FaceLayout {
	Vec2 offset;
	Vec2 mirror;
	int angle;

	FaceLayout() : offset(0, 0), mirror(0, 0), angle(0) {}
};

struct NodeShapeSettings {
	Vec3 size;
	std::string normal;
	bool isPiece;

	NodeShapeSettings() : size(0, 0, 0), isPiece(false) {}
};

struct NodeShape {
	bool visible;
	bool doubleSided;
	std::string shadingMode;
	std::string type;
	Vec3 offset;
	Vec3 stretch;
	NodeShapeSettings settings;
	std::unordered_map<std::string, FaceLayout> textureLayout;

	NodeShape() : visible(true), doubleSided(false),
		offset(0, 0, 0), stretch(1, 1, 1) {
	}
};

struct ModelNodeJson {
	std::string name;
	Vec3 position;
	Vec4 orientation;
	NodeShape shape;
	std::vector<ModelNodeJson> children;

	ModelNodeJson() : position(0, 0, 0), orientation(Vec4::Identity()) {}
};

struct ModelJson {
	std::vector<ModelNodeJson> nodes;
	std::string lod;

	ModelJson() {}
};

class NodeNameManager {
private:
	std::unordered_map<std::string, int> nameToId;
	std::unordered_map<int, std::string> idToName;
	int nextId;

public:
	NodeNameManager() : nextId(0) {}

	int getOrAddNameId(const std::string& name);
	bool tryGetNameFromId(int id, std::string& outName) const;
	bool tryGetIdFromName(const std::string& name, int& outId) const;
};

class ModelInitializer {
public:
	static void parse(const uint8_t* data, size_t dataSize,
		NodeNameManager* nodeNameManager, Model& blockyModel);
	static void parse(const ModelJson& json,
		NodeNameManager* nodeNameManager, Model& blockyModel);

private:
	static void recurseParseNode(ModelNodeJson& jsonNode, Model& model,
		int parentNodeIndex, NodeNameManager* nodeNameManager);
	static ShadingMode parseShadingMode(const std::string& shadingMode);
	static ModelNode::QuadNormal parseQuadNormal(const std::string& quadNormal);
	static ModelFaceTextureLayout getFaceLayout(
		const std::unordered_map<std::string, FaceLayout>& jsonTextureLayout,
		const std::string& faceName);
};

// UV bounds for a texture within an atlas
struct AtlasRegion {
	Vec2 uvMin;
	Vec2 uvMax;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
};

struct TextureSource {
	std::string name;
	uint8_t* data;
	uint32_t width;
	uint32_t height;
	uint32_t channels;
};

class TextureRegistry {
private:
	std::unordered_map<std::string, AtlasRegion> textureRegions;
	std::unordered_map<std::string, TextureSource> textureSources;
	uint32_t atlasWidth, atlasHeight;
	uint32_t standardTileSize;
	std::unique_ptr<uint8_t[]> pixelData;
	
	void copyTextureToAtlas(const uint8_t* srcData, uint32_t srcWidth, uint32_t srcHeight,
		uint32_t srcChannels, uint32_t dstX, uint32_t dstY);
public:
	TextureRegistry(uint32_t width, uint32_t height, uint32_t tileSize)
		: atlasWidth(width), atlasHeight(height), standardTileSize(tileSize),
		pixelData(std::make_unique<uint8_t[]>(width* height * 4)) {
		std::memset(pixelData.get(), 0, width * height * 4);
	}

	void addTexture(const std::string& name, const std::string& filepath);
	void packTextures();
	void exportAtlas(const std::string& outputPath);
	const AtlasRegion* getTextureRegion(const std::string& name) const;
};

class ModelRegistry {
private:
	std::string assetPath;
	std::unordered_map<std::string, Model*> models;
	NodeNameManager nodeNameManager;

public:
	ModelRegistry(const std::string& assetPath) : assetPath(assetPath) {}

	std::string findModelPath(const std::string& modelName);
	std::string findTexturePath(const std::string& modelName);

	Model* loadModel(const std::string& modelName);
	Model* getModel(const std::string& modelName);
	bool hasModel(const std::string& modelName) const;

	NodeNameManager* getNodeNameManager() { return &nodeNameManager; }
};