#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "Shader.h"
#include "Buffer.h"

class GraphicsPipeline {
public:
	GraphicsPipeline(ScopedPtr<Window>& window, RefPtr<Device> device);
	~GraphicsPipeline();

	void Present();
	void Shutdown();
private:
	void CreatePipeline();
	void CreateCommandPool();
	void AllocateCommandBuffers();
	void CreateSyncObjects();
private:
	ScopedPtr<Window>& m_Window;

	VkPipeline m_Pipeline;

	RefPtr<Device> m_Device;
	ScopedPtr<SwapChain> m_SwapChain;

	VkPipelineLayout m_PipelineLayout;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;

	ScopedPtr<Shader> m_Shader;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFence> m_ImagesInFlight;
};