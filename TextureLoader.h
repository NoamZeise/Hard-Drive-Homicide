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


struct TempTexture
{
	std::string path;
	unsigned char* pixelData;
	int width;
	int height;
	int nrChannels;
	VkFormat format;
};

struct Texture
{
	VkImage image;
	VkImageView view;
};

class TextureLoader
{
public:
	TextureLoader(VkDevice device);
	uint32_t loadTexture(std::string path);
	void endLoading();

private:
	VkDevice device;
	std::vector<TempTexture> texToLoad;
	std::vector<Texture> textures;
	VkBuffer buffer;
	VkDeviceMemory memory;
};






#endif