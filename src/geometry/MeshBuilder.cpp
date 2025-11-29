#include "MeshBuilder.h"
#include <cmath>

const FaceOffset MeshBuilder::FACE_OFFSETS[6]{
    {0, 0, -1},  // NORTH (-Z)
    {0, 0, 1},   // SOUTH (+Z)
    {1, 0, 0},   // EAST (+X)
    {-1, 0, 0},  // WEST (-X)
    {0, 1, 0},   // TOP (+Y)
    {0, -1, 0}   // BOTTOM (-Y)
};

MeshBuilder::MeshBuilder(ModelRegistry* registry, BlockIDMappings* blockIDMappings,
    TextureRegistry* textureRegistry)
    : modelRegistry(registry), blockIDMappings(blockIDMappings), textureRegistry(textureRegistry) {}

bool MeshBuilder::isBlockOpaque(PackedBlock blockId) const {
    // TODO: Implement for transparent and non-full blocks (depending on rotation)
    return blockId != 0;
}

bool MeshBuilder::shouldRenderFace(const World* world, int32_t blockX, int32_t blockY, int32_t blockZ,
    Cube::FaceDirection face) const {
    const FaceOffset& offset = FACE_OFFSETS[static_cast<int>(face)];

    int32_t neighborX = blockX + offset.x;
    int32_t neighborY = blockY + offset.y;
    int32_t neighborZ = blockZ + offset.z;

    PackedBlock neighborBlock = world->getPackedBlockAt(neighborX, neighborY, neighborZ);
    return !isBlockOpaque(neighborBlock);
}

void MeshBuilder::generateBlockFace(Mesh& outputMesh, const Model& model, Cube::FaceDirection face,
    int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const {
    
    if (model.elements.empty()) return;

    const Cube& cube = model.elements[0];
    const CubeFace& faceData = cube.faces[static_cast<int>(face)];

    std::string textureName = resolveTexture(faceData.texture, model.textures);

    Vertex v0, v1, v2, v3;

    switch (face) {
        case Cube::FaceDirection::north: // -Z
            v0.position = Vec3(0, 0, 0);
            v1.position = Vec3(1, 0, 0);
            v2.position = Vec3(1, 1, 0);
            v3.position = Vec3(0, 1, 0);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, -1);
            break;

        case Cube::FaceDirection::south: // +Z
            v0.position = Vec3(1, 0, 1);
            v1.position = Vec3(0, 0, 1);
            v2.position = Vec3(0, 1, 1);
            v3.position = Vec3(1, 1, 1);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, 1);
            break;

        case Cube::FaceDirection::east: // +X
            v0.position = Vec3(1, 0, 0);
            v1.position = Vec3(1, 0, 1);
            v2.position = Vec3(1, 1, 1);
            v3.position = Vec3(1, 1, 0);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(1, 0, 0);
            break;

        case Cube::FaceDirection::west: // -X
            v0.position = Vec3(0, 0, 1);
            v1.position = Vec3(0, 0, 0);
            v2.position = Vec3(0, 1, 0);
            v3.position = Vec3(0, 1, 1);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(-1, 0, 0);
            break;

        case Cube::FaceDirection::up: // +Y
            v0.position = Vec3(0, 1, 0);
            v1.position = Vec3(1, 1, 0);
            v2.position = Vec3(1, 1, 1);
            v3.position = Vec3(0, 1, 1);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 1, 0);
            break;

        case Cube::FaceDirection::down: // -Y
            v0.position = Vec3(0, 0, 1);
            v1.position = Vec3(1, 0, 1);
            v2.position = Vec3(1, 0, 0);
            v3.position = Vec3(0, 0, 0);
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, -1, 0);
            break;
    }

    // Apply UVs
    v0.uv = Vec2(faceData.uv.x, faceData.uv.w); // min U, max V (bottom-left)
    v1.uv = Vec2(faceData.uv.z, faceData.uv.w); // max U, max V (bottom-right)
    v2.uv = Vec2(faceData.uv.z, faceData.uv.y); // max U, min V (top-right)
    v3.uv = Vec2(faceData.uv.x, faceData.uv.y); // min U, min V (top-left)

    if (faceData.uvRotation != 0) {
        rotateUVs(v0.uv, v1.uv, v2.uv, v3.uv, faceData.uvRotation);
    }

    // Rotation based on Block state
    v0.position = rotateVertex(v0.position, state);
    v1.position = rotateVertex(v1.position, state);
    v2.position = rotateVertex(v2.position, state);
    v3.position = rotateVertex(v3.position, state);

    // Translate to world position
    v0.position += Vec3(worldX, worldY, worldZ);
    v1.position += Vec3(worldX, worldY, worldZ);
    v2.position += Vec3(worldX, worldY, worldZ);
    v3.position += Vec3(worldX, worldY, worldZ);

    // Add vertices to mesh and fetch indices
    uint32_t idx0 = outputMesh.addVertex(v0);
    uint32_t idx1 = outputMesh.addVertex(v1);
    uint32_t idx2 = outputMesh.addVertex(v2);
    uint32_t idx3 = outputMesh.addVertex(v3);

    // Create quad
    MeshFace quadFace;
    quadFace.indices[0] = idx0;
    quadFace.indices[1] = idx1;
    quadFace.indices[2] = idx2;
    quadFace.indices[3] = idx3;
    quadFace.vertexCount = 4;
    quadFace.material = textureName;

    outputMesh.addFace(quadFace);
}

// For fetching the texture from the '#' reference used (based on Vintage Story)
std::string MeshBuilder::resolveTexture(const std::string& textureRef,
    const std::unordered_map<std::string, std::string>& textures) const {

    if (textureRef.empty() || textureRef[0] != '#') {
        return textureRef;
    }

    // Remove '#' and look up in textures map
    std::string key = textureRef.substr(1);
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }

    return "default";
}

void MeshBuilder::rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const {
    // Rotate in 90-degree increments clockwise
    int steps = (rotation / 90) % 4;

    for (int i = 0; i < steps; i++) {
        Vec2 temp = uv0;
        uv0 = uv3;
        uv3 = uv2;
        uv2 = uv1;
        uv1 = temp;
    }
}

Vec3 MeshBuilder::rotateVertex(const Vec3& vertex, uint16_t state) const {
    // TODO: Rotate vertex based on BlockState (may need to support flipped/mirrored?)
    return vertex;
}

Vec2 MeshBuilder::calculateUV(const std::string& textureName, float localU, float localV) const {
    Vec2 uvMin, uvMax;

    if (textureRegistry->getTextureUVs(textureName, uvMin, uvMax)) {
        Vec2 result;
        result.u = uvMin.u + (uvMax.u - uvMin.u) * localU;
        result.v = uvMin.v + (uvMax.v - uvMin.v) * localV;
        return result;
    }

    // No texture found
    return Vec2(0, 0);
}

void MeshBuilder::generateChunkMesh(const World* world, const Chunk* chunk, Mesh& outputMesh) {
    outputMesh.clear();

    for (uint8_t y = 0; y < CHUNK_SIZE_Y; y++) {
        for (uint8_t z = 0; z < CHUNK_SIZE_Z; z++) {
            for (uint8_t x = 0; x < CHUNK_SIZE_X; x++) {
                PackedBlock blockId = chunk->getPackedBlock(x, y, z);

                // Skip air
                if (blockId == 0) continue;

                int32_t worldX = chunk->position.x * CHUNK_SIZE_X + x;
                int32_t worldY = y;
                int32_t worldZ = chunk->position.z * CHUNK_SIZE_Z + z;


                std::string modelName;
                Model* model;
                uint16_t state;

                // TODO: Mapping from blockId to the modelName
                //BlockModel* model = modelRegistry->getModel(modelName);
                
                // TODO: Fetching BlockState

                for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
                    Cube::FaceDirection face = static_cast<Cube::FaceDirection> (faceIdx);

                    if (shouldRenderFace(world, worldX, worldY, worldZ, face)) {
                        generateBlockFace(outputMesh, *model, face, worldX, worldY, worldZ, state);
                    }
                }
            }
        }
    }
}

void MeshBuilder::generateBlockMesh(const World* world,
    int32_t blockX, int32_t blockY, int32_t blockZ, Mesh& outputMesh) {
    outputMesh.clear();

    uint16_t blockId = world->getBlockIDAt(blockX, blockY, blockZ);

    if (blockId == 0) return;

    // TODO: Get block state
    uint16_t state = world->getBlockStateAt(blockX, blockY, blockZ);

    // TODO: Mapping from blockId to the modelName
    //BlockModel* model = modelRegistry->getModel(modelName);
    Model* model;
    
    for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        Cube::FaceDirection face = static_cast<Cube::FaceDirection>(faceIdx);

        if (shouldRenderFace(world, blockX, blockY, blockZ, face)) {
            generateBlockFace(outputMesh, *model, face, blockX, blockY, blockZ, state);
        }
    }
}