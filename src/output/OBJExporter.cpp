#include "OBJExporter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

bool OBJExporter::exportMesh(const Mesh& mesh, const std::string& filename, 
    const std::string& assetsPath, const TextureRegistry* textureRegistry, const OBJExportOptions& options) {
	std::vector<Mesh> meshes = { mesh };
	return exportMeshes(meshes, filename, assetsPath, textureRegistry, options);
}

bool OBJExporter::exportMeshes(const std::vector<Mesh>& meshes, const std::string& filename,
    const std::string& assetsPath, const TextureRegistry* textureRegistry, const OBJExportOptions& options) {
    if (meshes.empty()) {
        std::cerr << "No meshes to export" << std::endl;
        return false;
    }

    std::string baseName = filename;
    if (baseName.size() > 4 && baseName.substr(baseName.size() - 4) == ".obj") {
        baseName = baseName.substr(0, baseName.size() - 4);
    }

    // Ensure output directory has trailing slash
    std::string outputDir = options.outputDirectory;
    if (!outputDir.empty() && outputDir.back() != '/' && outputDir.back() != '\\') {
        outputDir += '/';
    }

    std::string objPath = outputDir + baseName + ".obj";
    std::string mtlFilename = baseName + ".mtl";
    std::string mtlPath = outputDir + mtlFilename;

    std::ofstream objFile(objPath);
    if (!objFile.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;
        return false;
    }

    if (!writeOBJ(objFile, meshes, mtlFilename, options)) {
        objFile.close();
        return false;
    }
    objFile.close();

    if (options.exportMTL) {
        if (!writeMTL(mtlPath, meshes, textureRegistry, options)) {
            return false;
        }

        if (options.exportTextures) {
            std::string texturePath = outputDir + baseName + "_atlas.png";
            if (!exportTextureAtlas(textureRegistry, assetsPath, texturePath)) {
                std::cerr << "Warning: Failed to export texture atlas" << std::endl;
            }
        }
    }

    return true;
}

bool OBJExporter::writeOBJ(std::ofstream& file,
    const std::vector<Mesh>& meshes,
    const std::string& mtlFilename,
    const OBJExportOptions& options) {
    file << "# Voxel Mesh Export" << std::endl;
    file << "# Meshes: " << meshes.size() << std::endl;
    file << std::endl;

    if (options.exportMTL) {
        file << "mtllib " << mtlFilename << std::endl;
        file << std::endl;
    }

    uint32_t vertexOffset = 0;

    for (size_t meshIdx = 0; meshIdx < meshes.size(); ++meshIdx) {
        const Mesh& mesh = meshes[meshIdx];

        file << "# Mesh " << (meshIdx + 1) << std::endl;
        file << "o mesh_" << meshIdx << std::endl;
        file << std::endl;

        // Write vertices
        file << "# Vertices: " << mesh.vertices.size() << std::endl;
        for (const auto& vertex : mesh.vertices) {
            file << "v "
                << vertex.position.x << " "
                << vertex.position.y << " "
                << vertex.position.z << std::endl;
        }
        file << std::endl;

        // Write texture coordinates
        file << "# Texture coordinates: " << mesh.vertices.size() << std::endl;
        for (const auto& vertex : mesh.vertices) {
            float v = options.flipVCoordinate ? (1.0f - vertex.uv.v) : vertex.uv.v;
            file << "vt " << vertex.uv.u << " " << v << std::endl;
        }
        file << std::endl;

        // Write normals
        file << "# Normals: " << mesh.vertices.size() << std::endl;
        for (const auto& vertex : mesh.vertices) {
            file << "vn "
                << vertex.normal.x << " "
                << vertex.normal.y << " "
                << vertex.normal.z << std::endl;
        }
        file << std::endl;

        // Write faces
        file << "# Faces: " << mesh.faces.size() << std::endl;
        std::string currentMaterial = "";

        for (const auto& face : mesh.faces) {
            // Switch material if needed
            if (!face.material.empty() && face.material != currentMaterial) {
                file << "usemtl " << face.material << std::endl;
                currentMaterial = face.material;
            }
            else if (face.material.empty() && !mesh.materialName.empty() &&
                mesh.materialName != currentMaterial) {
                file << "usemtl " << mesh.materialName << std::endl;
                currentMaterial = mesh.materialName;
            }

            file << "f";
            for (uint8_t i = 0; i < face.vertexCount; ++i) {
                uint32_t idx = face.indices[i] + vertexOffset + 1; // OBJ is 1-indexed
                file << " " << idx << "/" << idx << "/" << idx;
            }
            file << std::endl;
        }
        file << std::endl;

        vertexOffset += static_cast<uint32_t>(mesh.vertices.size());
    }

    return true;
}

bool OBJExporter::writeMTL(const std::string& filename,
    const std::vector<Mesh>& meshes,
    const TextureRegistry* textureRegistry,
    const OBJExportOptions& options) {
    std::ofstream mtlFile(filename);
    if (!mtlFile.is_open()) {
        std::cerr << "Failed to open MTL file: " << filename << std::endl;
        return false;
    }

    mtlFile << "# Material Library" << std::endl;
    mtlFile << std::endl;

    // Collect unique materials
    std::unordered_map<std::string, bool> writtenMaterials;

    for (const auto& mesh : meshes) {
        // Add mesh-level material
        if (!mesh.materialName.empty() &&
            writtenMaterials.find(mesh.materialName) == writtenMaterials.end()) {
            writtenMaterials[mesh.materialName] = true;
        }

        // Add face-level materials
        for (const auto& face : mesh.faces) {
            if (!face.material.empty() &&
                writtenMaterials.find(face.material) == writtenMaterials.end()) {
                writtenMaterials[face.material] = true;
            }
        }
    }

    // Write materials
    std::string atlasTexture = options.exportTextures ?
        (filename.substr(0, filename.size() - 4) + "_atlas.png") : "";

    for (const auto& pair : writtenMaterials) {
        const std::string& matName = pair.first;

        mtlFile << "newmtl " << matName << std::endl;
        mtlFile << "Ka 1.000 1.000 1.000" << std::endl;  // Ambient
        mtlFile << "Kd 1.000 1.000 1.000" << std::endl;  // Diffuse
        mtlFile << "Ks 0.000 0.000 0.000" << std::endl;  // Specular
        mtlFile << "d 1.0" << std::endl;                  // Dissolve (opacity)
        mtlFile << "illum 1" << std::endl;                // Illumination model

        if (!atlasTexture.empty()) {
            // Extract just the filename from the full path
            size_t lastSlash = atlasTexture.find_last_of("/\\");
            std::string texFilename = (lastSlash != std::string::npos) ?
                atlasTexture.substr(lastSlash + 1) : atlasTexture;
            mtlFile << "map_Kd " << texFilename << std::endl;
        }

        mtlFile << std::endl;
    }

    mtlFile.close();
    return true;
}

bool OBJExporter::exportTextureAtlas(const TextureRegistry* textureRegistry, const std::string& assetsPath,
    const std::string& filename) {

    if (!textureRegistry) {
        std::cerr << "Error: TextureRegistry is null" << std::endl;
        return false;
    }

    textureRegistry->exportAtlas(filename);
    return true;
}