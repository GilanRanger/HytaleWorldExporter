#pragma once
#include "Vec.h"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#define STANDARD_TILE_SIZE 32

struct Vertex {
	Vec3 position;
	Vec2 uv;
	Vec3 normal;
};

// Can be quad or triangle
struct MeshFace {
	uint32_t indices[4];
	uint8_t vertexCount;
	std::string material;

	MeshFace() : vertexCount(4), material("") {
		indices[0] = indices[1] = indices[2] = indices[3] = 0;
	}
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<MeshFace> faces;
	std::string materialName;

	Vec3 minBounds;
	Vec3 maxBounds;

	inline uint32_t addVertex(const Vertex& v) {
		vertices.push_back(v);
		return static_cast<uint32_t>(vertices.size() - 1);
	}

	inline void addFace(const MeshFace& f) {
		faces.push_back(f);
	}

	inline void clear() {
		vertices.clear();
		faces.clear();
	}
};