#include "raydpch.h"
#include "Command.h"

static struct CommandObjects {
	RefPtr<Device> GPU;
	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;
}* s_Objects = new CommandObjects;

void Command::Init(RefPtr<Device> device, uint8_t numBuffers)
{
	s_Objects->GPU = device;

	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = *s_Objects->GPU->GetQueueFamilies().Graphics.Index;

	RAYD_VK_VALIDATE(vkCreateCommandPool(s_Objects->GPU->GetDeviceHandle(), &commandPoolInfo, nullptr, &s_Objects->CommandPool),
		"Failed to create command pool!");

	s_Objects->CommandBuffers.resize(numBuffers);

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = s_Objects->CommandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = (uint32_t)s_Objects->CommandBuffers.size();

	RAYD_VK_VALIDATE(vkAllocateCommandBuffers(s_Objects->GPU->GetDeviceHandle(), &allocateInfo, s_Objects->CommandBuffers.data()),
		"Failed to allocate command buffers!");
}

void Command::CopyBuffer(VkDeviceSize size, VkBuffer& srcBuffer, VkBuffer& dstBuffer)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = s_Objects->CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(s_Objects->GPU->GetDeviceHandle(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(s_Objects->GPU->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(s_Objects->GPU->GetQueueFamilies().Graphics.Queue);

	vkFreeCommandBuffers(s_Objects->GPU->GetDeviceHandle(), s_Objects->CommandPool, 1, &commandBuffer);
}

void Command::DrawIndexed(SceneData* sceneData, GraphicsObjects* graphicsObjects)
{
	for (uint32_t i = 0; i < s_Objects->CommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		RAYD_VK_VALIDATE(vkBeginCommandBuffer(s_Objects->CommandBuffers[i], &beginInfo), "Failed to start recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = graphicsObjects->SC->GetRenderPass().GetRenderPassHandle();
		renderPassInfo.framebuffer = graphicsObjects->SC->GetFramebuffers()[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = graphicsObjects->SC->GetExtent();
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(s_Objects->CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(s_Objects->CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->Pipeline->GetPipelineHandle());
		vkCmdBindDescriptorSets(s_Objects->CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, sceneData->Pipeline->GetPipelineLayout(), 0, 1, &sceneData->Pipeline->GetDescriptorSets()[i], 0, nullptr);
		sceneData->VBuffer->Bind(s_Objects->CommandBuffers[i]);
		sceneData->IBuffer->Bind(s_Objects->CommandBuffers[i]);

		vkCmdDrawIndexed(s_Objects->CommandBuffers[i], static_cast<uint32_t>(sceneData->IBuffer->GetIndexCount()), 1, 0, 0, 0);

		vkCmdEndRenderPass(s_Objects->CommandBuffers[i]);
		RAYD_VK_VALIDATE(vkEndCommandBuffer(s_Objects->CommandBuffers[i]), "Failed to record command buffer!");
	}
}

void Command::Shutdown()
{
	vkDeviceWaitIdle(s_Objects->GPU->GetDeviceHandle());
	vkDestroyCommandPool(s_Objects->GPU->GetDeviceHandle(), s_Objects->CommandPool, nullptr);
	delete s_Objects;
}

VkCommandBuffer* Command::GetCommandBuffers()
{
	return s_Objects->CommandBuffers.data();
}

