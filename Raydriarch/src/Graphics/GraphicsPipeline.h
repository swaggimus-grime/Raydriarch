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
private:
	VkPipeline m_Pipeline;

	ScopedPtr<Device> m_Device;
	ScopedPtr<SwapChain> m_SwapChain;

	VkPipelineLayout m_PipelineLayout;
	VkRenderPass m_RenderPass;

	std::vector<VkFramebuffer> m_Framebuffers;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	VkSemaphore m_ImageAvailableSemaphore;
	VkSemaphore m_RenderFinishedSemaphore;
};