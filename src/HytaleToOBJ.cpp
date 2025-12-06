#include "Export.h"
#include <iostream>
#include <string>
#include <cstdlib>

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
        << "\nRequired:\n"
        << "  -w, --world <path>       Path to world file\n"
        << "  -a, --assets <path>      Path to assets folder\n"
        << "  -o, --output <path>      Output directory\n"
        << "  --chunk-min <x,y>        Min chunk coordinates\n"
        << "  --chunk-max <x,y>        Max chunk coordinates\n"
        << "\nOptional:\n"
        << "  -n, --name <name>        Output filename (default: export)\n"
        << "  -h, --help               Show this help\n"
        << "\nExample:\n"
        << "  " << programName << " -w saves/world.hydbs -a hytale/assets -o ./out --chunk-min 0,0 --chunk-max 4,4\n";
}

bool parseChunkCoord(const std::string& str, int& x, int& y) {
    size_t comma = str.find(',');
    if (comma == std::string::npos) return false;

    try {
        x = std::stoi(str.substr(0, comma));
        y = std::stoi(str.substr(comma + 1));
        return true;
    }
    catch (...) {
        return false;
    }
}

bool parseArgs(int argc, char* argv[], ExportConfig& config) {
    config.outputName = "export";  // default

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }

        // Check for a value following the flag
        if (i + 1 >= argc && arg[0] == '-') {
            std::cerr << "Error: " << arg << " requires a value\n";
            return false;
        }

        if (arg == "-w" || arg == "--world") {
            config.worldPath = argv[++i];
        }
        else if (arg == "-a" || arg == "--assets") {
            config.assetsPath = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            config.outputPath = argv[++i];
        }
        else if (arg == "-n" || arg == "--name") {
            config.outputName = argv[++i];
        }
        else if (arg == "--chunk-min") {
            if (!parseChunkCoord(argv[++i], config.chunkMinX, config.chunkMinY)) {
                std::cerr << "Error: Invalid chunk-min format. Use x,y\n";
                return false;
            }
        }
        else if (arg == "--chunk-max") {
            if (!parseChunkCoord(argv[++i], config.chunkMaxX, config.chunkMaxY)) {
                std::cerr << "Error: Invalid chunk-max format. Use x,y\n";
                return false;
            }
        }
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return false;
        }
    }

    // Validate required args
    if (config.worldPath.empty()) {
        std::cerr << "Error: --world is required\n";
        return false;
    }
    if (config.assetsPath.empty()) {
        std::cerr << "Error: --assets is required\n";
        return false;
    }
    if (config.outputPath.empty()) {
        std::cerr << "Error: --output is required\n";
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    ExportConfig config;

    if (!parseArgs(argc, argv, config)) {
        return 1;
    }

    std::cout << "World:      " << config.worldPath << "\n"
        << "Assets:     " << config.assetsPath << "\n"
        << "Output:     " << config.outputPath << "/" << config.outputName << ".obj\n"
        << "Chunk Min:  " << config.chunkMinX << ", " << config.chunkMinY << "\n"
        << "Chunk Max:  " << config.chunkMaxX << ", " << config.chunkMaxY << "\n";

    Export worldExport(&config);
    worldExport.exportWorld();

    return 0;
}
