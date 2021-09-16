#include "TextureLoader.h"

TextureLoader::TextureLoader(Base base)
{
	this->base = base;
}

uint32_t TextureLoader::loadTexture(std::string path)
{
	texToLoad.push_back({ path });
	TempTexture* tex = &texToLoad.back();
	tex->pixelData = stbi_load(tex->path.c_str(), &tex->width, &tex->height, &tex->nrChannels, 0);
	if (!tex->pixelData)
		throw std::runtime_error("failed to load texture at " + path);

	tex->fileSize = tex->width * tex->height * tex->nrChannels;

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

	VkDeviceSize totalFilesize = 0;
	for (const auto& tex : texToLoad)
		totalFilesize += tex.fileSize;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	vkhelper::createBufferAndMemory(base, totalFilesize, &stagingBuffer, &stagingMemory,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkBindBufferMemory(base.device, buffer, memory, 0);
	void* pMem;
	vkMapMemory(base.device, memory, 0, totalFilesize, 0, &pMem);

	//move image pixel data to buffer - TODO

	//make image and view
	for (size_t i = 0; i < texToLoad.size(); i++)
	{
		textures[i] = Texture(texToLoad[i]);

		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = textures[i].width;
		imageInfo.extent.height = textures[i].height;
		imageInfo.mipLevels = textures[i].mipLevels; //todo add mipmap
		imageInfo.arrayLayers = 1;
		imageInfo.format = texToLoad[i].format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; //todo add multsampling
		
		if (vkCreateImage(base.device, &imageInfo, nullptr, &textures[i].image) != VK_SUCCESS)
			throw std::runtime_error("failed to create image from texture at: " + texToLoad[i].path);

		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = textures[i].image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = texToLoad[i].format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = textures[i].mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(base.device, &viewInfo, nullptr, &textures[i].view) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image view from texture at: " + texToLoad[i].path);

	}




	for (const auto& tex : texToLoad)
	{
		stbi_image_free(tex.pixelData);
	}
}