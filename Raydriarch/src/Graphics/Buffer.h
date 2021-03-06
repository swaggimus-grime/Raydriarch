#pragma once

#include "GraphicsCore.h"

#include "Device.h"

class Buffer {
public:
	~Buffer();
protected:
	void Create(VkBuffer& buffer, VkDeviceMemory& memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags);
	void Copy(VkDeviceSize size, VkBuffer& srcBuffer, VkBuffer& dstBuffer);
	void Map(VkDeviceMemory& memory, VkDeviceSize size, const void* data);
	void Allocate(VkDeviceMemory& memory, VkMemoryRequirements& memReqs, VkMemoryPropertyFlags memFlags);

protected:
	RefPtr<Device> m_Device;
	VkDeviceMemory m_Memory;
};

struct Attribute {
	uint32_t Binding;
	uint32_t Location;
	VkFormat Format;
};

class VertexBuffer : public Buffer {
public:
	VertexBuffer(RefPtr<Device> device, uint32_t vertexCount, VkDeviceSize size, const void* data);
	~VertexBuffer();
	void Bind(VkCommandBuffer& cmdBuffer);
	inline const VkBuffer& GetBufferHandle() const { return m_Buffer; }
	inline uint32_t GetVertexCount() const { return m_VertexCount; }
private:
	VkBuffer m_Buffer;
	VkDeviceSize m_Size;

	uint32_t m_VertexCount;
};

class IndexBuffer : public Buffer {
public:
	IndexBuffer(RefPtr<Device> device, uint32_t indexCount, VkDeviceSize size, const void* data);
	~IndexBuffer();
	void Bind(VkCommandBuffer& cmdBuffer);
	inline const VkBuffer& GetBufferHandle() const { return m_Buffer; }
	inline uint32_t GetIndexCount() const { return m_IndexCount; }
private:
	VkBuffer m_Buffer;
	VkDeviceSize m_Size;

	uint32_t m_IndexCount;
};

class VertexLayout {
public:
	VertexLayout() : m_Offset(0) {}
	void AddAttribute(uint32_t location, uint32_t binding, VkFormat format);
	void AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);

	inline const std::vector<VkVertexInputAttributeDescription>& GetAttributes() { return m_AttribDescriptions; }
	inline const std::vector<VkVertexInputBindingDescription>& GetBindings() { return m_Bindings; }

private:
	std::vector<VkVertexInputAttributeDescription> m_AttribDescriptions;
	std::vector<VkVertexInputBindingDescription> m_Bindings;
	uint32_t m_Offset;
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer(RefPtr<Device> device, VkDeviceSize size);
	~UniformBuffer();
	void Update(VkDeviceSize size, const void* data);
	inline const VkBuffer& GetBufferHandle() const { return m_Buffer; }
private:
	VkBuffer m_Buffer;
	VkDeviceSize m_Size;
};
