#include "../data/BlockModel.h"

BlockModelRegistry::BlockModelRegistry() {}

BlockModel* BlockModelRegistry::loadModel(const std::string& modelName) {
	// Will load the model from the assets folder into a BlockModel
	return nullptr;
}

BlockModel* BlockModelRegistry::getModel(const std::string& modelName) {
	auto it = models.find(modelName);
	if (it != models.end()) {
		return &it->second;
	}
	return nullptr;
}

bool BlockModelRegistry::hasModel(const std::string& modelName) const {
	return models.find(modelName) != models.end();
}

void BlockModelRegistry::createDefaultCubeModel() {
	BlockModel cubeModel;
	cubeModel.modelName = "cube";
	cubeModel.isFullCube = true;
	cubeModel.boundsMin = Vec3(0, 0, 0);
	cubeModel.boundsMax = Vec3(1, 1, 1);

	cubeModel.faceTextures["north"] = "default";
	cubeModel.faceTextures["south"] = "default";
	cubeModel.faceTextures["east"] = "default";
	cubeModel.faceTextures["west"] = "default";
	cubeModel.faceTextures["top"] = "default";
	cubeModel.faceTextures["bottom"] = "default";

	models["cube"] = cubeModel;
}
