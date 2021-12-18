#include "raydpch.h"
#include "Graphics.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_FRAMES_IN_FLIGHT 2

static SceneData* s_Data = new SceneData;
static GraphicsObjects* s_Objects = new GraphicsObjects;

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4
};
void Graphics::Init(ScopedPtr<Window>& window)
{
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	s_Objects->GPU = MakeRefPtr<Device>(window->GetGraphicsContext().GetInstance(), window->GetSurface(), deviceFeatures);

	auto [width, height] = window->GetFramebufferSize();
	s_Objects->SC = MakeScopedPtr<SwapChain>(s_Objects->GPU, window->GetSurface(), width, height);

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	std::vector<VkDescriptorSetLayoutBinding> descLayoutBindings;
	descLayoutBindings.push_back(uboLayoutBinding);
	descLayoutBindings.push_back(samplerLayoutBinding);
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = descLayoutBindings.size();
	layoutInfo.pBindings = descLayoutBindings.data();

	RAYD_VK_VALIDATE(vkCreateDescriptorSetLayout(s_Objects->GPU->GetDeviceHandle(), &layoutInfo, nullptr, &s_Data->DescSetLayout), "Failed to create descriptor set layout!");

	s_Data->VLayout.AddAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT);
	s_Data->VLayout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT);
	s_Data->VLayout.AddAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT);
	s_Data->VLayout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);
	s_Data->Pipeline = MakeRefPtr<GraphicsPipeline>(s_Objects->GPU, s_Objects->SC, s_Data->DescSetLayout, "res/shaders/vert.spv", "res/shaders/frag.spv", s_Data->VLayout);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = *s_Objects->GPU->GetQueueFamilies().Graphics.Index;

	RAYD_VK_VALIDATE(vkCreateCommandPool(s_Objects->GPU->GetDeviceHandle(), &poolInfo, nullptr, &s_Objects->CommandPool), "Failed to create graphics command pool!");
	Command::Init(s_Objects->GPU, s_Objects->CommandPool);

	s_Data->Texture = MakeRefPtr<Image>(s_Objects->GPU, "res/textures/bruh.jpg");
	s_Data->Sampler = MakeRefPtr<Sampler>(s_Objects->GPU);
	s_Data->VBuffer = MakeRefPtr<VertexBuffer>(s_Objects->GPU, vertices.size(), vertices.size() * sizeof(Vertex), vertices.data());
	s_Data->IBuffer = MakeRefPtr<IndexBuffer>(s_Objects->GPU, indices.size(), indices.size() * sizeof(uint16_t), indices.data());
	s_Data->UBuffers.resize(s_Objects->SC->GetImages().size());
	for (auto& ubuff : s_Data->UBuffers)
		ubuff = MakeScopedPtr<UniformBuffer>(s_Objects->GPU, sizeof(UniformBufferObject));
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(s_Objects->SC->GetImages().size()) });
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(s_Objects->SC->GetImages().size()) });
	s_Data->DescPool = MakeRefPtr<DescriptorPool>(s_Objects->GPU, s_Objects->SC->GetImages().size(), poolSizes);
	std::vector<VkDescriptorSetLayout> layouts(s_Objects->SC->GetImages().size(), s_Data->DescSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = s_Data->DescPool->GetPoolHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(s_Objects->SC->GetImages().size());
	allocInfo.pSetLayouts = layouts.data();

	s_Data->DescSets.resize(s_Objects->SC->GetImages().size());
	RAYD_VK_VALIDATE(vkAllocateDescriptorSets(s_Objects->GPU->GetDeviceHandle(), &allocInfo, s_Data->DescSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < s_Objects->SC->GetImages().size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = s_Data->UBuffers[i]->GetBufferHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = s_Data->Texture->GetViewHandle();
		imageInfo.sampler = s_Data->Sampler->GetHandle();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = s_Data->DescSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = s_Data->DescSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(s_Objects->GPU->GetDeviceHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	s_Objects->CBuffers.resize(s_Objects->SC->GetFramebuffers().size());

	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = s_Objects->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = s_Objects->CBuffers.size();

		RAYD_VK_VALIDATE(vkAllocateCommandBuffers(s_Objects->GPU->GetDeviceHandle(), &allocInfo, s_Objects->CBuffers.data()), "Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < s_Objects->CBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		RAYD_VK_VALIDATE(vkBeginCommandBuffer(s_Objects->CBuffers[i], &beginInfo), "Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = s_Objects->SC->GetRenderPass();
		renderPassInfo.framebuffer = s_Objects->SC->GetFramebuffers()[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = s_Objects->SC->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(s_Objects->CBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(s_Objects->CBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data->Pipeline->GetPipelineHandle());

		VkBuffer vertexBuffers[] = { s_Data->VBuffer->GetBufferHandle() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(s_Objects->CBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(s_Objects->CBuffers[i], s_Data->IBuffer->GetBufferHandle(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(s_Objects->CBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data->Pipeline->GetLayoutHandle(), 0, 1, &s_Data->DescSets[i], 0, nullptr);

		vkCmdDrawIndexed(s_Objects->CBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(s_Objects->CBuffers[i]);

		RAYD_VK_VALIDATE(vkEndCommandBuffer(s_Objects->CBuffers[i]), "Failed to record command buffer!");
	}

	s_Objects->ImageAvailSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	s_Objects->RenderFinishSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	s_Objects->InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	s_Objects->ImagesInFlightFenches.resize(s_Objects->SC->GetImages().size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		RAYD_VK_VALIDATE(vkCreateSemaphore(s_Objects->GPU->GetDeviceHandle(), &semaphoreInfo, nullptr, &s_Objects->ImageAvailSemaphores[i]) ||
			vkCreateSemaphore(s_Objects->GPU->GetDeviceHandle(), &semaphoreInfo, nullptr, &s_Objects->RenderFinishSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(s_Objects->GPU->GetDeviceHandle(), &fenceInfo, nullptr, &s_Objects->InFlightFences[i]), "Failed to create synchronization objects for a frame!");
	}
}

void Graphics::Present(ScopedPtr<class Window>& window, float deltaTime)
{
	static uint8_t currentFrame = 0;
	vkWaitForFences(s_Objects->GPU->GetDeviceHandle(), 1, &s_Objects->InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(s_Objects->GPU->GetDeviceHandle(), s_Objects->SC->GetSwapChainHandle(), UINT64_MAX, s_Objects->ImageAvailSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		Graphics::RecreateSwapChain(window);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		RAYD_ERROR("failed to acquire swap chain image!");

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	auto [width, height] = s_Objects->SC->GetExtent();
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	s_Data->UBuffers[imageIndex]->Update(sizeof(ubo), &ubo);

	if (s_Objects->ImagesInFlightFenches[imageIndex] != VK_NULL_HANDLE) 
		vkWaitForFences(s_Objects->GPU->GetDeviceHandle(), 1, &s_Objects->ImagesInFlightFenches[imageIndex], VK_TRUE, UINT64_MAX);
	s_Objects->ImagesInFlightFenches[imageIndex] = s_Objects->InFlightFences[currentFrame];

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { s_Objects->ImageAvailSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &s_Objects->CBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { s_Objects->RenderFinishSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(s_Objects->GPU->GetDeviceHandle(), 1, &s_Objects->InFlightFences[currentFrame]);

	RAYD_VK_VALIDATE(vkQueueSubmit(s_Objects->GPU->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, s_Objects->InFlightFences[currentFrame]), "Failed to submit draw command buffer!");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { s_Objects->SC->GetSwapChainHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(s_Objects->GPU->GetQueueFamilies().Present.Queue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->m_Resized) {
		window->m_Resized = false;
		Graphics::RecreateSwapChain(window);
	}
	else
		RAYD_VK_VALIDATE(result, "Failed to present swap chain image!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphics::RecreateSwapChain(ScopedPtr<class Window>& window)
{
	auto [width, height] = window->GetFramebufferSize();
	while (width == 0 || height == 0) {
		auto [width, height] = window->GetFramebufferSize();
		glfwWaitEvents();
	}

	CleanupSwapChain();
	s_Objects->SC.reset();
	s_Objects->SC = MakeScopedPtr<SwapChain>(s_Objects->GPU, window->GetSurface(), width, height);

	s_Data->Pipeline.reset();
	s_Data->Pipeline = MakeRefPtr<GraphicsPipeline>(s_Objects->GPU, s_Objects->SC, s_Data->DescSetLayout, "res/shaders/vert.spv", "res/shaders/frag.spv", s_Data->VLayout);
	for (auto& ubuff : s_Data->UBuffers) {
		ubuff.reset();
		ubuff = MakeScopedPtr<UniformBuffer>(s_Objects->GPU, sizeof(UniformBufferObject));
	}

	std::vector<VkDescriptorPoolSize> poolSizes{};
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(s_Objects->SC->GetImages().size()) });
	poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(s_Objects->SC->GetImages().size()) });
	s_Data->DescPool = MakeRefPtr<DescriptorPool>(s_Objects->GPU, s_Objects->SC->GetImages().size(), poolSizes);
	std::vector<VkDescriptorSetLayout> layouts(s_Objects->SC->GetImages().size(), s_Data->DescSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = s_Data->DescPool->GetPoolHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(s_Objects->SC->GetImages().size());
	allocInfo.pSetLayouts = layouts.data();

	RAYD_VK_VALIDATE(vkAllocateDescriptorSets(s_Objects->GPU->GetDeviceHandle(), &allocInfo, s_Data->DescSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < s_Objects->SC->GetImages().size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = s_Data->UBuffers[i]->GetBufferHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = s_Data->Texture->GetViewHandle();
		imageInfo.sampler = s_Data->Sampler->GetHandle();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = s_Data->DescSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = s_Data->DescSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(s_Objects->GPU->GetDeviceHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	s_Objects->CBuffers.resize(s_Objects->SC->GetFramebuffers().size());

	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = s_Objects->CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = s_Objects->CBuffers.size();

		RAYD_VK_VALIDATE(vkAllocateCommandBuffers(s_Objects->GPU->GetDeviceHandle(), &allocInfo, s_Objects->CBuffers.data()), "Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < s_Objects->CBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		RAYD_VK_VALIDATE(vkBeginCommandBuffer(s_Objects->CBuffers[i], &beginInfo), "Failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = s_Objects->SC->GetRenderPass();
		renderPassInfo.framebuffer = s_Objects->SC->GetFramebuffers()[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = s_Objects->SC->GetExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(s_Objects->CBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(s_Objects->CBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data->Pipeline->GetPipelineHandle());

		VkBuffer vertexBuffers[] = { s_Data->VBuffer->GetBufferHandle() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(s_Objects->CBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(s_Objects->CBuffers[i], s_Data->IBuffer->GetBufferHandle(), 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(s_Objects->CBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, s_Data->Pipeline->GetLayoutHandle(), 0, 1, &s_Data->DescSets[i], 0, nullptr);

		vkCmdDrawIndexed(s_Objects->CBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(s_Objects->CBuffers[i]);

		RAYD_VK_VALIDATE(vkEndCommandBuffer(s_Objects->CBuffers[i]), "Failed to record command buffer!");
	}

	s_Objects->ImagesInFlightFenches.resize(s_Objects->SC->GetImages().size(), VK_NULL_HANDLE);
}

void Graphics::CleanupSwapChain()
{
	s_Objects->GPU->Join();
	s_Objects->SC.reset();
	vkFreeCommandBuffers(s_Objects->GPU->GetDeviceHandle(), s_Objects->CommandPool, static_cast<uint32_t>(s_Objects->CBuffers.size()), s_Objects->CBuffers.data());
	s_Data->Pipeline.reset();

	for (auto& buff : s_Data->UBuffers)
		buff.reset();

	s_Data->DescPool.reset();
}

void Graphics::Shutdown()
{
	CleanupSwapChain();

	vkDestroyDescriptorSetLayout(s_Objects->GPU->GetDeviceHandle(), s_Data->DescSetLayout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(s_Objects->GPU->GetDeviceHandle(), s_Objects->RenderFinishSemaphores[i], nullptr);
		vkDestroySemaphore(s_Objects->GPU->GetDeviceHandle(), s_Objects->ImageAvailSemaphores[i], nullptr);
		vkDestroyFence(s_Objects->GPU->GetDeviceHandle(), s_Objects->InFlightFences[i], nullptr);
	}

	Command::Shutdown();
	vkDestroyCommandPool(s_Objects->GPU->GetDeviceHandle(), s_Objects->CommandPool, nullptr);

	delete s_Data;
	delete s_Objects;
}
