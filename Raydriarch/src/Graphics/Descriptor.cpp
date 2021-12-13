#include "raydpch.h"
#include "Descriptor.h"

DescriptorPool::DescriptorPool(RefPtr<Device> device, uint32_t numSwapcbainImages, uint32_t numSizes, const VkDescriptorPoolSize* sizes)
	:m_Device(device)
{
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = numSizes;
	poolInfo.pPoolSizes = sizes;
	poolInfo.maxSets = numSwapcbainImages;

	RAYD_VK_VALIDATE(vkCreateDescriptorPool(m_Device->GetDeviceHandle(), &poolInfo, nullptr, &m_Pool), "Failed to create descriptor pool!");
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_Device->GetDeviceHandle(), m_Pool, nullptr);
}

DescriptorSetLayout::DescriptorSetLayout(RefPtr<Device> device, uint32_t numBindings, const VkDescriptorSetLayoutBinding* bindings)
	:m_Device(device)
{
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = numBindings;
	layoutInfo.pBindings = bindings;

	RAYD_VK_VALIDATE(vkCreateDescriptorSetLayout(m_Device->GetDeviceHandle(), &layoutInfo, nullptr, &m_Layout), "Failed to create descriptor set layout!");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(m_Device->GetDeviceHandle(), m_Layout, nullptr);
}
