#include "raydpch.h"
#include "Command.h"

static struct CommandObjects {
	RefPtr<Device> GPU;
	VkCommandPool Pool;
}*s_Objects = new CommandObjects;

void Command::Init(RefPtr<Device> device, VkCommandPool& cmdPool)
{
	s_Objects->GPU = device;
	s_Objects->Pool = cmdPool;
}

void Command::Shutdown()
{
	delete s_Objects;
}

VkCommandBuffer Command::BeginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = s_Objects->Pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(s_Objects->GPU->GetDeviceHandle(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Command::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(s_Objects->GPU->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(s_Objects->GPU->GetQueueFamilies().Graphics.Queue);

	vkFreeCommandBuffers(s_Objects->GPU->GetDeviceHandle(), s_Objects->Pool, 1, &commandBuffer);
}



