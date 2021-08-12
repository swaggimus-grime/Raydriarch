#pragma once

#include "GraphicsCore.h"

#include "Device.h"

struct Binding {

};

struct Attribute {
	VkVertexInputAttributeDescription Description;

	Attribute(VkFormat format) {
		Description.format = format;
	}
};

class Buffer {
public:
	virtual void Bind(VkCommandBuffer& cmdBuffer) = 0;

protected:
	void CreateAndAllocateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceMemory& memory, VkMemoryPropertyFlags memFlags);
	uint32_t FindMemoryTypeIndex(uint32_t typeMask, VkMemoryPropertyFlags propFlags);
	void MapData(VkDeviceMemory& memory, const void* data);
	void CopyBuffer(const VkCommandPool& commandPool, VkBuffer& srcBuffer, VkBuffer& dstBuffer);

protected:
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	VkDeviceSize m_Size;

	const Device* m_Device;
};

class VertexBuffer : public Buffer {
public:
	VertexBuffer(Device* device, VkCommandPool& commandPool, VkDeviceSize size, const void* data);
	~VertexBuffer();

	virtual void Bind(VkCommandBuffer& cmdBuffer) override;

private:
	
};

class IndexBuffer : public Buffer {
public:
	IndexBuffer(Device* device, VkCommandPool& commandPool, VkDeviceSize size, const void* data);
	~IndexBuffer();

	virtual void Bind(VkCommandBuffer& cmdBuffer) override;

private:

};
