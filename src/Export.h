#pragma once
#include "HytaleToOBJ.cpp"
#include <string>

class Export {
public:
	Export(ExportConfig* config);

	void exportWorld();
private:
	ExportConfig* config;
};