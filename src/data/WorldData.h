#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#define CHUNK_SIZE_X 32
#define CHUNK_SIZE_Y 32
#define CHUNK_SIZE_Z 320

struct Chunk;
struct BlockEntity;

typedef uint16_t BlockID;
typedef uint16_t BlockState;

struct BlockPos {
    int32_t x, y, z;
};

struct ChunkPos {
    int32_t x, y, z;
};

struct World {
    std::unordered_map<int64_t, Chunk*> chunks;
    std::string worldName;

    Chunk* getChunk(int32_t chunkX, int32_t chunkY, int32_t chunkZ) const;
    BlockID getBlockAt(int32_t x, int32_t y, int32_t z) const;
};

struct Chunk {
    ChunkPos position;

    // Block data
    BlockID blocks[CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z];
    BlockState blockStates[CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z];

    // Block entities
    std::unordered_map<uint32_t, BlockEntity> blockEntities;

    inline uint32_t getIndex(uint8_t x, uint8_t y, uint8_t z) const {
        return x + z * CHUNK_SIZE_X + y * CHUNK_SIZE_X * CHUNK_SIZE_Z;
    }

    inline BlockID getBlock(uint8_t x, uint8_t y, uint8_t z) const {
        return blocks[getIndex(x, y, z)];
    }

    inline BlockState getBlockState(uint8_t x, uint8_t y, uint8_t z) const {
        return blockStates[getIndex(x, y, z)];
    }
};

// Block entity for complex blocks
struct BlockEntity {
    std::string entityType;
    std::unordered_map<std::string, std::string> data;
};