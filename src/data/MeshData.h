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