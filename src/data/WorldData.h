#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#define CHUNK_SIZE_X 32
#define CHUNK_SIZE_Y 320
#define CHUNK_SIZE_Z 32
#define CHUNK_VOLUME (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z)

struct Chunk;
struct BlockEntity;

// bits 0-15:  BlockID
// bits 16-31: BlockState
typedef uint32_t PackedBlock;

inline PackedBlock packBlock(uint16_t blockID, uint16_t state) {
    return static_cast<uint32_t>(blockID) | (static_cast<uint32_t>(state) << 16);
}

inline uint16_t getBlockID(PackedBlock packed) {
    return static_cast<uint16_t>(packed & 0xFFFF);
}

inline uint16_t getBlockState(PackedBlock packed) {
    return static_cast<uint16_t>((packed >> 16) & 0xFFFF);
}

struct BlockPos {
    int32_t x, y, z;

    bool operator==(const BlockPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct ChunkPos {
    int32_t x, z;

    bool operator==(const ChunkPos& other) const {
        return x == other.x && z == other.z;
    }
};

namespace std {
    template<>
    struct hash<ChunkPos> {
        size_t operator()(const ChunkPos& pos) const {
            uint64_t a = static_cast<uint64_t>(pos.x);
            uint64_t b = static_cast<uint64_t>(pos.z);
            return static_cast<size_t>((a << 32) | (b & 0xFFFFFFFF));
        }
    };
}

struct BlockEntity {
    std::string entityType;
    std::unordered_map<std::string, std::string> data;
};

struct Chunk {
    ChunkPos position;
    PackedBlock blocks[CHUNK_VOLUME];
    std::unordered_map<uint32_t, BlockEntity> blockEntities;

    inline uint32_t getIndex(uint8_t x, uint16_t y, uint8_t z) const {
        return x + z * CHUNK_SIZE_X + y * CHUNK_SIZE_X * CHUNK_SIZE_Z;
    }

    inline PackedBlock getPackedBlock(uint8_t x, uint8_t y, uint8_t z) const {
        return blocks[getIndex(x, y, z)];
    }

    inline uint16_t getBlockID(uint8_t x, uint8_t y, uint8_t z) const {
        return ::getBlockID(blocks[getIndex(x, y, z)]);
    }

    inline uint16_t getBlockState(uint8_t x, uint8_t y, uint8_t z) const {
        return ::getBlockState(blocks[getIndex(x, y, z)]);
    }

    inline void setBlock(uint8_t x, uint8_t y, uint8_t z, uint16_t blockID, uint16_t state = 0) {
        blocks[getIndex(x, y, z)] = packBlock(blockID, state);
    }

    inline void setPackedBlock(uint8_t x, uint8_t y, uint8_t z, PackedBlock packed) {
        blocks[getIndex(x, y, z)] = packed;
    }

    inline void setBlockState(uint8_t x, uint8_t y, uint8_t z, uint16_t state) {
        uint32_t idx = getIndex(x, y, z);
        blocks[idx] = packBlock(::getBlockID(blocks[idx]), state);
    }

    inline bool hasBlockEntity(uint8_t x, uint8_t y, uint8_t z) const {
        return blockEntities.find(getIndex(x, y, z)) != blockEntities.end();
    }

    inline BlockEntity* getBlockEntity(uint8_t x, uint8_t y, uint8_t z) {
        auto it = blockEntities.find(getIndex(x, y, z));
        return it != blockEntities.end() ? &it->second : nullptr;
    }

    inline const BlockEntity* getBlockEntity(uint8_t x, uint8_t y, uint8_t z) const {
        auto it = blockEntities.find(getIndex(x, y, z));
        return it != blockEntities.end() ? &it->second : nullptr;
    }

    inline void setBlockEntity(uint8_t x, uint8_t y, uint8_t z, const BlockEntity& entity) {
        blockEntities[getIndex(x, y, z)] = entity;
    }

    inline void removeBlockEntity(uint8_t x, uint8_t y, uint8_t z) {
        blockEntities.erase(getIndex(x, y, z));
    }
};

struct World {
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> chunks;
    std::string worldName;

    Chunk* getChunk(const ChunkPos& pos) const {
        auto it = chunks.find(pos);
        return it != chunks.end() ? it->second.get() : nullptr;
    }

    Chunk* getChunk(int32_t chunkX, int32_t chunkZ) const {
        return getChunk({ chunkX, chunkZ });
    }

    Chunk* createChunk(const ChunkPos& pos) {
        auto chunk = std::make_unique<Chunk>();
        chunk->position = pos;
        auto* ptr = chunk.get();
        chunks[pos] = std::move(chunk);
        return ptr;
    }

    Chunk* createChunk(int32_t chunkX, int32_t chunkZ) {
        return createChunk({ chunkX, chunkZ });
    }

    Chunk* getOrCreateChunk(const ChunkPos& pos) {
        auto* existing = getChunk(pos);
        return existing ? existing : createChunk(pos);
    }

    Chunk* getOrCreateChunk(int32_t chunkX, int32_t chunkZ) {
        return getOrCreateChunk({ chunkX, chunkZ });
    }

    void removeChunk(const ChunkPos& pos) {
        chunks.erase(pos);
    }

    void removeChunk(int32_t chunkX, int32_t chunkZ) {
        removeChunk({ chunkX, chunkZ });
    }

    PackedBlock getPackedBlockAt(int32_t x, int32_t y, int32_t z) const {
        int32_t chunkX = x >> 5;
        int32_t chunkZ = z >> 5;

        Chunk* chunk = getChunk(chunkX, chunkZ);
        if (!chunk) return 0;

        uint8_t localX = x & 31;
        uint8_t localZ = z & 31;

        return chunk->getPackedBlock(localX, y, localZ);
    }

    uint16_t getBlockIDAt(int32_t x, int32_t y, int32_t z) const {
        return ::getBlockID(getPackedBlockAt(x, y, z));
    }

    uint16_t getBlockStateAt(int32_t x, int32_t y, int32_t z) const {
        return ::getBlockState(getPackedBlockAt(x, y, z));
    }

    void setBlockAt(int32_t x, int32_t y, int32_t z, uint16_t blockID, uint16_t state = 0) {
        int32_t chunkX = x >> 5;
        int32_t chunkZ = z >> 5;

        Chunk* chunk = getChunk(chunkX, chunkZ);
        if (!chunk) return;

        uint8_t localX = x & 31;
        uint8_t localZ = z & 31;

        chunk->setBlock(localX, y, localZ, blockID, state);
    }
};