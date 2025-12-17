#include "Export.h"
#include "data/Model.h"
#include "geometry/MeshBuilder.h"
#include "output/OBJExporter.h"
#include <iostream>

Export::Export(ExportConfig* config) : config(config) {}

void Export::exportWorld()
{
    // Get world file
    // Parse world file
    World world;
    std::vector<ChunkPos> selectedChunks;

    // Gamedata/Worlddata
        // Block id mappings
    BlockIDMappings blockIDMappings;
    // Parse and populate blockIDMappings

    // For each selected chunk in data
        // Create an empty Chunk struct
        // Fill with PackedBlocks (blockID + state)
        // Fill with block entities
        // Store in world.chunks

    // Parse assets to create BlockModels
    ModelRegistry blockModelRegistry;
    TextureRegistry textureRegistry(2048,2048,32);

    // For each block type that appears in selected chunks:
        // Load its model(s), add to model registry
        // Collect texture references, add to texture registry

    // Generate geometry
    MeshBuilder meshBuilder(&blockModelRegistry, &blockIDMappings, &textureRegistry);

    std::vector<Mesh> chunkMeshes;

    for (const ChunkPos& chunkPos : selectedChunks) {
        ChunkColumn* chunk = world.getChunk(chunkPos);
        if (!chunk) continue;

        Mesh chunkMesh;
        meshBuilder.generateChunkMesh(&world, chunk, chunkMesh);

        chunkMeshes.push_back(chunkMesh);
    }

    // Export OBJ file, MTL file, and atlas PNG
    std::string outputFilename = config->outputName + ".obj";
    OBJExporter::OBJExportOptions options;
    options.outputDirectory = config->outputPath;
    options.exportMTL = true;
    options.exportTextures = true;
    options.flipVCoordinate = true;

    OBJExporter exporter;
    bool success = exporter.exportMeshes(chunkMeshes, outputFilename, config->assetsPath, &textureRegistry, options);

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
