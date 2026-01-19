#include "Export.h"
#include "data/Model.h"
#include "geometry/PrefabMesher.h"
#include "parse/HytalePrefabParser.h"
#include "output/OBJExporter.h"
#include <iostream>

Export::Export(ExportConfig* config) : config(config) {}

void Export::exportPrefab()
{
    ModelRegistry blockModelRegistry(config->assetsPath);
    TextureRegistry textureRegistry(2048, 2048, 32);

    auto prefab = PrefabLoader::loadFromFile(config->prefabPath);
    if (!prefab) {
        std::cerr << "Failed to load prefab: " << config->prefabPath << "\n";
        return;
    }

    // Get unique block types in prefab
    std::unordered_set<std::string> uniqueBlockTypes = prefab->getUniqueBlockTypes();

    std::cout << "Loading " << uniqueBlockTypes.size() << " unique block types...\n";

    // For each unique block type: load model and texture
    for (const std::string& blockName : uniqueBlockTypes) {
        std::cout << "  Loading: " << blockName << "\n";

        // Load model
        Model* model = blockModelRegistry.loadModel(blockName);
        if (!model) {
            std::cerr << "    Warning: Could not load model for " << blockName << "\n";
        }

        // Load texture
        std::string texturePath = blockModelRegistry.findTexturePath(blockName);
        if (!texturePath.empty()) {
            textureRegistry.addTexture(blockName, texturePath);
        }
        else {
            std::cerr << "    Warning: Could not find texture for " << blockName << "\n";
        }
    }

    // Pack textures into atlas
    std::cout << "Packing textures into atlas...\n";
    textureRegistry.packTextures();

    // Generate mesh
    std::cout << "Generating mesh...\n";
    PrefabMesher prefabMesher(&blockModelRegistry, &textureRegistry);
    Mesh prefabMesh;
    prefabMesher.generatePrefabMesh(*prefab, prefabMesh);

    std::vector<Mesh> meshes = { prefabMesh };

    // Export
    std::cout << "Exporting...\n";
    std::string outputFilename = config->outputName + ".obj";
    OBJExporter::OBJExportOptions options;
    options.outputDirectory = config->outputPath;
    options.exportMTL = true;
    options.exportTextures = true;
    options.flipVCoordinate = true;

    OBJExporter exporter;
    bool success = exporter.exportMeshes(meshes, outputFilename, config->assetsPath, &textureRegistry, options);

    if (success) {
        std::cout << "Export complete!\n";
        std::cout << "  OBJ: " << config->outputPath << "/" << outputFilename << "\n";
        std::cout << "  MTL: " << config->outputPath << "/" << config->outputName << ".mtl\n";
        if (options.exportTextures) {
            std::cout << "  Texture: " << config->outputPath << "/"
                << config->outputName << "_atlas.png\n";
        }
    }
    else {
        std::cerr << "Export failed!\n";
    }
}