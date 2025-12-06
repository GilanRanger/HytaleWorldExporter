#include "../data/Model.h"

ModelRegistry::ModelRegistry() {}

ModelRegistry::~ModelRegistry() {}

Model* ModelRegistry::loadModel(const std::string& modelName) {
	// Will load the model from the assets folder into a BlockModel
	return nullptr;
}

Model* ModelRegistry::getModel(const std::string& modelName) {
	auto it = models.find(modelName);
	if (it != models.end()) {
		return it->second;
	}
	return nullptr;
}

bool ModelRegistry::hasModel(const std::string& modelName) const {
	return models.find(modelName) != models.end();
}

