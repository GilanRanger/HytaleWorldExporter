#pragma once
#include "../data/WorldData.h"
#include "../data/MeshData.h"
#include "../data/Model.h"
#include <memory>
#include <vector>

struct FaceOffset {
    int8_t x, y, z;
};

class MeshBuilder {
private:
    ModelRegistry* modelRegistry;
    TextureRegistry* textureRegistry;

    static const FaceOffset FACE_OFFSETS[6];

    // Face culling
    bool shouldRenderFace(const World* world,
        int32_t blockX, int32_t blockY, int32_t blockZ,
        ModelNode::QuadNormal face) const;
    bool isBlockOpaque(ResolvedBlock blockId) const;

    // Face generation
    void generateBlockFace(Mesh& outputMesh, const Model& model,
        ModelNode::QuadNormal face, int32_t worldX, int32_t worldY, int32_t worldZ,
        uint16_t state) const;
    void generateNodeFace(Mesh& outputMesh, const Model& model,
        const ModelNode& node, ModelNode::QuadNormal face,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const;

    // Transform utilities
    Mat4 calculateNodeTransform(const Model& model, const ModelNode& node) const;
    Vec3 transformPoint(const Mat4& matrix, const Vec3& point) const;
    Vec3 transformNormal(const Mat4& matrix, const Vec3& normal) const;
    Mat4 extractRotation(const Mat4& matrix) const;

    // UV utilities
    bool getAtlasUVs(uint8_t atlasIndex, const Vec2& pixelOffset,
        const Vec3& nodeSize, Vec2& uvMin, Vec2& uvMax) const;
    void rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const;

    // Block state rotation
    Vec3 rotateVertex(const Vec3& vertex, uint16_t state) const;
    Vec3 rotateNormal(const Vec3& normal, uint16_t state) const;

public:
    MeshBuilder(ModelRegistry* registry, TextureRegistry* textureRegistry);

    void generateChunkMesh(const World* world, const ChunkColumn* chunk, Mesh& outputMesh);
    void generateBlockMesh(const World* world, int32_t blockX, int32_t blockY, int32_t blockZ,
        Mesh& outputMesh);
};