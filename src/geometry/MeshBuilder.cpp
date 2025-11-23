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

MeshBuilder::MeshBuilder(BlockModelRegistry* registry, BlockIDMappings* blockIDMappings,
    TextureRegistry* textureRegistry, TextureAtlas* atlas)
    : modelRegistry(registry), blockIDMappings(blockIDMappings), textureRegistry(textureRegistry), textureAtlas(atlas) {}

bool MeshBuilder::isBlockOpaque(PackedBlock blockId) const {
    // TODO: Implement for transparent and non-full blocks (depending on rotation)
    return blockId != 0;
}

bool MeshBuilder::shouldRenderFace(const World* world, int32_t blockX, int32_t blockY, int32_t blockZ,
    FaceDirection face) const {
    const FaceOffset& offset = FACE_OFFSETS[static_cast<int>(face)];

    int32_t neighborX = blockX + offset.x;
    int32_t neighborY = blockY + offset.y;
    int32_t neighborZ = blockZ + offset.z;

    PackedBlock neighborBlock = world->getPackedBlockAt(neighborX, neighborY, neighborZ);
    return !isBlockOpaque(neighborBlock);
}

void MeshBuilder::generateBlockFace(Mesh& outputMesh, const BlockModel& model, FaceDirection face,
    int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const {
    // Each face is a quad with 4 vertices, defined counter-clockwise when viewed from outside
    Vertex v0, v1, v2, v3;
    std::string faceTextureName;

    // Get the texture for face
    switch (face) {
        case FaceDirection::NORTH: // -Z
            faceTextureName = model.faceTextures.count("north") ? model.faceTextures.at("north") : "default";

            v0.position = Vec3(0, 0, 0);
            v1.position = Vec3(1, 0, 0);
            v2.position = Vec3(1, 1, 0);
            v3.position = Vec3(0, 1, 0);

            // -Z direction
            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, -1);
            
            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;
        
        case FaceDirection::SOUTH: // +Z face
            faceTextureName = model.faceTextures.count("south") ?
                model.faceTextures.at("south") : "default";

            v0.position = Vec3(1, 0, 1);
            v1.position = Vec3(0, 0, 1);
            v2.position = Vec3(0, 1, 1);
            v3.position = Vec3(1, 1, 1);

            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, 1);

            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;

        case FaceDirection::EAST: // +X face
            faceTextureName = model.faceTextures.count("east") ?
                model.faceTextures.at("east") : "default";

            v0.position = Vec3(1, 0, 0);
            v1.position = Vec3(1, 0, 1);
            v2.position = Vec3(1, 1, 1);
            v3.position = Vec3(1, 1, 0);

            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(1, 0, 0);

            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;

        case FaceDirection::WEST: // -X face
            faceTextureName = model.faceTextures.count("west") ?
                model.faceTextures.at("west") : "default";

            v0.position = Vec3(0, 0, 1);
            v1.position = Vec3(0, 0, 0);
            v2.position = Vec3(0, 1, 0);
            v3.position = Vec3(0, 1, 1);

            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(-1, 0, 0);

            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;

        case FaceDirection::TOP: // +Y face
            faceTextureName = model.faceTextures.count("top") ?
                model.faceTextures.at("top") : "default";

            v0.position = Vec3(0, 1, 0);
            v1.position = Vec3(1, 1, 0);
            v2.position = Vec3(1, 1, 1);
            v3.position = Vec3(0, 1, 1);

            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 1, 0);

            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;

        case FaceDirection::BOTTOM: // -Y face
            faceTextureName = model.faceTextures.count("bottom") ?
                model.faceTextures.at("bottom") : "default";

            v0.position = Vec3(0, 0, 1);
            v1.position = Vec3(1, 0, 1);
            v2.position = Vec3(1, 0, 0);
            v3.position = Vec3(0, 0, 0);

            v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, -1, 0);

            v0.uv = calculateUV(faceTextureName, 0, 1);
            v1.uv = calculateUV(faceTextureName, 1, 1);
            v2.uv = calculateUV(faceTextureName, 1, 0);
            v3.uv = calculateUV(faceTextureName, 0, 0);
            break;
    }

    // Apply rotation based on BlockState
    v0.position = rotateVertex(v0.position, state);
    v1.position = rotateVertex(v1.position, state);
    v2.position = rotateVertex(v2.position, state);
    v3.position = rotateVertex(v3.position, state);

    // Translate to world position
    v0.position.x += worldX;
    v0.position.y += worldY;
    v0.position.z += worldZ;

    v1.position.x += worldX;
    v1.position.y += worldY;
    v1.position.z += worldZ;

    v2.position.x += worldX;
    v2.position.y += worldY;
    v2.position.z += worldZ;

    v3.position.x += worldX;
    v3.position.y += worldY;
    v3.position.z += worldZ;

    // Add vertices to mesh and fetch indices
    uint32_t idx0 = outputMesh.addVertex(v0);
    uint32_t idx1 = outputMesh.addVertex(v1);
    uint32_t idx2 = outputMesh.addVertex(v2);
    uint32_t idx3 = outputMesh.addVertex(v3);

    // Create the quad
    Face quadFace;
    quadFace.indices[0] = idx0;
    quadFace.indices[1] = idx1;
    quadFace.indices[2] = idx2;
    quadFace.indices[3] = idx3;
    quadFace.vertexCount = 4;
    quadFace.material = faceTextureName;

    outputMesh.addFace(quadFace);
}

Vec3 MeshBuilder::rotateVertex(const Vec3& vertex, uint16_t state) const {
    // TODO: Rotate vertex based on BlockState (may need to support flipped/mirrored?)
    return vertex;
}

Vec2 MeshBuilder::calculateUV(const std::string& textureName, float localU, float localV) const {
    Vec2 uvMin, uvMax;

    if (textureAtlas->getTextureUVs(textureName, uvMin, uvMax)) {
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
                int32_t worldY = chunk->position.y * CHUNK_SIZE_Y + y;
                int32_t worldZ = z;


                std::string modelName;
                BlockModel* model;
                uint16_t state;

                // TODO: Mapping from blockId to the modelName
                //BlockModel* model = modelRegistry->getModel(modelName);
                
                // TODO: Fetching BlockState

                for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
                    FaceDirection face = static_cast<FaceDirection>(faceIdx);

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
    BlockModel* model;
    
    for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
        FaceDirection face = static_cast<FaceDirection>(faceIdx);

        if (shouldRenderFace(world, blockX, blockY, blockZ, face)) {
            generateBlockFace(outputMesh, *model, face, blockX, blockY, blockZ, state);
        }
    }
}