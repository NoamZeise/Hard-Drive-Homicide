#include "TextureLoader.h"

TextureLoader::TextureLoader(VkDevice device)
{
	this->device = device;
}

uint32_t TextureLoader::loadTexture(std::string path)
{
	texToLoad.push_back({ path });
	TempTexture* tex = &texToLoad.back();
	tex->pixelData = stbi_load(tex->path.c_str(), &tex->width, &tex->height, &tex->nrChannels, 0);
	if (!tex->pixelData)
		throw std::runtime_error("failed to load texture at " + path);
	switch (tex->nrChannels)
	{
	case 1:
		tex->format = VK_FORMAT_R8_SRGB;
		break;
	case 2:
		tex->format = VK_FORMAT_R8G8_SRGB;
		break;
	case 3:
		tex->format = VK_FORMAT_R8G8B8_SRGB;
		break;
	case 4:
		tex->format = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	default:
		throw std::runtime_error("texture at " + path + " has an unsupported number of channels");
	}
	return texToLoad.size() - 1;
}


void TextureLoader::endLoading()
{
	textures.resize(texToLoad.size());
	std::vector<VkMemoryRequirements> memReqs(texToLoad.size());
	for (size_t i = 0; i < texToLoad.size(); i++)
	{
		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = texToLoad[i].width;
		imageInfo.extent.height = texToLoad[i].height;
		imageInfo.mipLevels = 1; //todo add mipmap
		imageInfo.arrayLayers = 1;
		imageInfo.format = texToLoad[i].format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; //todo add multsampling
		
		if (vkCreateImage(device, &imageInfo, nullptr, &textures[i].image) != VK_SUCCESS)
			throw std::runtime_error("failed to create image from texture at: " + texToLoad[i].path);

		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = textures[i].image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = texToLoad[i].format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = VK_SAMPLE_COUNT_1_BIT;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, &textures[i].view) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view from texture at: " + texToLoad[i].path);
	}




	for (const auto& tex : texToLoad)
	{
		stbi_image_free(tex.pixelData);
	}
}