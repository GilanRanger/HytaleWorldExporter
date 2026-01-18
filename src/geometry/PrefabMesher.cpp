#include "PrefabMesher.h"
#include <cmath>
#include <utility>

const PrefabMesher::FaceOffset PrefabMesher::FACE_OFFSETS[6] = {
    {0, 0, -1},  // MinusZ - North
    {0, 0, 1},   // PlusZ - South
    {1, 0, 0},   // PlusX - East
    {-1, 0, 0},  // MinusX - West
    {0, 1, 0},   // PlusY - Up
    {0, -1, 0}   // MinusY - Down
};

PrefabMesher::PrefabMesher(ModelRegistry* registry, TextureRegistry* textureRegistry)
    : modelRegistry(registry), textureRegistry(textureRegistry) {
}

bool PrefabMesher::isBlockOpaque(const std::string& blockName) const {
    return blockName != "Empty" && !blockName.empty();
}

bool PrefabMesher::hasBlockAt(const Prefab& prefab, int32_t x, int32_t y, int32_t z) const {
    for (const auto& block : prefab.blocks) {
        if (block.x == x && block.y == y && block.z == z) {
            return isBlockOpaque(block.name);
        }
    }
    return false;
}

bool PrefabMesher::shouldRenderFace(const Prefab& prefab, int32_t blockX, int32_t blockY, int32_t blockZ,
    ModelNode::QuadNormal face) const {

    const FaceOffset& offset = FACE_OFFSETS[static_cast<int>(face)];

    int32_t neighborX = blockX + offset.x;
    int32_t neighborY = blockY + offset.y;
    int32_t neighborZ = blockZ + offset.z;

    return !hasBlockAt(prefab, neighborX, neighborY, neighborZ);
}

void PrefabMesher::generatePrefabMesh(const Prefab& prefab, Mesh& outputMesh) {
    outputMesh.clear();

    for (const auto& block : prefab.blocks) {
        if (block.name == "Empty" || block.name.empty()) continue;

        generateBlockMesh(prefab, block, block.x, block.y, block.z, outputMesh);
    }
}

void PrefabMesher::generateBlockMesh(const Prefab& prefab, const PrefabBlock& block,
    int32_t worldX, int32_t worldY, int32_t worldZ, Mesh& outputMesh) {

    Model* model = modelRegistry->getModel(block.name);
    if (!model) return;

    for (int faceIdx = 0; faceIdx < 6; ++faceIdx) {
        ModelNode::QuadNormal face = static_cast<ModelNode::QuadNormal>(faceIdx);

        if (shouldRenderFace(prefab, worldX, worldY, worldZ, face)) {
            generateBlockFace(outputMesh, *model, face, worldX, worldY, worldZ, block.rotation);
        }
    }
}

void PrefabMesher::generateBlockFace(Mesh& outputMesh, const Model& model,
    ModelNode::QuadNormal face, int32_t worldX, int32_t worldY, int32_t worldZ,
    uint16_t rotation) {

    if (model.rootNodes.empty()) return;

    for (int i = 0; i < model.nodeCount; ++i) {
        const ModelNode& node = model.allNodes[i];

        if (!node.visible || node.type != ModelNode::ShapeType::Box) {
            continue;
        }

        generateNodeFace(outputMesh, model, node, face, worldX, worldY, worldZ, rotation);
    }
}

void PrefabMesher::generateNodeFace(Mesh& outputMesh, const Model& model,
    const ModelNode& node, ModelNode::QuadNormal face,
    int32_t worldX, int32_t worldY, int32_t worldZ, uint16_t rotation) {

    if (node.type != ModelNode::ShapeType::Box) return;

    int faceIndex = static_cast<int>(face);
    if (faceIndex < 0 || faceIndex >= node.textureLayoutSize) return;

    const ModelFaceTextureLayout& faceLayout = node.textureLayout[faceIndex];
    if (faceLayout.hidden) return;

    Mat4 transform = calculateNodeTransform(model, node);

    Vec3 halfSize = node.size * (1.0f / 32.0f) * 0.5f;
    Vec3 center = node.offset * (1.0f / 32.0f);

    Vertex v0, v1, v2, v3;

    switch (face) {
    case ModelNode::QuadNormal::MinusZ:
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, -1);
        break;

    case ModelNode::QuadNormal::PlusZ:
        v0.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 0, 1);
        break;

    case ModelNode::QuadNormal::PlusX:
        v0.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(1, 0, 0);
        break;

    case ModelNode::QuadNormal::MinusX:
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v2.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(-1, 0, 0);
        break;

    case ModelNode::QuadNormal::PlusY:
        v0.position = center + Vec3(-halfSize.x, halfSize.y, -halfSize.z);
        v1.position = center + Vec3(halfSize.x, halfSize.y, -halfSize.z);
        v2.position = center + Vec3(halfSize.x, halfSize.y, halfSize.z);
        v3.position = center + Vec3(-halfSize.x, halfSize.y, halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, 1, 0);
        break;

    case ModelNode::QuadNormal::MinusY:
        v0.position = center + Vec3(-halfSize.x, -halfSize.y, halfSize.z);
        v1.position = center + Vec3(halfSize.x, -halfSize.y, halfSize.z);
        v2.position = center + Vec3(halfSize.x, -halfSize.y, -halfSize.z);
        v3.position = center + Vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        v0.normal = v1.normal = v2.normal = v3.normal = Vec3(0, -1, 0);
        break;
    }

    v0.position = transformPoint(transform, v0.position);
    v1.position = transformPoint(transform, v1.position);
    v2.position = transformPoint(transform, v2.position);
    v3.position = transformPoint(transform, v3.position);

    Mat4 normalTransform = extractRotation(transform);
    v0.normal = transformNormal(normalTransform, v0.normal);
    v1.normal = transformNormal(normalTransform, v1.normal);
    v2.normal = transformNormal(normalTransform, v2.normal);
    v3.normal = transformNormal(normalTransform, v3.normal);

    Vec2 uvMin, uvMax;
    if (getAtlasUVs(node.atlasIndex, faceLayout.offset, node.size, uvMin, uvMax)) {
        v0.uv = Vec2(uvMin.u, uvMax.v);
        v1.uv = Vec2(uvMax.u, uvMax.v);
        v2.uv = Vec2(uvMax.u, uvMin.v);
        v3.uv = Vec2(uvMin.u, uvMin.v);

        if (faceLayout.angle != 0) {
            rotateUVs(v0.uv, v1.uv, v2.uv, v3.uv, faceLayout.angle);
        }

        if (faceLayout.mirrorX) {
            std::swap(v0.uv.u, v1.uv.u);
            std::swap(v2.uv.u, v3.uv.u);
        }
        if (faceLayout.mirrorY) {
            std::swap(v0.uv.v, v3.uv.v);
            std::swap(v1.uv.v, v2.uv.v);
        }
    }

    v0.position = rotateVertex(v0.position, rotation);
    v1.position = rotateVertex(v1.position, rotation);
    v2.position = rotateVertex(v2.position, rotation);
    v3.position = rotateVertex(v3.position, rotation);

    v0.normal = rotateNormal(v0.normal, rotation);
    v1.normal = rotateNormal(v1.normal, rotation);
    v2.normal = rotateNormal(v2.normal, rotation);
    v3.normal = rotateNormal(v3.normal, rotation);

    v0.position += Vec3(static_cast<float>(worldX), static_cast<float>(worldY), static_cast<float>(worldZ));
    v1.position += Vec3(static_cast<float>(worldX), static_cast<float>(worldY), static_cast<float>(worldZ));
    v2.position += Vec3(static_cast<float>(worldX), static_cast<float>(worldY), static_cast<float>(worldZ));
    v3.position += Vec3(static_cast<float>(worldX), static_cast<float>(worldY), static_cast<float>(worldZ));

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

Mat4 PrefabMesher::calculateNodeTransform(const Model& model, const ModelNode& node) const {
    int nodeIndex = -1;
    for (int i = 0; i < model.nodeCount; ++i) {
        if (&model.allNodes[i] == &node) {
            nodeIndex = i;
            break;
        }
    }

    if (nodeIndex == -1) return Mat4::Identity();

    std::vector<int> hierarchy;
    int current = nodeIndex;
    while (current != -1) {
        hierarchy.push_back(current);
        current = model.parentNodes[current];
    }

    Mat4 transform = Mat4::Identity();
    for (int i = static_cast<int>(hierarchy.size()) - 1; i >= 0; --i) {
        const ModelNode& n = model.allNodes[hierarchy[i]];

        Vec3 pos = (n.position + n.proceduralOffset) * (1.0f / 32.0f);

        Mat4 translation = Mat4::Translate(pos);
        Mat4 rotation = n.orientation.toMatrix();
        Mat4 stretch = Mat4::Scale(n.stretch);

        transform = transform * translation * rotation * stretch;
    }

    return transform;
}

Vec3 PrefabMesher::transformPoint(const Mat4& matrix, const Vec3& point) const {
    float x = matrix.m[0][0] * point.x + matrix.m[0][1] * point.y + matrix.m[0][2] * point.z + matrix.m[0][3];
    float y = matrix.m[1][0] * point.x + matrix.m[1][1] * point.y + matrix.m[1][2] * point.z + matrix.m[1][3];
    float z = matrix.m[2][0] * point.x + matrix.m[2][1] * point.y + matrix.m[2][2] * point.z + matrix.m[2][3];
    return Vec3(x, y, z);
}

Vec3 PrefabMesher::transformNormal(const Mat4& matrix, const Vec3& normal) const {
    float x = matrix.m[0][0] * normal.x + matrix.m[0][1] * normal.y + matrix.m[0][2] * normal.z;
    float y = matrix.m[1][0] * normal.x + matrix.m[1][1] * normal.y + matrix.m[1][2] * normal.z;
    float z = matrix.m[2][0] * normal.x + matrix.m[2][1] * normal.y + matrix.m[2][2] * normal.z;
    return Vec3(x, y, z).normalize();
}

Mat4 PrefabMesher::extractRotation(const Mat4& matrix) const {
    Mat4 result = Mat4::Identity();
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = matrix.m[i][j];
        }
    }
    return result;
}

bool PrefabMesher::getAtlasUVs(uint8_t atlasIndex, const Vec2& pixelOffset,
    const Vec3& nodeSize, Vec2& uvMin, Vec2& uvMax) const {

    const float atlasWidth = 1024.0f;
    const float atlasHeight = 1024.0f;
    const float tileSize = 32.0f;

    uvMin.u = pixelOffset.u / atlasWidth;
    uvMin.v = pixelOffset.v / atlasHeight;
    uvMax.u = (pixelOffset.u + tileSize) / atlasWidth;
    uvMax.v = (pixelOffset.v + tileSize) / atlasHeight;

    return true;
}

void PrefabMesher::rotateUVs(Vec2& uv0, Vec2& uv1, Vec2& uv2, Vec2& uv3, int rotation) const {
    int steps = (rotation / 90) % 4;

    for (int i = 0; i < steps; ++i) {
        Vec2 temp = uv0;
        uv0 = uv3;
        uv3 = uv2;
        uv2 = uv1;
        uv1 = temp;
    }
}

Vec3 PrefabMesher::rotateVertex(const Vec3& vertex, uint16_t rotation) const {
    // TODO: Implement block rotation based on rotation value
    return vertex;
}

Vec3 PrefabMesher::rotateNormal(const Vec3& normal, uint16_t rotation) const {
    return rotateVertex(normal, rotation);
}