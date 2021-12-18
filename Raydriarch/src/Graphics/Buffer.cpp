#include "raydpch.h"
#include "Buffer.h"

#include "Command.h"

Buffer::~Buffer()
{
	vkFreeMemory(m_Device->GetDeviceHandle(), m_Memory, nullptr);
}

void Buffer::Copy(VkDeviceSize size, VkBuffer& srcBuffer, VkBuffer& dstBuffer)
{
	VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	Command::EndSingleTimeCommands(commandBuffer);
}

void Buffer::Map(VkDeviceMemory& memory, VkDeviceSize size, const void* data)
{
	void* mappedData;
	RAYD_VK_VALIDATE(vkMapMemory(m_Device->GetDeviceHandle(), memory, 0, size, 0, &mappedData), "Failed to map to memory!");
	memcpy(mappedData, data, size);
	vkUnmapMemory(m_Device->GetDeviceHandle(), memory);
}

void Buffer::Allocate(VkDeviceMemory& memory, VkMemoryRequirements& memReqs, VkMemoryPropertyFlags memFlags)
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReqs.size;
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(m_Device->GetPhysicalDeviceHandle(), &memProps);

	int32_t memTypeIndex = -1;
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((memReqs.memoryTypeBits & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & memFlags) == memFlags) {
			memTypeIndex = i;
			break;
		}
	}

	RAYD_ASSERT(memTypeIndex >= 0, "Failed to find suitable memory type!");
	allocInfo.memoryTypeIndex = memTypeIndex;

	RAYD_VK_VALIDATE(vkAllocateMemory(m_Device->GetDeviceHandle(), &allocInfo, nullptr, &memory), "Failed to allocate memory!");
}

void Buffer::Create(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
 	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	RAYD_VK_VALIDATE(vkCreateBuffer(m_Device->GetDeviceHandle(), &bufferInfo, nullptr, &buffer), "Failed to create buffer!");

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_Device->GetDeviceHandle(), buffer, &memReqs);

	Allocate(memory, memReqs, memFlags);
	vkBindBufferMemory(m_Device->GetDeviceHandle(), buffer, memory, 0);
}

VertexBuffer::VertexBuffer(RefPtr<Device> device, uint32_t vertexCount, VkDeviceSize size, const void* data)
	:m_VertexCount(vertexCount)
{
	m_Device = device;
	m_Size = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMem;
	Create(stagingBuffer, stagingMem, m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	Map(stagingMem, m_Size, data);

	Create(m_Buffer, m_Memory, m_Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Copy(m_Size, stagingBuffer, m_Buffer);

	vkDestroyBuffer(m_Device->GetDeviceHandle(), stagingBuffer, nullptr);
	vkFreeMemory(m_Device->GetDeviceHandle(), stagingMem, nullptr);
}

VertexBuffer::~VertexBuffer()
{
	vkDestroyBuffer(m_Device->GetDeviceHandle(), m_Buffer, nullptr);
}

void VertexBuffer::Bind(VkCommandBuffer& cmdBuffer)
{
	const VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_Buffer, &offset);
}

IndexBuffer::IndexBuffer(RefPtr<Device> device, uint32_t indexCount, VkDeviceSize size, const void* data)
	:m_IndexCount(indexCount)
{
	m_Device = device;
	m_Size = size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMem;
	Create(stagingBuffer, stagingMem, m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	Map(stagingMem, m_Size, data);

	Create(m_Buffer, m_Memory, m_Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	Copy(m_Size, stagingBuffer, m_Buffer);

	vkDestroyBuffer(m_Device->GetDeviceHandle(), stagingBuffer, nullptr);
	vkFreeMemory(m_Device->GetDeviceHandle(), stagingMem, nullptr);
}

IndexBuffer::~IndexBuffer()
{
	vkDestroyBuffer(m_Device->GetDeviceHandle(), m_Buffer, nullptr);
}

void IndexBuffer::Bind(VkCommandBuffer& cmdBuffer)
{
	vkCmdBindIndexBuffer(cmdBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT16);
}

UniformBuffer::UniformBuffer(RefPtr<Device> device, VkDeviceSize size)
{
	m_Device = device;
	m_Size = size;

	Create(m_Buffer, m_Memory, m_Size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

UniformBuffer::~UniformBuffer()
{
	vkDestroyBuffer(m_Device->GetDeviceHandle(), m_Buffer, nullptr);
}

void UniformBuffer::Update(VkDeviceSize size, const void* data)
{
	Map(m_Memory, size, data);
}

uint32_t VertexLayout::CalculateOffset(VkFormat format)
{
	uint32_t offset = 0;
	switch (format) {
	case VK_FORMAT_R32G32_SFLOAT:
		offset = 2 * 4;
		break;
	case VK_FORMAT_R32G32B32_SFLOAT:
		offset = 3 * 4;
		break;
	default:
		RAYD_ERROR("Unsupported format!");
	}

	return offset;
}

void VertexLayout::AddAttribute(uint32_t location, uint32_t binding, VkFormat format)
{
	VkVertexInputAttributeDescription desc;
	desc.binding = binding;
	desc.location = location;
	desc.format = format;
	desc.offset = CalculateOffset(format);
	m_AttribDescriptions.push_back(desc);
}

void VertexLayout::AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	VkVertexInputBindingDescription desc;
	desc.binding = binding;
	desc.stride = stride;
	desc.inputRate = inputRate;
	m_Bindings.push_back(desc);
}
