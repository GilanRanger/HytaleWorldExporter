#include "MeshBuilder.h"
#include <cmath>
#include <utility>

const FaceOffset MeshBuilder::FACE_OFFSETS[6] = {
    {0, 0, -1},  // MinusZ - North
    {0, 0, 1},   // PlusZ - South
    {1, 0, 0},   // PlusX - East
    {-1, 0, 0},  // MinusX - West
    {0, 1, 0},   // PlusY - Up
    {0, -1, 0}   // MinusY - Down
};

MeshBuilder::MeshBuilder(ModelRegistry* registry, BlockIDMappings* blockIDMappings,
    TextureRegistry* textureRegistry)
    : modelRegistry(registry), blockIDMappings(blockIDMappings), textureRegistry(textureRegistry) {
}

bool MeshBuilder::isBlockOpaque(PackedBlock blockId) const {
    // TODO: Implement proper transparency checking based on block properties
    return blockId != 0;
}

bool MeshBuilder::shouldRenderFace(const World* world, int32_t blockX, int32_t blockY, int32_t blockZ,
    ModelNode::QuadNormal face) const {

    const FaceOffset& offset = FACE_OFFSETS[static_cast<int>(face)];

    int32_t neighborX = blockX + offset.x;
    int32_t neighborY = blockY + offset.y;
    int32_t neighborZ = blockZ + offset.z;

    PackedBlock neighborBlock = world->getPackedBlockAt(neighborX, neighborY, neighborZ);
    return !isBlockOpaque(neighborBlock);
}

void MeshBuilder::generateBlockFace(Mesh& outputMesh, const Model& model,
    ModelNode::QuadNormal face, int32_t worldX, int32_t worldY, int32_t worldZ,
    uint16_t state) const {

    if (model.rootNodes.empty()) return;

    // Iterate through all visible box nodes
    for (int i = 0; i < model.nodeCount; ++i) {
        const ModelNode& node = model.allNodes[i];

        if (!node.visible || node.type != ModelNode::ShapeType::Box) {
            continue;
        }

        generateNodeFace(outputMesh, model, node, face, worldX, worldY, worldZ, state);
    }
}

void MeshBuilder::generateNodeFace(Mesh& outputMesh, const Model& model,
    const ModelNode& node, ModelNode::QuadNormal face,
    int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t state) const {

    if (node.type != ModelNode::ShapeType::Box) return;

    // Validate face index
    int faceIndex = static_cast<int>(face);
    if (faceIndex < 0 || faceIndex >= node.textureLayoutSize) return;

    const ModelFaceTextureLayout& faceLayout = node.textureLayout[faceIndex];
    if (faceLayout.hidden) return;

    // Calculate node's world transform
    Mat4 transform = calculateNodeTransform(model, node);

    // Calculate vertex positions
    // Hytale uses 1 unit = 1/32 of a block
    Vec3 halfSize = node.size * (1.0f / 32.0f) * 0.5f;
    Vec3 center = node.offset * (1.0f / 32.0f);

    Vertex v0, v1, v2, v3;

    switch (face) {
    case ModelNode::QuadNormal::MinusZ: // North (-Z)
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, -1);
        break;

    case ModelNode::QuadNormal::PlusZ: // South (+Z)
        v0.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, 1);
        break;

    case ModelNode::QuadNormal::PlusX: // East (+X)
        v0.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(1, 0, 0);
        break;

    case ModelNode::QuadNormal::MinusX: // West (-X)
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v2.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(-1, 0, 0);
        break;

    case ModelNode::QuadNormal::PlusY: // Up (+Y)
        v0.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 1, 0);
        break;

    case ModelNode::QuadNormal::MinusY: // Down (-Y)
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, -1, 0);
        break;
    }

    // Apply node transform (position, orientation, stretch)
    v0.position = transformPoint(transform, v0.position);
    v1.position = transformPoint(transform, v1.position);
    v2.position = transformPoint(transform, v2.position);
    v3.position = transformPoint(transform, v3.position);

    // Transform normals by rotation only
    Mat4 normalTransform = extractRotation(transform);
    v0.normal = transformNormal(normalTransform, v0.normal);
    v1.normal = transformNormal(normalTransform, v1.normal);
    v2.normal = transformNormal(normalTransform, v2.normal);
    v3.normal = transformNormal(normalTransform, v3.normal);

    // Apply UVs from texture atlas
    Vec2 uvMin, uvMax;
    if (getAtlasUVs(node.atlasIndex, faceLayout.offset, node.size, uvMin, uvMax)) {
        v0.uv = Vec2(uvMin.u, uvMax.v); // bottom-left
        v1.uv = Vec2(uvMax.u, uvMax.v); // bottom-right
        v2.uv = Vec2(uvMax.u, uvMin.v); // top-right
        v3.uv = Vec2(uvMin.u, uvMin.v); // top-left

        // Apply UV rotation
        if (faceLayout.angle != 0) {
            rotateUVs(v0.uv, v1.uv, v2.uv, v3.uv, faceLayout.angle);
        }

        // Apply UV mirroring
        if (faceLayout.mirrorX) {
            std::swap(v0.uv.u, v1.uv.u);
            std::swap(v2.uv.u, v3.uv.u);
        }
        if (faceLayout.mirrorY) {
            std::swap(v0.uv.v, v3.uv.v);
            std::swap(v1.uv.v, v2.uv.v);
        }
    }

    // Apply block state rotation
    v0.position = rotateVertex(v0.position, state);
    v1.position = rotateVertex(v1.position, state);
    v2.position = rotateVertex(v2.position, state);
    v3.position = rotateVertex(v3.position, state);

    v0.normal = rotateNormal(v0.normal, state);
    v1.normal = rotateNormal(v1.normal, state);
    v2.normal = rotateNormal(v2.normal, state);
    v3.normal = rotateNormal(v3.normal, state);

    // Translate to world position
    v0.position += Vec3(worldX, worldY, worldZ);
    v1.position += Vec3(worldX, worldY, worldZ);
    v2.position += Vec3(worldX, worldY, worldZ);
    v3.position += Vec3(worldX, worldY, worldZ);

    // Add vertices and create face
    uint32_t idx0 = outputMesh.addVertex(v0);
    uint32_t idx1 = outputMesh.addVertex(v1);
    uint32_t idx2 = outputMesh.addVertex(v2);
    uint32_t idx3 = outputMesh.addVertex(v3);

    MeshFace quadFace;
    quadFace.indices[0] = idx0;
    quadFace.indices[1] = idx1;
    quadFace.indices[2] = idx2;
    quadFace.indices[3] = idx3;
    quadFace.vertexCount = 4;
    quadFace.material = std::to_string(node.atlasIndex);
    outputMesh.addFace(quadFace);
}

// ============================================================================
// Transform Utilities
// ============================================================================

Mat4 MeshBuilder::calculateNodeTransform(const Model& model, const ModelNode& node) const {
    // Find node index
    int nodeIndex = -1;
    for (int i = 0; i < model.nodeCount; ++i) {
        if (&model.allNodes[i] == &node) {
            nodeIndex = i;
            break;
        }
    }

    if (nodeIndex == -1) return Mat4::Identity();

    // Build hierarchy path from node to root
    std::vector<int> hierarchy;
    int current = nodeIndex;
    while (current != -1) {
        hierarchy.push_back(current);
        current = model.parentNodes[current];
    }

    // Apply transforms from root to node
    Mat4 transform = Mat4::Identity();
    for (int i = static_cast<int>(hierarchy.size()) - 1; i >= 0; --i) {
        const ModelNode& n = model.allNodes[hierarchy[i]];

        // Convert position to block space (Hytale units to blocks)
        Vec3 pos = (n.position + n.proceduralOffset) * (1.0f / 32.0f);

        Mat4 translation = Mat4::Translate(pos);
        Mat4 rotation = n.orientation.toMatrix();
        Mat4 stretch = Mat4::Scale(n.stretch);

        // Combine: Translate * Rotate * Stretch
        transform = transform * translation * rotation * stretch;
    }

    return transform;
}

Vec3 MeshBuilder::transformPoint(const Mat4& matrix, const Vec3& point) const {
    float x = matrix.m[0][0] * point.x + matrix.m[0][1] * point.y + matrix.m[0][2] * point.z + matrix.m[0][3];
    float y = matrix.m[1][0] * point.x + matrix.m[1][1] * point.y + matrix.m[1][2] * point.z + matrix.m[1][3];
    float z = matrix.m[2][0] * point.x + matrix.m[2][1] * point.y + matrix.m[2][2] * point.z + matrix.m[2][3];
    return Vec3(x, y, z);
}

Vec3 MeshBuilder::transformNormal(const Mat4& matrix, const Vec3& normal) const {
    float x = matrix.m[0][0] * normal.x + matrix.m[0][1] * normal.y + matrix.m[0][2] * normal.z;
    float y = matrix.m[1][0] * normal.x + matrix.m[1][1] * normal.y + matrix.m[1][2] * normal.z;
    float z = matrix.m[2][0] * normal.x + matrix.m[2][1] * normal.y + matrix.m[2][2] * normal.z;
    return Vec3(x, y, z).normalize();
}

Mat4 MeshBuilder::extractRotation(const Mat4& matrix) const {
    Mat4 result = Mat4::Identity();
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = matrix.m[i][j];
        }
    }
    return result;
}

bool MeshBuilder::getAtlasUVs(uint8_t atlasIndex, const Vec2& pixelOffset,
    const Vec3& nodeSize, Vec2& uvMin, Vec2& uvMax) const {

    // TODO: Get actual atlas dimensions from TextureRegistry
    const float atlasWidth = 1024.0f;
    const float atlasHeight = 1024.0f;
    const float tileSize = 32.0f;

    uvMin.u = pixelOffset.u / atlasWidth;
    uvMin.v = pixelOffset.v / atlasHeight;
    uvMax.u = (pixelOffset.u + tileSize) / atlasWidth;
    uvMax.v = (pixelOffset.v + tileSize) / atlasHeight;

    return true;
}

void MeshBuilder::rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const {
    int steps = (rotation / 90) % 4;

    for (int i = 0; i < steps; ++i) {
        Vec2 temp = uv0;
        uv0 = uv3;
        uv3 = uv2;
        uv2 = uv1;
        uv1 = temp;
    }
}

Vec3 MeshBuilder::rotateVertex(const Vec3& vertex, uint16_t state) const {
    // TODO: Implement block state rotation based on state bits
    return vertex;
}

Vec3 MeshBuilder::rotateNormal(const Vec3& normal, uint16_t state) const {
    // TODO: Implement block state normal rotation
    return rotateVertex(normal, state);
}

void MeshBuilder::generateChunkMesh(const World* world, const Chunk* chunk, Mesh& outputMesh) {
    outputMesh.clear();

    for (uint8_t y = 0; y < CHUNK_SIZE_Y; ++y) {
        for (uint8_t z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (uint8_t x = 0; x < CHUNK_SIZE_X; ++x) {
                PackedBlock blockId = chunk->getPackedBlock(x, y, z);

                if (blockId == 0) continue; // Skip air

                int32_t worldX = chunk->position.x * CHUNK_SIZE_X + x;
                int32_t worldY = y;
                int32_t worldZ = chunk->position.z * CHUNK_SIZE_Z + z;

                // TODO: Map blockId to model name
                std::string modelName = blockIDMappings->getBlockName(blockId);
                Model* model = modelRegistry->getModel(modelName);
                if (!model) continue;

                // TODO: Get block state from world
                uint16_t state = world->getBlockStateAt(worldX, worldY, worldZ);

                // Generate all visible faces
                for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
                    ModelNode::QuadNormal face = static_cast<ModelNode::QuadNormal>(faceIdx);

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

    PackedBlock blockId = world->getPackedBlockAt(blockX, blockY, blockZ);
    if (blockId == 0) return; // Air

    // TODO: Map blockId to model name
    std::string modelName = blockIDMappings->getBlockName(blockId);
    Model* model = modelRegistry->getModel(modelName);
    if (!model) return;

    uint16_t state = world->getBlockStateAt(blockX, blockY, blockZ);

    // Generate all visible faces
    for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
        ModelNode::QuadNormal face = static_cast<ModelNode::QuadNormal>(faceIdx);

        if (shouldRenderFace(world, blockX, blockY, blockZ, face)) {
            generateBlockFace(outputMesh, *model, face, blockX, blockY, blockZ, state);
        }
    }
}