#include "Export.h"
#include <iostream>
#include <string>

/*
HytaleWorldExporter.exe -p C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\Simple_Test.prefab.json -a C:\Users\brend\Desktop\Hytale\templates\Assets -o C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\output -n simple
*/

/*
HytaleWorldExporter.exe -p C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\Model_Test.prefab.json -a C:\Users\brend\Desktop\Hytale\templates\Assets -o C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\output -n complex
*/

/*
HytaleWorldExporter.exe -p C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\OasisSection.prefab.json -a C:\Users\brend\Desktop\Hytale\templates\Assets -o C:\Users\brend\Desktop\Hytale\external_tools\HytaleWorldExporter\test\output -n oasis
*/

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
        << "\nRequired:\n"
        << "  -p, --prefab <path>      Path to prefab.json file\n"
        << "  -a, --assets <path>      Path to game assets folder\n"
        << "  -o, --output <path>      Output directory\n"
        << "\nOptional:\n"
        << "  -n, --name <name>        Output filename (default: prefab)\n"
        << "  -h, --help               Show this help\n"
        << "\nExample:\n"
        << "  " << programName << " -p house.prefab.json -a C:/User/me/unzippedHytale/Assets -o ./out\n";
}

bool parseArgs(int argc, char* argv[], ExportConfig& config) {
    config.outputName = "prefab";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }

        if (i + 1 >= argc && arg[0] == '-') {
            std::cerr << "Error: " << arg << " requires a value\n";
            return false;
        }

        if (arg == "-p" || arg == "--prefab") {
            config.prefabPath = argv[++i];
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
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return false;
        }
    }

    if (config.prefabPath.empty()) {
        std::cerr << "Error: --prefab is required\n";
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

    std::cout << "Prefab:     " << config.prefabPath << "\n"
        << "Assets:     " << config.assetsPath << "\n"
        << "Output:     " << config.outputPath << "/" << config.outputName << ".obj\n";

    Export prefabExport(&config);
    prefabExport.exportPrefab();

    return 0;
}