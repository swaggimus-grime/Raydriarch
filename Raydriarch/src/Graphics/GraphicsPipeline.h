#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "Shader.h"

class GraphicsPipeline {
public:
	GraphicsPipeline(Window* window);
	~GraphicsPipeline();

	void Present();
	void Shutdown();
private:
	void CreatePipeline(Shader* shader);
	void CreateFramebuffers();
	void AllocateCommandBuffers();
	void CreateSyncObjects();
private:
	VkPipeline m_Pipeline;

	ScopedPtr<Device> m_Device;
	ScopedPtr<SwapChain> m_SwapChain;

	VkPipelineLayout m_PipelineLayout;
	VkRenderPass m_RenderPass;

	std::vector<VkFramebuffer> m_Framebuffers;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFence> m_ImagesInFlight;
};