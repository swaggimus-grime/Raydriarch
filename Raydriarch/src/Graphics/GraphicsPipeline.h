#pragma once

#include "Core/Window.h"
#include "Device.h"
#include "Shader.h"
#include "Descriptor.h"

class GraphicsPipeline {
public:
	GraphicsPipeline(RefPtr<Device> device, ScopedPtr<SwapChain>& swapChain, VkDescriptorSetLayout& descSetLayout, 
		const std::string& vertShaderPath, const std::string& fragShaderPath, VertexLayout& vlayout);
	~GraphicsPipeline();

	inline VkPipeline& GetPipelineHandle() { return m_Pipeline; }
	inline VkPipelineLayout& GetLayoutHandle() { return m_Layout; }
private:
	RefPtr<Device> m_Device;

	VkPipeline m_Pipeline;
	VkPipelineLayout m_Layout;
};