#pragma once
#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>

#include "typeStructs.h"
#include "vkhelper.h"

struct TempTexture
{
	
	std::string path;
	unsigned char* pixelData;
	int width;
	int height;
	int nrChannels;
	VkFormat format;
	VkDeviceSize fileSize;
};

struct Texture
{
	Texture(TempTexture tex)
	{
		width = tex.width;
		height = tex.height;
		mipLevels = std::floor(std::log2(width > height ? width : height)) + 1;

	}
	uint32_t width;
	uint32_t height;
	VkImage image;
	VkImageView view;
	uint32_t mipLevels;
};

class TextureLoader
{
public:
	TextureLoader(Base base);
	uint32_t loadTexture(std::string path);
	void endLoading();

private:
	Base base;
	std::vector<TempTexture> texToLoad;
	std::vector<Texture> textures;
	VkBuffer buffer;
	VkDeviceMemory memory;
};






#endif