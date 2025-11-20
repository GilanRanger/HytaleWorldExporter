#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#define STANDARD_TILE_SIZE 32

struct Vec3 {
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Vec2 {
	float u, v;

	Vec2() : u(0), v(0) {}
	Vec2(float u, float v) : u(u), v(v) {}
};

struct Vertex {
	Vec3 position;
	Vec2 uv;
	Vec3 normal;
};

// Can be quad or triangle
struct Face {
	uint32_t indices[4];
	uint8_t vertexCount;
	std::string material;

	Face() : vertexCount(4), material("") {
		indices[0] = indices[1] = indices[2] = indices[3] = 0;
	}
};

// UV bounds for a texture within an atlas
struct AtlasRegion {
    Vec2 uvMin;     // Top-left UV coordinate (0.0 - 1.0)
    Vec2 uvMax;     // Bottom-right UV coordinate (0.0 - 1.0)
    uint32_t pixelWidth;
    uint32_t pixelHeight;
};

// Texture atlas information
struct TextureAtlas {
    uint32_t atlasWidth;
    uint32_t atlasHeight;
    uint32_t standardTileSize;  // Standard block texture size 32x32

    std::vector<std::string> textureNames;

    std::unordered_map<std::string, AtlasRegion> textureRegions;

    inline bool getTextureUVs(const std::string& textureName, Vec2& uvMin, Vec2& uvMax) const {
        auto it = textureRegions.find(textureName);
        if (it != textureRegions.end()) {
            uvMin = it->second.uvMin;
            uvMax = it->second.uvMax;
            return true;
        }
        return false;
    }

    inline void addTextureRegion(const std::string& name,
        uint32_t pixelX, uint32_t pixelY,
        uint32_t width, uint32_t height) {
        AtlasRegion region;
        region.uvMin.u = static_cast<float>(pixelX) / atlasWidth;
        region.uvMin.v = static_cast<float>(pixelY) / atlasHeight;
        region.uvMax.u = static_cast<float>(pixelX + width) / atlasWidth;
        region.uvMax.v = static_cast<float>(pixelY + height) / atlasHeight;
        region.pixelWidth = width;
        region.pixelHeight = height;

        textureRegions[name] = region;
    }
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	std::string materialName;

	Vec3 minBounds;
	Vec3 maxBounds;

	inline uint32_t addVertex(const Vertex& v) {
		vertices.push_back(v);
		return static_cast<uint32_t>(vertices.size() - 1);
	}

	inline void addFace(const Face& f) {
		faces.push_back(f);
	}

	inline void clear() {
		vertices.clear();
		faces.clear();
	}
};