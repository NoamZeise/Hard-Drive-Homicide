#include "Render.h"

Render::Render(GLFWwindow* window)
{
	mWindow = window;
	initVulkan::instance(&mInstance);
#ifndef NDEBUG
	initVulkan::debugMessenger(mInstance, &mDebugMessenger);
#endif
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");
	initVulkan::device(mInstance, mBase.physicalDevice, &mBase.device, mSurface, &mBase.queue);
	initVulkan::swapChain(mBase.device, mBase.physicalDevice, mSurface, &mSwapchain, mWindow, mBase.queue.graphicsPresentFamilyIndex);
	initVulkan::renderPass(mBase.device, &mRenderPass, mSwapchain);
	initVulkan::framebuffers(mBase.device, &mSwapchain, mRenderPass);


	initVulkan::CreateDescriptorSets(mBase.device, &viewprojDS, 0, mSwapchain.frameData.size(), 
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<VkDescriptorSetLayoutBinding> bindings(2);
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].binding = 1;
	bindings[1].descriptorCount = MAX_TEXTURES_SUPPORTED;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();
	if (vkCreateDescriptorSetLayout(mBase.device, &layoutInfo, nullptr, &texturesDS.layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

	std::vector<VkDescriptorSetLayout> dsLs = {viewprojDS.layout, texturesDS.layout };

	initVulkan::graphicsPipeline(mBase.device, &mPipeline, mSwapchain, mRenderPass, dsLs);
	prepareViewProjDS();


	//create general command pool
	VkCommandPoolCreateInfo commandPoolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolInfo.queueFamilyIndex = mBase.queue.graphicsPresentFamilyIndex;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(mBase.device, &commandPoolInfo, nullptr, &generalCommandPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool");
	//create transfer command buffer
	VkCommandBufferAllocateInfo commandBufferInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferInfo.commandPool = generalCommandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(mBase.device, &commandBufferInfo, &transferCommandBuffer))
		throw std::runtime_error("failed to allocate command buffer");
	
	loadDataToGpu();
	copyDataToLocalGPUMemory();


	ubo.view = glm::mat4(1.0f);
	ubo.proj = glm::ortho(0.0f, (float)mSwapchain.extent.width, 0.0f, (float)mSwapchain.extent.height, -1.0f, 1.0f);

	textureLoader = TextureLoader(mBase, generalCommandPool);
	textureLoader.loadTexture("error.png");
}

Render::~Render()
{
	vkQueueWaitIdle(mBase.queue.graphicsPresentQueue);
	textureLoader.~TextureLoader();
	destroySwapchainComponents();
	vkDestroyBuffer(mBase.device, mMemory.vertexBuffer, nullptr);
	vkDestroyBuffer(mBase.device, mMemory.indexBuffer, nullptr);
	vkFreeMemory(mBase.device, mMemory.memory, nullptr);
	vkDestroyBuffer(mBase.device, mMemory.stagingBuffer, nullptr);
	vkFreeMemory(mBase.device, mMemory.stagingMemory, nullptr);
	vkDestroyCommandPool(mBase.device, generalCommandPool, nullptr);
	initVulkan::destroySwapchain(&mSwapchain, mBase.device);
	vkDestroyDevice(mBase.device, nullptr);
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
#ifndef NDEBUG
	initVulkan::DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif
	vkDestroyInstance(mInstance, nullptr);
}

void Render::loadDataToGpu()
{

	size_t vertexSize = sizeof(quadVerts[0]) * quadVerts.size();
	size_t indexSize = sizeof(quadInds[0]) * quadInds.size();

	VkBufferCreateInfo stagingBufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	stagingBufferInfo.size = vertexSize + indexSize;
	stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.queueFamilyIndexCount = 1;
	stagingBufferInfo.pQueueFamilyIndices = &mBase.queue.graphicsPresentFamilyIndex;
	stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(mBase.device, &stagingBufferInfo, nullptr, &mMemory.stagingBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer!");
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mBase.device, mMemory.stagingBuffer, &memRequirements);

	uint32_t memIndex = vkhelper::findMemoryIndex(mBase.physicalDevice, memRequirements.memoryTypeBits,
		(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
	
	VkMemoryAllocateInfo memInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memInfo.allocationSize = memRequirements.size;
	memInfo.memoryTypeIndex = memIndex;
	if (vkAllocateMemory(mBase.device, &memInfo, nullptr, &mMemory.stagingMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");

	vkBindBufferMemory(mBase.device, mMemory.stagingBuffer, mMemory.stagingMemory, 0);
	
	void* data;
	vkMapMemory(mBase.device, mMemory.stagingMemory, 0, memRequirements.size, 0, &data);
	memcpy(data, quadVerts.data(), vertexSize);
	memcpy(static_cast<char*>(data) + vertexSize, quadInds.data(), indexSize);
}

void Render::copyDataToLocalGPUMemory()
{
	size_t vertexSize = sizeof(quadVerts[0]) * quadVerts.size();
	size_t indexSize = sizeof(quadInds[0]) * quadInds.size();

	VkBufferCreateInfo vbufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	vbufferInfo.size = vertexSize;
	vbufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vbufferInfo.queueFamilyIndexCount = 1;
	vbufferInfo.pQueueFamilyIndices = &mBase.queue.graphicsPresentFamilyIndex;
	vbufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(mBase.device, &vbufferInfo, nullptr, &mMemory.vertexBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer!");
	VkMemoryRequirements vmemRequirements;
	vkGetBufferMemoryRequirements(mBase.device, mMemory.vertexBuffer, &vmemRequirements);

	VkBufferCreateInfo ibufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	ibufferInfo.size = indexSize;
	ibufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	ibufferInfo.queueFamilyIndexCount = 1;
	ibufferInfo.pQueueFamilyIndices = &mBase.queue.graphicsPresentFamilyIndex;
	ibufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(mBase.device, &ibufferInfo, nullptr, &mMemory.indexBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer!");
	VkMemoryRequirements imemRequirements;
	vkGetBufferMemoryRequirements(mBase.device, mMemory.indexBuffer, &imemRequirements);

	uint32_t memIndex = vkhelper::findMemoryIndex(
		mBase.physicalDevice,imemRequirements.memoryTypeBits & vmemRequirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo memInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	memInfo.allocationSize = vmemRequirements.size + imemRequirements.size;
	memInfo.memoryTypeIndex = memIndex;
	if (vkAllocateMemory(mBase.device, &memInfo, nullptr, &mMemory.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");

	vkBindBufferMemory(mBase.device, mMemory.vertexBuffer, mMemory.memory, 0);
	//begin command recording
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = vertexSize;
	vkCmdCopyBuffer(transferCommandBuffer, mMemory.stagingBuffer, mMemory.vertexBuffer,1, &copyRegion);

	vkEndCommandBuffer(transferCommandBuffer);
	//submit commands for execution
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;
	vkQueueSubmit(mBase.queue.graphicsPresentQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mBase.queue.graphicsPresentQueue);

	vkResetCommandPool(mBase.device, generalCommandPool, 0);

	vkBindBufferMemory(mBase.device, mMemory.indexBuffer, mMemory.memory, vmemRequirements.size);
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	copyRegion.srcOffset = vertexSize;
	copyRegion.size = indexSize;
	vkCmdCopyBuffer(transferCommandBuffer, mMemory.stagingBuffer, mMemory.indexBuffer, 1, &copyRegion);

	vkEndCommandBuffer(transferCommandBuffer);
	//submit commands for execution
	submitInfo.pCommandBuffers = &transferCommandBuffer;
	vkQueueSubmit(mBase.queue.graphicsPresentQueue, 1, &submitInfo, VK_NULL_HANDLE);

	vkQueueWaitIdle(mBase.queue.graphicsPresentQueue);
	vkResetCommandBuffer(transferCommandBuffer, 0);
	
}

uint32_t Render::loadTexture(std::string filepath)
{
	return textureLoader.loadTexture(filepath);
}

Font* Render::loadFont(std::string filepath)
{
	try
	{
		return new Font(filepath, &textureLoader);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return nullptr;
	}
}

void Render::endTextureLoad()
{
	textureLoader.endLoading();
	prepareFragmentDescriptorSets();

	fragPushConstants fps{
		glm::vec4(1, 1, 1, 1),
		glm::vec4(0, 0, 1, 1),
		0
	};
	vkCmdPushConstants(mSwapchain.frameData[img].commandBuffer, mPipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(vectPushConstants), sizeof(fragPushConstants), &fps);
}

void Render::startDraw()
{
	begunDraw = true;
	if (mSwapchain.imageAquireSem.empty())
	{
		VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		if (vkCreateSemaphore(mBase.device, &semaphoreInfo, nullptr, &imgAquireSem) != VK_SUCCESS)
			throw std::runtime_error("failed to create image available semaphore");
	}
	else
	{
		imgAquireSem = mSwapchain.imageAquireSem.back();
		mSwapchain.imageAquireSem.pop_back();
	}
	if (vkAcquireNextImageKHR(mBase.device, mSwapchain.swapChain, UINT64_MAX,
		imgAquireSem, VK_NULL_HANDLE, &img) != VK_SUCCESS)
	{
		mSwapchain.imageAquireSem.push_back(imgAquireSem);
		return;
	}

	if (mSwapchain.frameData[img].frameFinishedFen != VK_NULL_HANDLE)
	{
		vkWaitForFences(mBase.device, 1, &mSwapchain.frameData[img].frameFinishedFen, VK_TRUE, UINT64_MAX);
		vkResetFences(mBase.device, 1, &mSwapchain.frameData[img].frameFinishedFen);
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 
	beginInfo.pInheritanceInfo = nullptr; //optional - for secondary command buffers, which state to inherit from primary

	if (vkBeginCommandBuffer(mSwapchain.frameData[img].commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to being recording command buffer");
	}

	ubo.view = glm::mat4(1.0f);
	ubo.proj = glm::ortho(0.0f, (float)mSwapchain.extent.width, 0.0f, (float)mSwapchain.extent.height, -1.0f, 1.0f);

	memcpy(static_cast<char*>(mMemory.viewProj.pointer) + (img * mMemory.viewProj.slotSize), &ubo, mMemory.viewProj.slotSize);

	//fill render pass begin struct
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.framebuffer = mSwapchain.frameData[img].framebuffer; //framebuffer for each swapchain image
												   //should match size of attachments
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = mSwapchain.extent;
	//clear colour -> values for VK_ATTACHMENT_LOAD_OP_CLEAR load operation in colour attachment
	VkClearValue clearColour = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColour;

	//begin render pass
	vkCmdBeginRenderPass(mSwapchain.frameData[img].commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	//bind descriptor sets
	vkCmdBindDescriptorSets(mSwapchain.frameData[img].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		mPipeline.layout, 0, 1, &viewprojDS.sets[img], 0, nullptr);

	vkCmdBindDescriptorSets(mSwapchain.frameData[img].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		mPipeline.layout, 1, 1, &texturesDS.sets[img], 0, 0);

	//bind graphics pipeline
	vkCmdBindPipeline(mSwapchain.frameData[img].commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.pipeline);
	//bind vertex buffer
	VkBuffer vertexBuffers[] = { mMemory.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(mSwapchain.frameData[img].commandBuffer, 0, 1, vertexBuffers, offsets);
	//bind index buffer - can only have one index buffer
	vkCmdBindIndexBuffer(mSwapchain.frameData[img].commandBuffer, mMemory.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Render::endDraw()
{
	if (!begunDraw)
		throw std::runtime_error("start draw before ending it");
	begunDraw = false;
	//end render pass
	vkCmdEndRenderPass(mSwapchain.frameData[img].commandBuffer);
	if (vkEndCommandBuffer(mSwapchain.frameData[img].commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	std::array<VkSemaphore, 1> submitWaitSemaphores = { imgAquireSem };
	std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	std::array<VkSemaphore, 1> submitSignalSemaphores = { mSwapchain.frameData[img].presentReadySem };

	//submit draw command
	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.waitSemaphoreCount = submitWaitSemaphores.size();
	submitInfo.pWaitSemaphores = submitWaitSemaphores.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mSwapchain.frameData[img].commandBuffer;
	submitInfo.signalSemaphoreCount = submitSignalSemaphores.size();
	submitInfo.pSignalSemaphores = submitSignalSemaphores.data();
	if (vkQueueSubmit(mBase.queue.graphicsPresentQueue, 1, &submitInfo, mSwapchain.frameData[img].frameFinishedFen) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer");

	//submit present command
	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = submitSignalSemaphores.size();
	presentInfo.pWaitSemaphores = submitSignalSemaphores.data();
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapchain.swapChain;
	presentInfo.pImageIndices = &img;
	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(mBase.queue.graphicsPresentQueue, &presentInfo);

	if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized)
	{
		framebufferResized = false;
		resize();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("failed to present swapchain image to queue");

	mSwapchain.imageAquireSem.push_back(imgAquireSem);
}

void Render::resize()
{
	vkDeviceWaitIdle(mBase.device);
	destroySwapchainComponents();
	//recreate swapchain and framebuffer
	initVulkan::swapChain(mBase.device, mBase.physicalDevice, mSurface, &mSwapchain, mWindow, mBase.queue.graphicsPresentFamilyIndex);
	initVulkan::renderPass(mBase.device, &mRenderPass, mSwapchain);

	initVulkan::CreateDescriptorSets(mBase.device, &viewprojDS, 0, mSwapchain.frameData.size(),
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

	std::vector<VkDescriptorSetLayoutBinding> bindings(2);
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].binding = 1;
	bindings[1].descriptorCount = MAX_TEXTURES_SUPPORTED;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();
	if (vkCreateDescriptorSetLayout(mBase.device, &layoutInfo, nullptr, &texturesDS.layout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout");

	std::vector<VkDescriptorSetLayout> dsLs = { viewprojDS.layout, texturesDS.layout };
	initVulkan::graphicsPipeline(mBase.device, &mPipeline, mSwapchain, mRenderPass, dsLs);
	initVulkan::framebuffers(mBase.device, &mSwapchain, mRenderPass);
	prepareViewProjDS();
	prepareFragmentDescriptorSets();
	vkDeviceWaitIdle(mBase.device);
}

void Render::DrawSquare(glm::vec4 drawRect, float rotate, glm::vec4 colour, uint32_t texID)
{
	DrawSquare(drawRect, rotate, colour, glm::vec4(0, 0, drawRect.z, drawRect.w), texID);
}

void Render::DrawSquare(glm::vec4 drawRect, float rotate, uint32_t texID)
{
	DrawSquare(drawRect, rotate, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), texID);
}

void Render::DrawSquare(glm::vec4 drawRect, float rotate, glm::vec4 colour, glm::vec4 textureRect, uint32_t texID)
{
	//push constants
	vectPushConstants vps{};
	vps.model = glm::mat4(1.0f);
	vps.model = vkhelper::getModel(drawRect, rotate);
	vkCmdPushConstants(mSwapchain.frameData[img].commandBuffer, mPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(vectPushConstants), &vps);

	fragPushConstants fps{
		colour,
		glm::vec4(0, 0, 1, 1),
		texID
	};
	fps.texOffset = vkhelper::getTextureOffset(drawRect, textureRect);
	//vkCmdPushConstants(mSwapchain.frameData[img].commandBuffer, mPipeline.layout, VK_SHADER_STAGE_FRAGMENT_BIT,
	//	sizeof(vectPushConstants), sizeof(fragPushConstants), &fps);

	//draw verticies	
	vkCmdDrawIndexed(mSwapchain.frameData[img].commandBuffer, static_cast<uint32_t>(quadInds.size()), 1, 0, 0, 0);
}


void Render::DrawString(Font* font, std::string text, glm::vec2 position, float size, float rotate, glm::vec4 colour)
{
	if (font == nullptr)
	{
		std::cout << "font is NULL" << std::endl;
		return;
	}
	for (std::string::const_iterator c = text.begin(); c != text.end(); c++)
	{
		Character* cTex = font->getChar(*c);
		if (cTex == nullptr)
			continue;
		else if (cTex->TextureID != 0) //if character is added but no texture loaded for it (eg space)
		{
			glm::vec4 thisPos = glm::vec4(position.x, position.y, 0, 0);
			thisPos.x += cTex->Bearing.x * size;
			thisPos.y += (cTex->Size.y - cTex->Bearing.y) * size;
			thisPos.y -= cTex->Size.y * size;

			thisPos.z = cTex->Size.x * size;
			thisPos.w = cTex->Size.y * size;
			thisPos.z /= 1;
			thisPos.w /= 1;

			DrawSquare(thisPos, 0, colour, cTex->TextureID);
		}
		position.x += cTex->Advance * size;
	}
}

float Render::measureString(Font* font, std::string text, float size)
{
	float sz = 0;
	for (std::string::const_iterator c = text.begin(); c != text.end(); c++)
	{
		Character* cTex = font->getChar(*c);
		if (cTex == nullptr)
			continue;
		sz += cTex->Advance * size;
	}
	return sz;
}

void Render::destroySwapchainComponents()
{
	vkDestroyBuffer(mBase.device, mMemory.viewProj.buffer, nullptr);
	vkFreeMemory(mBase.device, mMemory.viewProj.memory, nullptr);
	viewprojDS.destroySet(mBase.device);
	texturesDS.destroySet(mBase.device);
	for (size_t i = 0; i < mSwapchain.frameData.size(); i++)
		vkDestroyFramebuffer(mBase.device, mSwapchain.frameData[i].framebuffer, nullptr);
	vkDestroyPipeline(mBase.device, mPipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(mBase.device, mPipeline.layout, nullptr);
	vkDestroyRenderPass(mBase.device, mRenderPass, nullptr);
}

void Render::prepareViewProjDS()
{
	VkDeviceSize slot = sizeof(viewProjectionBufferObj);

	VkPhysicalDeviceProperties physDevProps;
	vkGetPhysicalDeviceProperties(mBase.physicalDevice, &physDevProps);
	if (slot % physDevProps.limits.minUniformBufferOffsetAlignment != 0)
		slot = slot + physDevProps.limits.minUniformBufferOffsetAlignment 
		- (slot % physDevProps.limits.minUniformBufferOffsetAlignment);

	mMemory.viewProj.memSize = slot * viewprojDS.sets.size();

	VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.size = mMemory.viewProj.memSize;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.pQueueFamilyIndices = &mBase.queue.graphicsPresentFamilyIndex;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(mBase.device, &bufferInfo, nullptr, &mMemory.viewProj.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create uniform buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mBase.device, mMemory.viewProj.buffer, &memRequirements);


	uint32_t memIndex = vkhelper::findMemoryIndex(mBase.physicalDevice, memRequirements.memoryTypeBits,
		(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	VkMemoryAllocateInfo memInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memInfo.allocationSize = memRequirements.size;
	memInfo.memoryTypeIndex = memIndex;
	if (vkAllocateMemory(mBase.device, &memInfo, nullptr, &mMemory.viewProj.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");

	vkBindBufferMemory(mBase.device, mMemory.viewProj.buffer, mMemory.viewProj.memory, 0);

	vkMapMemory(mBase.device, mMemory.viewProj.memory, 0, mMemory.viewProj.memSize, 0, &mMemory.viewProj.pointer);

	for (size_t i = 0; i < viewprojDS.sets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mMemory.viewProj.buffer;
		bufferInfo.offset = slot * i;
		bufferInfo.range = slot;

		VkWriteDescriptorSet writeSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeSet.dstSet = viewprojDS.sets[i];
		writeSet.pBufferInfo = &bufferInfo;
		writeSet.dstBinding = 0;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeSet.descriptorCount = 1;

		vkUpdateDescriptorSets(mBase.device, 1, &writeSet, 0, nullptr);
	}
	mMemory.viewProj.slotSize = slot;
}


void Render::prepareFragmentDescriptorSets()
{
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{VK_DESCRIPTOR_TYPE_SAMPLER, 1},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_TEXTURES_SUPPORTED}
	};
	texturesDS.sets.resize(mSwapchain.frameData.size());

	VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.maxSets = texturesDS.sets.size();
	poolInfo.poolSizeCount = sizes.size();
	poolInfo.pPoolSizes = sizes.data();
	if (vkCreateDescriptorPool(mBase.device, &poolInfo, nullptr, &texturesDS.pool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool");

	for (int i = 0; i < texturesDS.sets.size(); i++)
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = texturesDS.pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &texturesDS.layout;

		vkAllocateDescriptorSets(mBase.device, &allocInfo, &texturesDS.sets[i]);
	}


	std::vector<VkDescriptorImageInfo> texInfos(MAX_TEXTURES_SUPPORTED);
	for (uint32_t i = 0; i < MAX_TEXTURES_SUPPORTED; i++)
	{
		texInfos[i].sampler = nullptr;
		texInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		texInfos[i].imageView = textureLoader.getImageView(i);
	}

	VkDescriptorImageInfo imgSamplerInfo = {};
	imgSamplerInfo.sampler = textureLoader.sampler;

	//sampler
	std::vector<VkWriteDescriptorSet> sampDSWrite(texturesDS.sets.size() * 2, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
	int index = 0;
	for (size_t i = 0; i < texturesDS.sets.size() * 2; i+=2)
	{
		sampDSWrite[i].dstSet = texturesDS.sets[index];
		sampDSWrite[i].pBufferInfo = 0;
		sampDSWrite[i].dstBinding = 0;
		sampDSWrite[i].dstArrayElement = 0;
		sampDSWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		sampDSWrite[i].descriptorCount = 1;
		sampDSWrite[i].pImageInfo = &imgSamplerInfo; //todo sampler

		sampDSWrite[i + 1].dstSet = texturesDS.sets[index];
		sampDSWrite[i + 1].pBufferInfo = 0;
		sampDSWrite[i + 1].dstBinding = 1;
		sampDSWrite[i + 1].dstArrayElement = 0;
		sampDSWrite[i + 1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		sampDSWrite[i + 1].descriptorCount = MAX_TEXTURES_SUPPORTED;
		sampDSWrite[i + 1].pImageInfo = texInfos.data();
		index++;
	}
	vkUpdateDescriptorSets(mBase.device, texturesDS.sets.size() * 2, sampDSWrite.data(), 0, nullptr);
}

