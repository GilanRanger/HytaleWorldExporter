#pragma once
#include "../data/Prefab.h"
#include "../data/MeshData.h"
#include "../data/Model.h"
#include "../data/Vec.h"
#include <unordered_map>
#include <string>

class PrefabMesher {
public:
    PrefabMesher(ModelRegistry* registry, TextureRegistry* textureRegistry);

    void generatePrefabMesh(const Prefab& prefab, Mesh& outputMesh);

    void generateBlockMesh(const Prefab& prefab, const PrefabBlock& block,
        int32_t worldX, int32_t worldY, int32_t worldZ,
        Mesh& outputMesh);

private:
    ModelRegistry* modelRegistry;
    TextureRegistry* textureRegistry;

    void generateBlockFace(Mesh& outputMesh, const Model& model,
        ModelNode::QuadNormal face, int32_t worldX, int32_t worldY, int32_t worldZ,
        uint16_t rotation);

    void generateNodeFace(Mesh& outputMesh, const Model& model,
        const ModelNode& node, ModelNode::QuadNormal face,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation);

    bool shouldRenderFace(const Prefab& prefab, int32_t blockX, int32_t blockY, int32_t blockZ,
        ModelNode::QuadNormal face) const;

    bool isBlockOpaque(const std::string& blockName) const;
    bool hasBlockAt(const Prefab& prefab, int32_t x, int32_t y, int32_t z) const;

    Mat4 calculateNodeTransform(const Model& model, const ModelNode& node) const;
    Vec3 transformPoint(const Mat4& matrix, const Vec3& point) const;
    Vec3 transformNormal(const Mat4& matrix, const Vec3& normal) const;
    Mat4 extractRotation(const Mat4& matrix) const;

    bool getAtlasUVs(uint8_t atlasIndex, const Vec2& pixelOffset,
        const Vec3& nodeSize, Vec2& uvMin, Vec2& uvMax) const;

    void rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const;
    Vec3 rotateVertex(const Vec3& vertex, uint16_t rotation) const;
    Vec3 rotateNormal(const Vec3& normal, uint16_t rotation) const;

    static const struct FaceOffset {
        int x, y, z;
    } FACE_OFFSETS[6];
};