#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#define STANDARD_TILE_SIZE 32

struct Vec4 {
	float x, y, z, w;

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

struct Vec3 {
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3& operator+=(const Vec3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
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