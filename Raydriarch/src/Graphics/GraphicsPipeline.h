#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "Shader.h"
#include "Buffer.h"
#include "Descriptor.h"

class GraphicsPipeline {
public:
	GraphicsPipeline(ScopedPtr<Window>& window, RefPtr<Device> device);
	~GraphicsPipeline();

	void Present();
	void Shutdown();
private:
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();
	void CreatePipeline();
	void CreateCommandPool();
	void AllocateCommandBuffers();
	void CreateSyncObjects();
	void CreateUniformBuffers();
private:
	ScopedPtr<Window>& m_Window;

	VkPipeline m_Pipeline;

	ScopedPtr<DescriptorSetLayout> m_DescriptorSetLayout;
	ScopedPtr<DescriptorPool> m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	RefPtr<Device> m_Device;
	ScopedPtr<SwapChain> m_SwapChain;

	VkPipelineLayout m_PipelineLayout;

	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	std::vector<ScopedPtr<UniformBuffer>> m_UniformBuffers;

	ScopedPtr<Shader> m_Shader;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFence> m_ImagesInFlight;
};