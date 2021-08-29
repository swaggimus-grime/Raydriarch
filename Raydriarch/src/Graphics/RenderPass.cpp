#include "raydpch.h"
#include "RenderPass.h"

RenderPass::RenderPass(RefPtr<Device> device, VkAttachmentDescription* attachmentDescriptions,
	VkSubpassDescription* subpassDescriptions, VkSubpassDependency* subpassDependencies)
	:m_Device(device)
{
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = attachmentDescriptions;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = subpassDescriptions;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = subpassDependencies;

	RAYD_VK_VALIDATE(vkCreateRenderPass(m_Device->GetDeviceHandle(), &renderPassInfo, nullptr, &m_RenderPass),
		"Failed to create render pass!");
}
