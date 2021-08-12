#include "raydpch.h"
#include "GraphicsPipeline.h"

#include <glm/glm.hpp>

struct Vertex {
	glm::vec2 Position;
	glm::vec3 Color;
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

//Defines max number of frames that are allowed to be processed by the graphics pipeline 
#define MAX_FRAMES_IN_FLIGHT 2

GraphicsPipeline::GraphicsPipeline(Window* window, Device* device)
	:m_Device(device)
{
	auto [width, height] = window->GetFramebufferSize();
	m_SwapChain = MakeScopedPtr<SwapChain>(device, window->GetSurface().GetSurfaceHandle(), width, height);

	ScopedPtr<Shader> shader = MakeScopedPtr<Shader>(m_Device->GetDeviceHandle(), "res/shaders/vert.spv", "res/shaders/frag.spv");

	CreatePipeline(shader.get());
	CreateFramebuffers();
	AllocateCommandBuffers();
	CreateSyncObjects();
}

GraphicsPipeline::~GraphicsPipeline()
{
	for (auto& framebuffer : m_Framebuffers) 
		vkDestroyFramebuffer(m_Device->GetDeviceHandle(), framebuffer, nullptr);

	vkDestroyPipeline(m_Device->GetDeviceHandle(), m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device->GetDeviceHandle(), m_PipelineLayout, nullptr);
	vkDestroyRenderPass(m_Device->GetDeviceHandle(), m_RenderPass, nullptr);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_Device->GetDeviceHandle(), m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device->GetDeviceHandle(), m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device->GetDeviceHandle(), m_InFlightFences[i], nullptr);
	}
		
	vkDestroyCommandPool(m_Device->GetDeviceHandle(), m_CommandPool, nullptr);
}

void GraphicsPipeline::Present()
{
	static uint32_t currentFrame = 0;

	vkWaitForFences(m_Device->GetDeviceHandle(), 1, &m_InFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_Device->GetDeviceHandle(), m_SwapChain->GetSwapChainHandle(), UINT64_MAX, m_ImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) 
		vkWaitForFences(m_Device->GetDeviceHandle(), 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	m_ImagesInFlight[imageIndex] = m_InFlightFences[currentFrame];

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_Device->GetDeviceHandle(), 1, &m_InFlightFences[currentFrame]);

	RAYD_VK_VALIDATE(vkQueueSubmit(m_Device->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, m_InFlightFences[currentFrame]), 
		"Failed to submit command buffer to graphics queue!")
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain->GetSwapChainHandle() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(m_Device->GetQueueFamilies().Present.Queue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void GraphicsPipeline::Shutdown()
{
	vkDeviceWaitIdle(m_Device->GetDeviceHandle());
	delete m_VertexBuffer;
	delete m_IndexBuffer;
}

void GraphicsPipeline::CreatePipeline(Shader* shader)
{
	//Configure pipeline
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = shader->GetVertexShaderModule();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = shader->GetFragmentShaderModule();
	fragShaderStageInfo.pName = "main";

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, Position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, Color);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
	rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	RAYD_VK_VALIDATE(vkCreatePipelineLayout(m_Device->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout),
		"Failed to create pipeline layout!");

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChain->GetFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	RAYD_VK_VALIDATE(vkCreateRenderPass(m_Device->GetDeviceHandle(), &renderPassInfo, nullptr, &m_RenderPass),
		"Failed to create render pass!");

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
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;

	RAYD_VK_VALIDATE(vkCreateGraphicsPipelines(m_Device->GetDeviceHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline),
		"Failed to create graphics pipeline!");
}

void GraphicsPipeline::CreateFramebuffers()
{
	m_Framebuffers.resize(m_SwapChain->GetImageViews().size());

	for (size_t i = 0; i < m_Framebuffers.size(); i++) {
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &m_SwapChain->GetImageViews()[i];
		framebufferInfo.width = m_SwapChain->GetExtent().width;
		framebufferInfo.height = m_SwapChain->GetExtent().height;
		framebufferInfo.layers = 1;

		RAYD_VK_VALIDATE(vkCreateFramebuffer(m_Device->GetDeviceHandle(), &framebufferInfo, nullptr, &m_Framebuffers[i]),
			"Failed to create framebuffer!");
	}
}

void GraphicsPipeline::AllocateCommandBuffers()
{
	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = *m_Device->GetQueueFamilies().Graphics.Index;

	RAYD_VK_VALIDATE(vkCreateCommandPool(m_Device->GetDeviceHandle(), &commandPoolInfo, nullptr, &m_CommandPool),
		"Failed to create command pool!");

	m_VertexBuffer = new VertexBuffer(m_Device, m_CommandPool, vertices.size() * sizeof(Vertex), vertices.data());
	m_IndexBuffer = new IndexBuffer(m_Device, m_CommandPool, indices.size() * sizeof(uint16_t), indices.data());

	m_CommandBuffers.resize(m_Framebuffers.size());

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = m_CommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

	RAYD_VK_VALIDATE(vkAllocateCommandBuffers(m_Device->GetDeviceHandle(), &allocateInfo, m_CommandBuffers.data()),
		"Failed to allocate command buffers!");

	for (uint32_t i = 0; i < m_CommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		RAYD_VK_VALIDATE(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo), "Failed to start recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Framebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain->GetExtent();
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
		m_VertexBuffer->Bind(m_CommandBuffers[i]);
		m_IndexBuffer->Bind(m_CommandBuffers[i]);

		vkCmdDrawIndexed(m_CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers[i]);
		RAYD_VK_VALIDATE(vkEndCommandBuffer(m_CommandBuffers[i]), "Failed to record command buffer!");
	}
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
