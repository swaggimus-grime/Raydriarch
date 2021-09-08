#include "raydpch.h"
#include "Buffer.h"

Buffer::~Buffer()
{
	vkDestroyBuffer(m_Device->GetDeviceHandle(), m_Buffer, nullptr);
	vkFreeMemory(m_Device->GetDeviceHandle(), m_Memory, nullptr);
}

void Buffer::Map(VkDeviceSize size, void* data)
{
	void* mappedData;
	RAYD_VK_VALIDATE(vkMapMemory(m_Device->GetDeviceHandle(), m_Memory, 0, size, 0, &mappedData), "Failed to map to memory!");
	memcpy(mappedData, data, size);
	vkUnmapMemory(m_Device->GetDeviceHandle(), m_Memory);
}

void Buffer::MapData(VkDeviceMemory& memory, const void* data)
{
	void* mappedData;
	vkMapMemory(m_Device->GetDeviceHandle(), memory, 0, m_Size, 0, &mappedData);
	memcpy(mappedData, data, static_cast<size_t>(m_Size));
	vkUnmapMemory(m_Device->GetDeviceHandle(), memory);
}

void Buffer::CopyBuffer(const VkCommandPool& commandPool, VkBuffer& srcBuffer, VkBuffer& dstBuffer)
{

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_Device->GetDeviceHandle(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = m_Size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_Device->GetQueueFamilies().Graphics.Queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_Device->GetQueueFamilies().Graphics.Queue);

	vkFreeCommandBuffers(m_Device->GetDeviceHandle(), commandPool, 1, &commandBuffer);
}

void Buffer::CreateAndAllocateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceMemory& memory, VkMemoryPropertyFlags memFlags)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_Size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	RAYD_VK_VALIDATE(vkCreateBuffer(m_Device->GetDeviceHandle(), &bufferInfo, nullptr, &buffer), "Failed to create buffer!");

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_Device->GetDeviceHandle(), buffer, &memReqs);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = FindMemoryTypeIndex(memReqs.memoryTypeBits, memFlags);

	RAYD_VK_VALIDATE(vkAllocateMemory(m_Device->GetDeviceHandle(), &allocInfo, nullptr, &memory), "Failed to allocate memory!");
	vkBindBufferMemory(m_Device->GetDeviceHandle(), buffer, memory, 0);
}

uint32_t Buffer::FindMemoryTypeIndex(uint32_t typeMask, VkMemoryPropertyFlags propFlags)
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(m_Device->GetPhysicalDeviceHandle(), &memProps);

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((typeMask & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & propFlags) == propFlags) 
			return i;
	}

	RAYD_ERROR("Failed to find suitable memory type!");
}

VertexBuffer::VertexBuffer(RefPtr<Device> device, VkCommandPool& commandPool, uint32_t vertexCount, VkDeviceSize size, const void* data)
	:m_VertexCount(vertexCount)
{
	m_Device = device;
	m_Size = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMem;
	CreateAndAllocateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	MapData(stagingMem, data);

	CreateAndAllocateBuffer(m_Buffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_Memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CopyBuffer(commandPool, stagingBuffer, m_Buffer);

	vkDestroyBuffer(m_Device->GetDeviceHandle(), stagingBuffer, nullptr);
	vkFreeMemory(m_Device->GetDeviceHandle(), stagingMem, nullptr);
}

void VertexBuffer::Bind(VkCommandBuffer& cmdBuffer)
{
	const VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_Buffer, &offset);
}

IndexBuffer::IndexBuffer(RefPtr<Device> device, VkCommandPool& commandPool, uint32_t indexCount, VkDeviceSize size, const void* data)
	:m_IndexCount(indexCount)
{
	m_Device = device;
	m_Size = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMem;
	CreateAndAllocateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	MapData(stagingMem, data);

	CreateAndAllocateBuffer(m_Buffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_Memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CopyBuffer(commandPool, stagingBuffer, m_Buffer);

	vkDestroyBuffer(m_Device->GetDeviceHandle(), stagingBuffer, nullptr);
	vkFreeMemory(m_Device->GetDeviceHandle(), stagingMem, nullptr);
}

void IndexBuffer::Bind(VkCommandBuffer& cmdBuffer)
{
	vkCmdBindIndexBuffer(cmdBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT16);
}

UniformBuffer::UniformBuffer(RefPtr<Device> device, VkDeviceSize size)
{
	m_Device = device;
	m_Size = size;

	CreateAndAllocateBuffer(m_Buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, m_Memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}
