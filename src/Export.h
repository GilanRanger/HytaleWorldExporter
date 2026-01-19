#pragma once
#include <string>

struct ExportConfig {
	std::string prefabPath;
	std::string assetsPath;
	std::string outputPath;
	std::string outputName;
};

class Export {
public:
	Export(ExportConfig* config);

	void exportPrefab();
private:
	ExportConfig* config;
};