#include "vkhelper.h"

uint32_t vkhelper::findMemoryIndex(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (size_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (memoryTypeBits & (1 << i)
			&& memProperties.memoryTypes[i].propertyFlags & properties)
		{
			return i;
		}
	}
	throw std::runtime_error("failed to find suitable memory type");
}

glm::mat4 vkhelper::getModel(glm::vec2 pos, glm::vec2 size, float rotate)
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(pos, 0.0f)); //translate object by position
	//rotate object
	model = glm::translate(model, glm::vec3(0.5 * size.x, 0.5 * size.y, 0.0)); // move object by half its size, so rotates around centre
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0, 0.0, 1.0));//then do rotation
	model = glm::translate(model, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0)); //then translate back to original position

	model = glm::scale(model, glm::vec3(size, 1.0f)); //then scale

	return model;
}