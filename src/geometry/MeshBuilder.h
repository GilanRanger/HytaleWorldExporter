#pragma once
#include "../data/WorldData.h"
#include "../data/MeshData.h"
#include "../data/Model.h"
#include <memory>

struct FaceOffset {
    int8_t x, y, z;
};

class MeshBuilder {
private:
    ModelRegistry* modelRegistry;
    BlockIDMappings* blockIDMappings;
    TextureRegistry* textureRegistry;

    static const FaceOffset FACE_OFFSETS[6];

    bool shouldRenderFace(const World* world, 
        int32_t blockX, int32_t blockY, int32_t blockZ, Cube::FaceDirection face) const;

    bool isBlockOpaque(PackedBlock blockId) const;

    void generateBlockFace(Mesh& outputMesh, const Model& model, Cube::FaceDirection face,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const;

    void generateFullModel(Mesh& outputMesh, const Model& model, Cube::FaceDirection face,
        int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const;

    void rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const;

    std::string resolveTexture(const std::string& textureRef,
        const std::unordered_map<std::string, std::string>& textures) const;

    Vec3 rotateVertex(const Vec3& vertex, uint16_t state) const;

    Vec2 calculateUV(const std::string& textureName, float localU, float localV) const;

public:
    MeshBuilder(ModelRegistry* registry, BlockIDMappings* blockIDMappings, 
        TextureRegistry* textureRegistry);

    void generateChunkMesh(const World* world, const Chunk* chunk, Mesh& outputMesh);

    void generateBlockMesh(const World* world,
        int32_t blockX, int32_t blockY, int32_t blockZ, Mesh& outputMesh);
};