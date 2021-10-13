#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "Shader.h"
#include "Buffer.h"
#include "Descriptor.h"

class GraphicsPipeline {
public:
	GraphicsPipeline(ScopedPtr<class Window>& window, RefPtr<Device> device, RefPtr<SwapChain> swapchain, ScopedPtr<Shader>& shader, VertexLayout& vertexLayout);
	~GraphicsPipeline();

	void Present(float deltaTime);
	void Shutdown();
	inline const VkPipeline& GetPipelineHandle() const { return m_Pipeline; }
	inline const VkPipelineLayout& GetPipelineLayout() const { return m_PipelineLayout; }
	inline const std::vector<VkDescriptorSet>& GetDescriptorSets() const { return m_DescriptorSets; }
private:
	void CreateDescriptorSetLayout();
	void CreateDescriptorSets();
	void CreatePipeline();
	void CreateSyncObjects();
	void CreateUniformBuffers();
private:
	ScopedPtr<Window>& m_Window;
	RefPtr<Device> m_Device;
	RefPtr<SwapChain> m_SwapChain;
	VertexLayout& m_VertexLayout;

	VkPipeline m_Pipeline;

	ScopedPtr<DescriptorSetLayout> m_DescriptorSetLayout;
	ScopedPtr<DescriptorPool> m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	VkPipelineLayout m_PipelineLayout;

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	std::vector<ScopedPtr<UniformBuffer>> m_UniformBuffers;

	ScopedPtr<Shader>& m_Shader;

	std::vector<VkSemaphore> m_ImageAvailableSemaphores;
	std::vector<VkSemaphore> m_RenderFinishedSemaphores;
	std::vector<VkFence> m_InFlightFences;
	std::vector<VkFence> m_ImagesInFlight;
};