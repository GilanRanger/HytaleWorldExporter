#pragma once
#include <string>

struct ExportConfig {
	std::string worldPath;
	std::string assetsPath;
	std::string outputPath;
	std::string outputName;
	int chunkMinX = 0;
	int chunkMinY = 0;
	int chunkMaxX = 0;
	int chunkMaxY = 0;
};

class Export {
public:
	Export(ExportConfig* config);

	void exportWorld();
private:
	ExportConfig* config;
};