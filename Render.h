#pragma once
#ifndef RENDER_H
#define RENDER_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>

#include "vkinit.h"
#include "vkhelper.h"
#include "typeStructs.h"
#include "TextureLoader.h"

class Render
{
public:
	Render(GLFWwindow* window);
	~Render();
	void startDraw();
	void endDraw();
	void DrawSquare(glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 colour);

	bool framebufferResized = false;
	TextureLoader textureLoader;
private:
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkSurfaceKHR mSurface;
	Base mBase;
	FrameData mFrame;
	SwapChain mSwapchain;
	VkRenderPass mRenderPass;
	Pipeline mPipeline;
	memoryObjects mMemory;
	VkCommandPool generalCommandPool;
	VkCommandBuffer transferCommandBuffer;
	DescriptorSets mFrameDescriptorSets;

	bool begunDraw = false;
	uint32_t img;
	VkSemaphore imgAquireSem;
	viewProjectionBufferObj ubo;

	std::vector<Vertex> quadVerts = {
		{{0.0f, 0.0f}, {0.0f, 0.0f}},
		{{1.0f, 0.0f}, {1.0f, 0.0f}},
		{{1.0f, 1.0f}, {1.0f, 1.0f}},
		{{0.0f, 1.0f}, {0.0f, 1.0f}},
	};
	std::vector<uint32_t> quadInds = { 0, 1, 2, 2, 3, 0 };

	void loadDataToGpu();
	void copyDataToLocalGPUMemory();
	void prepareDescriptorSets();
	void destroySwapchainComponents();
	void resize();

#ifndef NDEBUG
	VkDebugUtilsMessengerEXT mDebugMessenger;
#endif

};





#endif 