#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#define SECTION_WIDTH 32
#define SECTION_HEIGHT 32
#define SECTION_VOLUME (SECTION_WIDTH * SECTION_WIDTH * SECTION_HEIGHT)
#define MAX_SECTIONS_PER_COLUMN 10

struct ChunkColumn;

// The "Global" BlockData value, after unpacking
// bits 0-15:  Global BlockID
// bits 16-31: BlockState
typedef uint32_t ResolvedBlock;

inline ResolvedBlock packBlock(uint16_t globalBlockID, uint16_t state) {
    return static_cast<uint32_t>(globalBlockID) | (static_cast<uint32_t>(state) << 16);
}

inline uint16_t getGlobalBlockID(ResolvedBlock resolved) {
    return static_cast<uint16_t>(resolved & 0xFFFF);
}

inline uint16_t getBlockState(ResolvedBlock resolved) {
    return static_cast<uint16_t>((resolved >> 16) & 0xFFFF);
}

// Temporary stand-in for block entities (containers)
struct BlockEntity {
    std::string entityType;
    std::unordered_map<std::string, std::string> metadata;
};

struct ChunkSection {
	int bitsPerBlock = 0;

	std::vector<uint16_t> localToGlobal;
	std::vector<uint32_t> packedData;

    ResolvedBlock blocks[SECTION_VOLUME] = { 0 };
	
	bool isResolved = false;

	inline uint32_t getLocalIdx(uint8_t x, uint8_t y, uint8_t z) const {
		return x + (z * SECTION_WIDTH) + (y * SECTION_WIDTH * SECTION_WIDTH);
	}
};

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

struct ChunkPosHash {
    size_t operator()(const ChunkPos& pos) const {
        return (static_cast<size_t>(pos.x) << 32) | (static_cast<uint32_t>(pos.z));
    }
};

struct ChunkColumn {
	ChunkPos position;

	std::unique_ptr<ChunkSection> sections[MAX_SECTIONS_PER_COLUMN];
    std::unordered_map<uint32_t, BlockEntity> blockEntities;

    inline uint32_t getGlobalIndex(uint8_t x, uint16_t y, uint8_t z) const {
        return x + (z * SECTION_WIDTH) + (y * SECTION_WIDTH * SECTION_WIDTH);
    }

    ResolvedBlock getBlockAt(uint8_t x, uint16_t y, uint8_t z) const {
		int sectionIdx = y / SECTION_HEIGHT;
		if (sectionIdx >= MAX_SECTIONS_PER_COLUMN || !sections[sectionIdx]) return 0;

		return sections[sectionIdx]->blocks[sections[sectionIdx]->getLocalIdx(x, y % SECTION_HEIGHT, z)];
	}

    void setBlock(uint8_t x, uint16_t y, uint8_t z, uint16_t blockID, uint16_t state = 0) {
        int sectionIdx = y / SECTION_HEIGHT;
        if (sectionIdx < 0 || sectionIdx >= MAX_SECTIONS_PER_COLUMN) return;

        if (!sections[sectionIdx]) {
            sections[sectionIdx] = std::make_unique<ChunkSection>();
        }

        uint8_t localY = static_cast<uint8_t>(y % SECTION_HEIGHT);
        uint32_t localIdx = sections[sectionIdx]->getLocalIdx(x, localY, z);

        sections[sectionIdx]->blocks[localIdx] = packBlock(blockID, state);
    }

    void setBlockEntity(uint8_t x, uint16_t y, uint8_t z, const BlockEntity& entity) {
        blockEntities[getGlobalIndex(x, y, z)] = entity;
    }

    const BlockEntity* getBlockEntity(uint8_t x, uint16_t y, uint8_t z) const {
        auto it = blockEntities.find(getGlobalIndex(x, y, z));
        return (it != blockEntities.end()) ? &it->second : nullptr;
    }
};

struct World {
    std::unordered_map<ChunkPos, std::unique_ptr<ChunkColumn>, ChunkPosHash> chunks;
    std::string worldName;

    ChunkColumn* getChunk(const ChunkPos& pos) const {
        auto it = chunks.find(pos);
        return it != chunks.end() ? it->second.get() : nullptr;
    }

    ChunkColumn* getChunk(int32_t chunkX, int32_t chunkZ) const {
        return getChunk({ chunkX, chunkZ });
    }

    ResolvedBlock getResolvedBlockAt(int32_t x, int32_t y, int32_t z) const {
        ChunkColumn* chunk = getChunk(x >> 5, z >> 5);
        if (!chunk) return 0;
        return chunk->getBlockAt(x & 31, y, z & 31);
    }

    uint16_t getBlockIDAt(int32_t x, int32_t y, int32_t z) const {
        return static_cast<uint16_t>(getResolvedBlockAt(x, y, z) & 0xFFFF);
    }

    void setBlockAt(int32_t x, int32_t y, int32_t z, uint16_t blockID, uint16_t state = 0) {
        ChunkColumn* chunk = getChunk(x >> 5, z >> 5);
        if (!chunk) return;

        chunk->setBlock(x & 31, y, z & 31, blockID, state);
    }

    const BlockEntity* getBlockEntityAt(int32_t x, int32_t y, int32_t z) const {
        ChunkColumn* chunk = getChunk(x >> 5, z >> 5);
        return chunk ? chunk->getBlockEntity(x & 31, y, z & 31) : nullptr;
    }

    void setBlockEntityAt(int32_t x, int32_t y, int32_t z, const BlockEntity& entity) {
        ChunkColumn* chunk = getChunk(x >> 5, z >> 5);
        if (chunk) chunk->setBlockEntity(x & 31, y, z & 31, entity);
    }

    ChunkColumn* createChunk(const ChunkPos& pos) {
        auto chunk = std::make_unique<ChunkColumn>();
        chunk->position = pos;
        auto* ptr = chunk.get();
        chunks[pos] = std::move(chunk);
        return ptr;
    }

    ChunkColumn* getOrCreateChunk(const ChunkPos& pos) {
        auto* existing = getChunk(pos);
        return existing ? existing : createChunk(pos);
    }
};