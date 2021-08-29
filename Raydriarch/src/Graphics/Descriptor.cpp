#include "raydpch.h"
#include "Descriptor.h"

DescriptorPool::DescriptorPool(Device* device, VkDescriptorType type, uint32_t descriptorCount)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = type;
	uboLayoutBinding.descriptorCount = 1;


	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(descriptorCount);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = descriptorCount;

	RAYD_VK_VALIDATE(vkCreateDescriptorPool(device->GetDeviceHandle(), &poolInfo, nullptr, &m_Pool), "Failed to create descriptor pool!");

	//std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	//VkDescriptorSetAllocateInfo allocInfo{};
	//allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	//allocInfo.descriptorPool = descriptorPool;
	//allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	//allocInfo.pSetLayouts = layouts.data();
}

DescriptorPool::~DescriptorPool()
{
	//vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}
