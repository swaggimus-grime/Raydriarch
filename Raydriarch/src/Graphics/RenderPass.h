#pragma once

#include "GraphicsCore.h"

#include "Device.h"

struct Attachment {
	VkAttachmentDescription Description;
	VkAttachmentReference Reference;
};

class RenderPass {
public:
	RenderPass(RefPtr<Device> device, VkAttachmentDescription* attachmentDescriptions, VkSubpassDescription* subpassDescriptions, VkSubpassDependency* subpassDependencies);
	~RenderPass() { vkDestroyRenderPass(m_Device->GetDeviceHandle(), m_RenderPass, nullptr); }

	inline const VkRenderPass& GetHandle() const { return m_RenderPass; }
private:
	RefPtr<Device> m_Device;

	VkRenderPass m_RenderPass;
};