#pragma once

#include "GraphicsCore.h"
#include "Device.h"
#include "Buffer.h"

class Model {
public:
	Model(RefPtr<Device> device, const std::string& modelPath);
	void Render(VkCommandBuffer& cbuff);
	inline VertexLayout& GetVertexLayout() { return m_VLayout; }
private:
	RefPtr<Device> m_Device;
	ScopedPtr<VertexBuffer > m_VBuffer;
	ScopedPtr<IndexBuffer> m_IBuffer;
	VertexLayout m_VLayout;
};