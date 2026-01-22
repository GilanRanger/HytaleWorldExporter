#include "../data/Model.h"
#include <cstring>
#include <algorithm>

int Model::MaxNodeCount = 256;

Model::Model(int preAllocatedNodeCount)
    : nodeCount(0), allocatedNodeCount(preAllocatedNodeCount), gradientId(0) {

    allNodes = new ModelNode[preAllocatedNodeCount];
    parentNodes = new int[preAllocatedNodeCount];

    for (int i = 0; i < preAllocatedNodeCount; i++) {
        parentNodes[i] = -1;
    }
}

Model::~Model() {
    delete[] allNodes;
    delete[] parentNodes;
}

void Model::addNode(ModelNode& node, int parentNodeIndex) {
    ensureNodeCountAllocated(nodeCount + 1, NodeGrowthAmount);

    int nodeIndex = nodeCount;
    allNodes[nodeIndex] = node;
    parentNodes[nodeIndex] = parentNodeIndex;

    if (node.nameId != EmptyNodeNameId) {
        nodeIndicesByNameId[node.nameId] = nodeIndex;
    }

    if (parentNodeIndex == -1) {
        rootNodes.push_back(nodeIndex);
    }
    else {
        allNodes[parentNodeIndex].children.push_back(nodeIndex);
    }

    nodeCount++;
}

Model Model::clone() const {
    Model cloned(allocatedNodeCount);

    cloned.nodeCount = nodeCount;
    cloned.gradientId = gradientId;
    cloned.rootNodes = rootNodes;
    cloned.nodeIndicesByNameId = nodeIndicesByNameId;

    for (int i = 0; i < nodeCount; i++) {
        cloned.allNodes[i] = allNodes[i].clone();
        cloned.parentNodes[i] = parentNodes[i];
    }

    return cloned;
}

void Model::attach(Model* attachment, NodeNameManager* nodeNameManager,
    Vec2 uvMin, Vec2 uvMax, Vec2* uvOffset, int forcedTargetNodeNameId) {

    if (!attachment || attachment->nodeCount == 0) return;

    bool forcedAttachment = (forcedTargetNodeNameId != -1);

    for (int rootIndex : attachment->rootNodes) {
        ModelNode& attachmentRoot = attachment->allNodes[rootIndex];
        recurseAttach(attachment, attachmentRoot, -1, nodeNameManager,
            uvMin, uvMax, uvOffset, forcedAttachment);
    }
}

void Model::setUV(Vec2 min, Vec2 max) {
    for (int i = 0; i < nodeCount; i++) {
        allNodes[i].uvMin = min;
        allNodes[i].uvMax = max;
    }
}

void Model::setGradientId(uint8_t gradientId) {
    this->gradientId = gradientId;
    for (int i = 0; i < nodeCount; i++) {
        allNodes[i].gradientId = gradientId;
    }
}

void Model::offsetUVs(Vec2 offset) {
    for (int i = 0; i < nodeCount; i++) {
        ModelNode& node = allNodes[i];
        for (auto& layout : node.textureLayout) {
            layout.offset.u += offset.u;
            layout.offset.v += offset.v;
        }
    }
}

void Model::ensureNodeCountAllocated(int required, int growth) {
    if (required <= allocatedNodeCount) return;

    int newCapacity = std::max(required, allocatedNodeCount + growth);
    if (newCapacity > MaxNodeCount) {
        newCapacity = MaxNodeCount;
    }

    ModelNode* newNodes = new ModelNode[newCapacity];
    int* newParents = new int[newCapacity];

    for (int i = 0; i < nodeCount; i++) {
        newNodes[i] = allNodes[i];
        newParents[i] = parentNodes[i];
    }

    for (int i = nodeCount; i < newCapacity; i++) {
        newParents[i] = -1;
    }

    delete[] allNodes;
    delete[] parentNodes;

    allNodes = newNodes;
    parentNodes = newParents;
    allocatedNodeCount = newCapacity;
}

void Model::recurseAttach(Model* attachment, ModelNode& attachmentNode,
    int parentNodeIndex, NodeNameManager* nodeNameManager,
    Vec2 uvMin, Vec2 uvMax, Vec2* uvOffset, bool forcedAttachment) {

    ModelNode nodeCopy = attachmentNode.clone();

    nodeCopy.uvMin = uvMin;
    nodeCopy.uvMax = uvMax;

    if (uvOffset) {
        for (auto& layout : nodeCopy.textureLayout) {
            layout.offset.u += uvOffset->u;
            layout.offset.v += uvOffset->v;
        }
    }

    int newNodeIndex = nodeCount;
    addNode(nodeCopy, parentNodeIndex);

    for (int childIndex : attachmentNode.children) {
        ModelNode& childNode = attachment->allNodes[childIndex];
        recurseAttach(attachment, childNode, newNodeIndex, nodeNameManager,
            uvMin, uvMax, uvOffset, forcedAttachment);
    }
}

int NodeNameManager::getOrAddNameId(const std::string& name) {
    auto it = nameToId.find(name);
    if (it != nameToId.end()) {
        return it->second;
    }

    int id = nextId++;
    nameToId[name] = id;
    idToName[id] = name;
    return id;
}

bool NodeNameManager::tryGetNameFromId(int id, std::string& outName) const {
    auto it = idToName.find(id);
    if (it != idToName.end()) {
        outName = it->second;
        return true;
    }
    return false;
}

bool NodeNameManager::tryGetIdFromName(const std::string& name, int& outId) const {
    auto it = nameToId.find(name);
    if (it != nameToId.end()) {
        outId = it->second;
        return true;
    }
    return false;
}

void ModelInitializer::parse(const uint8_t* data, size_t dataSize,
    NodeNameManager* nodeNameManager, Model& blockyModel) {
    // Binary parsing not implemented yet
}

void ModelInitializer::parse(const ModelJson& json,
    NodeNameManager* nodeNameManager, Model& blockyModel) {

    for (auto& jsonNode : const_cast<ModelJson&>(json).nodes) {
        recurseParseNode(jsonNode, blockyModel, -1, nodeNameManager);
    }
}

void ModelInitializer::recurseParseNode(ModelNodeJson& jsonNode, Model& model,
    int parentNodeIndex, NodeNameManager* nodeNameManager) {

    ModelNode node;

    if (!jsonNode.name.empty()) {
        node.nameId = nodeNameManager->getOrAddNameId(jsonNode.name);
    }

    node.position = jsonNode.position;
    node.orientation = jsonNode.orientation;
    node.offset = jsonNode.shape.offset;
    node.stretch = jsonNode.shape.stretch;
    node.visible = jsonNode.shape.visible;
    node.doubleSided = jsonNode.shape.doubleSided;
    node.shadingMode = parseShadingMode(jsonNode.shape.shadingMode);
    node.isPiece = jsonNode.shape.settings.isPiece;

    if (jsonNode.shape.type == "box") {
        node.type = ModelNode::ShapeType::Box;
        node.size = jsonNode.shape.settings.size;
    }
    else if (jsonNode.shape.type == "quad") {
        node.type = ModelNode::ShapeType::Quad;
        node.size = Vec3(jsonNode.shape.settings.size.x, jsonNode.shape.settings.size.y, 0);
        node.quadNormalDirection = parseQuadNormal(jsonNode.shape.settings.normal);
    }
    else {
        node.type = ModelNode::ShapeType::None;
    }

    if (!jsonNode.shape.textureLayout.empty()) {
        if (node.type == ModelNode::ShapeType::Box) {
            node.textureLayout.resize(6);
            node.textureLayout[0] = getFaceLayout(jsonNode.shape.textureLayout, "front");
            node.textureLayout[1] = getFaceLayout(jsonNode.shape.textureLayout, "back");
            node.textureLayout[2] = getFaceLayout(jsonNode.shape.textureLayout, "right");
            node.textureLayout[3] = getFaceLayout(jsonNode.shape.textureLayout, "left");
            node.textureLayout[4] = getFaceLayout(jsonNode.shape.textureLayout, "top");
            node.textureLayout[5] = getFaceLayout(jsonNode.shape.textureLayout, "bottom");
        }
        else if (node.type == ModelNode::ShapeType::Quad) {
            node.textureLayout.resize(1);
            node.textureLayout[0] = getFaceLayout(jsonNode.shape.textureLayout, "front");
        }
    }

    int currentNodeIndex = model.nodeCount;
    model.addNode(node, parentNodeIndex);

    for (auto& child : jsonNode.children) {
        recurseParseNode(child, model, currentNodeIndex, nodeNameManager);
    }
}

ModelNode ModelNode::clone() const {
    ModelNode cloned;

    cloned.nameId = nameId;
    cloned.children = children;
    cloned.position = position;
    cloned.orientation = orientation;
    cloned.offset = offset;
    cloned.stretch = stretch;
    cloned.proceduralOffset = proceduralOffset;
    cloned.proceduralRotation = proceduralRotation;
    cloned.type = type;
    cloned.size = size;
    cloned.quadNormalDirection = quadNormalDirection;
    cloned.textureLayout = textureLayout;
    cloned.uvMin = uvMin;
    cloned.uvMax = uvMax;
    cloned.gradientId = gradientId;
    cloned.shadingMode = shadingMode;
    cloned.visible = visible;
    cloned.doubleSided = doubleSided;
    cloned.isPiece = isPiece;

    return cloned;
}

ShadingMode ModelInitializer::parseShadingMode(const std::string& shadingMode) {
    if (shadingMode == "flat") return ShadingMode::Flat;
    if (shadingMode == "fullbright") return ShadingMode::Fullbright;
    if (shadingMode == "reflective") return ShadingMode::Reflective;
    return ShadingMode::Standard;
}

ModelNode::QuadNormal ModelInitializer::parseQuadNormal(const std::string& quadNormal) {
    if (quadNormal == "+Z") return ModelNode::QuadNormal::PlusZ;
    if (quadNormal == "-Z") return ModelNode::QuadNormal::MinusZ;
    if (quadNormal == "+X") return ModelNode::QuadNormal::PlusX;
    if (quadNormal == "-X") return ModelNode::QuadNormal::MinusX;
    if (quadNormal == "+Y") return ModelNode::QuadNormal::PlusY;
    if (quadNormal == "-Y") return ModelNode::QuadNormal::MinusY;
    return ModelNode::QuadNormal::PlusZ;
}

ModelFaceTextureLayout ModelInitializer::getFaceLayout(
    const std::unordered_map<std::string, FaceLayout>& jsonTextureLayout,
    const std::string& faceName) {

    ModelFaceTextureLayout layout;

    auto it = jsonTextureLayout.find(faceName);
    if (it != jsonTextureLayout.end()) {
        const FaceLayout& faceLayout = it->second;
        layout.offset = faceLayout.offset;
        layout.angle = faceLayout.angle;
        layout.mirrorX = faceLayout.mirror.u != 0.0f;
        layout.mirrorY = faceLayout.mirror.v != 0.0f;
        layout.hidden = false;
    }

    return layout;
}