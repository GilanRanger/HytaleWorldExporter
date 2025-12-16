#include "MeshBuilder.h"
#include <stdexcept>
#include <algorithm>
#include "../data/Model.h"
#include "../output/stb/stb_image.h"



void TextureRegistry::addTexture(const std::string& name, const std::string& filepath) {
	int width, height, channels;
	uint8_t* imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

	if (!imageData) {
		throw std::runtime_error("Failed to load texture: " + filepath);
	}

	textureSources[name] = {
		.name = name,
		.data = imageData,
		.width = static_cast<unsigned int>(width),
		.height = static_cast<unsigned int>(height),
		.channels = static_cast<unsigned int>(channels)
	};
}

void TextureRegistry::packTextures() {
	// pack into one texture, create atlas regions, set pixelData
	
	// Sort textures from largest to smallest
	std::vector<TextureSource*> sortedSources;
	sortedSources.reserve(textureSources.size());
	for (auto& pair : textureSources) {
		sortedSources.push_back(&pair.second);
	}

	std::sort(sortedSources.begin(), sortedSources.end(),
		[](const TextureSource* a, const TextureSource* b) {
			// Sort by height first, then width
			if (a->height != b->height) {
				return a->height > b->height;
			}
			return a->width > b->width;
		}
	);

	struct Shelf {
		uint32_t y;
		uint32_t height;
		uint32_t currentX;
	};

	std::vector<Shelf> shelves;
	uint32_t totalAtlasHeight = 0;

	for (auto& source : sortedSources) {
		bool placed = false;

		// Try for current shelf
		for (auto& shelf : shelves) {
			if (shelf.currentX + source->width <= atlasWidth) {
				copyTextureToAtlas(source->data, source->width, source->height,
					source->channels, shelf.currentX, shelf.y);

				textureRegions[source->name] = {
					.uvMin = Vec2(shelf.currentX, shelf.y),
					.uvMax = Vec2(shelf.currentX + source->width, shelf.y + source->height),
					.pixelWidth = source->width,
					.pixelHeight = source->height,
				};

				shelf.currentX += source->width;
				placed = true;
				break;
			}
		}

		// New shelf
		if (!placed && (totalAtlasHeight + source->height) < atlasHeight) {
			Shelf newShelf;
			newShelf.y = totalAtlasHeight;
			newShelf.height = source->height;
			newShelf.currentX = source->width;

			copyTextureToAtlas(source->data, source->width, source->height,
				source->channels, 0, newShelf.y);

			textureRegions[source->name] = {
					.uvMin = Vec2(newShelf.currentX, newShelf.y),
					.uvMax = Vec2(newShelf.currentX + source->width, newShelf.y + source->height),
					.pixelWidth = source->width,
					.pixelHeight = source->height,
			};

			shelves.push_back(newShelf);
			totalAtlasHeight += source->height;
		}
	}

	// Clean up sources
	for (auto& [name, source] : textureSources) {
		stbi_image_free(source.data);
	}
	textureSources.clear();
}

void TextureRegistry::copyTextureToAtlas(const uint8_t* srcData, uint32_t srcWidth, uint32_t srcHeight,
	uint32_t srcChannels, uint32_t dstX, uint32_t dstY) {

	for (uint32_t y = 0; y < srcHeight; ++y) {
		for (uint32_t x = 0; x < srcWidth; ++x) {
			uint32_t srcIdx = (y * srcWidth + x) * srcChannels;
			uint32_t dstIdx = ((dstY + y) * atlasWidth + (dstX + x)) * 4; //RGBA

			pixelData[dstIdx + 0] = srcChannels > 0 ? srcData[srcIdx + 0] : 0;  // R
			pixelData[dstIdx + 1] = srcChannels > 1 ? srcData[srcIdx + 1] : 0;  // G
			pixelData[dstIdx + 2] = srcChannels > 2 ? srcData[srcIdx + 2] : 0;  // B
			pixelData[dstIdx + 3] = srcChannels > 3 ? srcData[srcIdx + 3] : 255; // A
		}
	}
}

void TextureRegistry::exportAtlas(const std::string& outputPath) {

}

bool TextureRegistry::getTextureUVs(const std::string& name, Vec2& uvMin, Vec2& uvMax) const {

}

void TextureRegistry::loadMetadata(const std::string& metadataPath) {

}

void TextureRegistry::saveMetadata(const std::string& metadataPath) const {

}