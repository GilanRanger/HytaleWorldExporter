#pragma once
#include "../data/MeshData.h"
#include <string>
#include <fstream>
#include <vector>

class OBJExporter {
public:
	struct OBJExportOptions {
		bool exportMTL = true;
		bool exportTextures = true;
		bool flipVCoordinate = true;
		std::string outputDirectory = "./";

		OBJExportOptions() = default;
	};

	static bool exportMesh(const Mesh& mesh, const std::string& filename, 
		const std::string& assetsPath, const TextureAtlas* atlas = nullptr, 
		const OBJExportOptions& options = OBJExportOptions());

	static bool exportMeshes(const std::vector<Mesh>& meshes, const std::string& filename, 
		const std::string& assetsPath, const TextureAtlas* atlas = nullptr, 
		const OBJExportOptions& options = OBJExportOptions());

private:
	static bool writeOBJ(std::ofstream& file, const std::vector<Mesh>& meshes,
		const std::string& mtlFilename, const OBJExportOptions& options);

	static bool writeMTL(const std::string& filename, const std::vector<Mesh>& meshes,
		const TextureAtlas* atlas, const OBJExportOptions& options);

	static bool exportTextureAtlas(const TextureAtlas& atlas, const std::string& assetsPath,
		const std::string& filename);
};