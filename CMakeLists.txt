# CMakeList.txt : CMake project for HytaleToOBJ, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HytaleToOBJ")

# Add source to this project's executable.
add_executable (HytaleToOBJ "src/HytaleToOBJ.cpp"  "src/data/WorldData.h" "src/data/MeshData.h" "src/data/BlockModel.h" "src/geometry/MeshBuilder.h" "src/geometry/MeshBuilder.cpp" "src/geometry/BlockModelRegistry.cpp" "src/output/OBJExporter.h" "src/output/OBJExporter.cpp" "src/output/stb/stb_impl.cpp" "src/Export.h" "src/Export.cpp" "src/geometry/TextureRegistry.cpp" "src/geometry/BlockIDMappings.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HytaleToOBJ PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
