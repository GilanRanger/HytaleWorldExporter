#pragma once
#include "../data/WorldData.h"
#include "../data/MeshData.h"
#include "BlockModel.h"
#include <memory>

enum class FaceDirection : uint8_t {
    NORTH = 0,  // -Z
    SOUTH = 1,  // +Z
    EAST = 2,  // +X
    WEST = 3,  // -X
    TOP = 4,  // +Y
    BOTTOM = 5  // -Y
};

struct FaceOffset {
    int8_t x, y, z;
};

class GeometryGenerator {
private:
    BlockModelRegistry* modelRegistry;
    TextureAtlas* textureAtlas;

    static const FaceOffset FACE_OFFSETS[6];

    bool shouldRenderFace(const World* world, 
        int32_t blockX, int32_t blockY, int32_t blockZ, FaceDirection face) const;

    bool isBlockOpaque(BlockID blockId) const;

    void generateBlockFace(Mesh& outputMesh, const BlockModel& model, FaceDirection face,
        int32_t worldX, int32_t worldY, int32_t worldZ, BlockState state) const;

    Vec3 rotateVertex(const Vec3& vertex, BlockState state) const;

    Vec2 calculateUV(const std::string& textureName, float localU, float localV) const;

public:
    GeometryGenerator(BlockModelRegistry* registry, TextureAtlas* atlas);

    void generateChunkMesh(const World* world, const Chunk* chunk, Mesh& outputMesh);

    void generateBlockMesh(const World* world,
        int32_t blockX, int32_t blockY, int32_t blockZ, Mesh& outputMesh);
};