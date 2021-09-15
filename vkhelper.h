#pragma once
#ifndef VKHELPER_H
#define VKHELPER_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

struct vkhelper
{
	static uint32_t findMemoryIndex(VkPhysicalDevice physicalDevice, uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
	static glm::mat4 getModel(glm::vec2 pos, glm::vec2 size, float rotate);
};




#endif