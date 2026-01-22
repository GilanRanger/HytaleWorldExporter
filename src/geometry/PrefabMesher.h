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

private:
    ModelRegistry* modelRegistry;
    TextureRegistry* textureRegistry;

    void generateBoxNode(Mesh& outputMesh, const Model& model,
        const ModelNode& node, int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation);
    void generateQuadNode(Mesh& outputMesh, const Model& model,
        const ModelNode& node, int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation);
    void generateBoxFace(Mesh& outputMesh, const Model& model, const ModelNode& node,
        ModelNode::QuadNormal face, const Mat4& transform, const Vec3& halfSize,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation);
    void generateQuadFace(Mesh& outputMesh, const Model& model, const ModelNode& node,
        ModelNode::QuadNormal normalDir, const Mat4& transform, const Vec2& halfSize,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation);

    Mat4 calculateNodeTransform(const Model& model, const ModelNode& node) const;
    Vec3 transformPoint(const Mat4& matrix, const Vec3& point) const;
    Vec3 transformNormal(const Mat4& matrix, const Vec3& normal) const;
    Mat4 extractRotation(const Mat4& matrix) const;

    bool getAtlasUVs(const ModelNode& node, const Vec2& pixelOffset,
        const Vec3& nodeSize, Vec2& uvMin, Vec2& uvMax) const;

    void rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const;
    Vec3 rotateVertex(const Vec3& vertex, uint16_t rotation) const;
    Vec3 rotateNormal(const Vec3& normal, uint16_t rotation) const;

    static const struct FaceOffset {
        int x, y, z;
    } FACE_OFFSETS[6];
};