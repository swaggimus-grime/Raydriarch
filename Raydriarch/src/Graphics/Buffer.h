#pragma once

#include "GraphicsCore.h"

#include "Device.h"

class Buffer {
public:
	~Buffer();

	inline const VkBuffer& GetBufferHandle() const { return m_Buffer; }

	void Map(VkDeviceSize size, void* data);
protected:
	void CreateAndAllocateBuffer(VkBuffer& buffer, VkBufferUsageFlags usage, VkDeviceMemory& memory, VkMemoryPropertyFlags memFlags);
	uint32_t FindMemoryTypeIndex(uint32_t typeMask, VkMemoryPropertyFlags propFlags);
	void MapData(VkDeviceMemory& memory, const void* data);
	void CopyBuffer(const VkCommandPool& commandPool, VkBuffer& srcBuffer, VkBuffer& dstBuffer);

protected:
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	VkDeviceSize m_Size;

	RefPtr<Device> m_Device;
};

class VertexBuffer : public Buffer {
public:
	VertexBuffer(RefPtr<Device> device, VkCommandPool& commandPool, uint32_t vertexCount, VkDeviceSize size, const void* data);

	void Bind(VkCommandBuffer& cmdBuffer);

	inline uint32_t GetVertexCount() const { return m_VertexCount; }
private:
	uint32_t m_VertexCount;
};

class IndexBuffer : public Buffer {
public:
	IndexBuffer(RefPtr<Device> device, VkCommandPool& commandPool, uint32_t indexCount, VkDeviceSize size, const void* data);
	
	void Bind(VkCommandBuffer& cmdBuffer);

	inline uint32_t GetIndexCount() const { return m_IndexCount; }
private:
	uint32_t m_IndexCount;
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer(RefPtr<Device> device, VkDeviceSize size);

private:
	
};
