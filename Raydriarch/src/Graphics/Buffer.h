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

protected:
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;
	VkDeviceSize m_Size;

	RefPtr<Device> m_Device;
};

struct Attribute {
	uint32_t Binding;
	uint32_t Location;
	VkFormat Format;
};

class VertexBuffer : public Buffer {
public:
	VertexBuffer(RefPtr<Device> device, uint32_t vertexCount, VkDeviceSize size, const void* data);

	void Bind(VkCommandBuffer& cmdBuffer);

	inline uint32_t GetVertexCount() const { return m_VertexCount; }
private:
	uint32_t m_VertexCount;
};

class IndexBuffer : public Buffer {
public:
	IndexBuffer(RefPtr<Device> device, uint32_t indexCount, VkDeviceSize size, const void* data);
	
	void Bind(VkCommandBuffer& cmdBuffer);

	inline uint32_t GetIndexCount() const { return m_IndexCount; }
private:
	uint32_t m_IndexCount;
};

class VertexLayout {
public:
	VertexLayout() = default;
	void AddAttribute(uint32_t location, uint32_t binding, VkFormat format);
	void AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);

	inline const uint32_t GetNumAttributes() const { return m_AttribDescriptions.size(); }
	inline const VkVertexInputAttributeDescription* GetAttributes() const { return m_AttribDescriptions.data(); }
	inline const VkVertexInputBindingDescription* GetBindings() const { return m_Bindings.data(); }
private:
	uint32_t CalculateOffset(VkFormat format);
private:
	std::vector<VkVertexInputAttributeDescription> m_AttribDescriptions;
	std::vector<VkVertexInputBindingDescription> m_Bindings;
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer(RefPtr<Device> device, VkDeviceSize size);

private:
	
};
