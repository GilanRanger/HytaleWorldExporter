#pragma once
#include "../data/MeshData.h"
#include <string>
#include <fstream>
#include <vector>

class OBJExporter {
public:
	struct ExportOptions {
		bool exportMTL = true;
		bool exportTextures = true;
		bool flipVCoordinate = true;
		std::string outputDirectory = "./";

		ExportOptions() = default;
	};

	static bool exportMesh(const Mesh& mesh, const std::string& filename,
		const TextureAtlas* atlas = nullptr, const ExportOptions& options = ExportOptions());

	static bool exportMeshes(const std::vector<Mesh>& meshes, const std::string& filename,
		const TextureAtlas* atlas = nullptr, const ExportOptions& options = ExportOptions());

private:
	static bool writeOBJ(std::ofstream& file, const std::vector<Mesh>& meshes,
		const std::string& mtlFilename, const ExportOptions& options);

	static bool writeMTL(const std::string& filename, const std::vector<Mesh>& meshes,
		const TextureAtlas* atlas, const ExportOptions& options);

	static bool exportTextureAtlas(const TextureAtlas& atlas, const std::string& filename);
};