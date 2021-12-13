#include "raydpch.h"
#include "GraphicsPipeline.h"

#include "Command.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//Defines max number of frames that are allowed to be processed by the graphics pipeline 
#define MAX_FRAMES_IN_FLIGHT 2

GraphicsPipeline::GraphicsPipeline(ScopedPtr<Window>& window, RefPtr<Device> device, ScopedPtr<SwapChain>& swapchain, RefPtr<Shader>& shader, VertexLayout& vertexLayout)
	:m_Window(window), m_Device(device), m_SwapChain(swapchain), m_Shader(shader), m_VertexLayout(vertexLayout)
{
	CreateDescriptorSetLayout();
	CreatePipeline();
	CreateUniformBuffers();
	m_Sampler = MakeRefPtr<Sampler2D>(m_Device);
	m_Texture = MakeRefPtr<Texture2D>(m_Device, "res/textures/bruh.jpg");
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapchain->GetImages().size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapchain->GetImages().size());
	m_DescriptorPool = MakeScopedPtr<DescriptorPool>(m_Device, m_SwapChain->GetImages().size(), poolSizes.size(), poolSizes.data());
	CreateDescriptorSets();
	CreateSyncObjects();
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(m_Device->GetDeviceHandle(), m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device->GetDeviceHandle(), m_PipelineLayout, nullptr);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_Device->GetDeviceHandle(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device->GetDeviceHandle(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device->GetDeviceHandle(), m_InFlightFences[i], nullptr);
	}
}

VkResult GraphicsPipeline::Present(float deltaTime)
{
	static uint32_t currentFrame = 0;

	vkWaitForFences(m_Device->GetDeviceHandle(), 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_Device->GetDeviceHandle(), m_SwapChain->GetSwapChainHandle(), UINT64_MAX, m_ImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return VK_ERROR_OUT_OF_DATE_KHR;
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
		RAYD_ERROR("Failed to acquire swap chain image!");

	if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(m_Device->GetDeviceHandle(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	m_ImagesInFlight[imageIndex] = m_InFlightFences[currentFrame];

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), deltaTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain->GetExtent().width / (float)m_SwapChain->GetExtent().height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	m_UniformBuffers[imageIndex]->Update(sizeof(ubo), &ubo);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &Command::GetCommandBuffers()[imageIndex];

	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_Device->GetDeviceHandle(), 1, &m_InFlightFences[currentFrame]);

	RAYD_VK_VALIDATE(vkQueueSubmit(m_Device->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, m_InFlightFences[currentFrame]),
		"Failed to submit command buffer to graphics queue!");

	VkPresentInfoKHR presentInfo {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain->GetSwapChainHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	if (result = vkQueuePresentKHR(m_Device->GetQueueFamilies().Present.Queue, &presentInfo);
		result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		result = VK_ERROR_OUT_OF_DATE_KHR;
	else if (result != VK_SUCCESS)
		RAYD_ERROR("Failed to rebuild swap chain!");
	
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return result;
}

void GraphicsPipeline::Shutdown()
{
	vkDeviceWaitIdle(m_Device->GetDeviceHandle());
}

void GraphicsPipeline::CreateDescriptorSetLayout()
{
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

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	m_DescriptorSetLayout = MakeScopedPtr<DescriptorSetLayout>(m_Device, bindings.size(), bindings.data());
}

void GraphicsPipeline::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_SwapChain->GetImages().size(), m_DescriptorSetLayout->GetLayoutHandle());
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool->GetPoolHandle();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_SwapChain->GetImages().size());
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(m_SwapChain->GetImages().size());
	RAYD_VK_VALIDATE(vkAllocateDescriptorSets(m_Device->GetDeviceHandle(), &allocInfo, m_DescriptorSets.data()), "Failed to allocate descriptor sets!");

	for (size_t i = 0; i < m_SwapChain->GetImages().size(); i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffers[i]->GetBufferHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_Texture->GetViewHandle();
		imageInfo.sampler = m_Sampler->GetHandle();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_DescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Device->GetDeviceHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void GraphicsPipeline::CreatePipeline()
{
	//Configure pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = m_Shader->GetVertexShaderModule();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = m_Shader->GetFragmentShaderModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = m_VertexLayout.GetNumAttributes();
	vertexInputInfo.pVertexBindingDescriptions = m_VertexLayout.GetBindings();
	vertexInputInfo.pVertexAttributeDescriptions = m_VertexLayout.GetAttributes();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	auto& extent = m_SwapChain->GetExtent();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterInfo{};
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.depthClampEnable = false;
	rasterInfo.rasterizerDiscardEnable = false;
	rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterInfo.lineWidth = 1.f;
	rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
	colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentInfo.blendEnable = VK_TRUE;
	colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachmentInfo;
	

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout->GetLayoutHandle();

	RAYD_VK_VALIDATE(vkCreatePipelineLayout(m_Device->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout),
		"Failed to create pipeline layout!");

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	VkPipelineShaderStageCreateInfo shaderStageInfos[] = { vertShaderStageInfo, fragShaderStageInfo };
	pipelineInfo.pStages = shaderStageInfos;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pRasterizationState = &rasterInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_SwapChain->GetRenderPass().GetRenderPassHandle();
	pipelineInfo.subpass = 0;

	RAYD_VK_VALIDATE(vkCreateGraphicsPipelines(m_Device->GetDeviceHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline),
		"Failed to create graphics pipeline!");
}


void GraphicsPipeline::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_ImagesInFlight.resize(m_SwapChain->GetImages().size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		RAYD_VK_VALIDATE(vkCreateSemaphore(m_Device->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]),
			"Failed to create semaphore for signaling image availability!");
		RAYD_VK_VALIDATE(vkCreateSemaphore(m_Device->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]),
			"Failed to create semaphore for signaling rendering finish!");
		RAYD_VK_VALIDATE(vkCreateFence(m_Device->GetDeviceHandle(), &fenceInfo, nullptr, &m_InFlightFences[i]),
			"Failed to create fence!");
	}
}

void GraphicsPipeline::CreateUniformBuffers()
{
	m_UniformBuffers.resize(m_SwapChain->GetImages().size());

	for (auto& buffer : m_UniformBuffers)
		buffer = MakeScopedPtr<UniformBuffer>(m_Device, sizeof(UniformBufferObject));
}
