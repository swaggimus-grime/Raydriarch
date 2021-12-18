#pragma once

#include "GraphicsCore.h"

#include "Device.h"

class DescriptorSetLayout {
public:
	DescriptorSetLayout(RefPtr<Device> device, std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();

	inline const VkDescriptorSetLayout& GetLayoutHandle() const { return m_Layout; }
private:
	RefPtr<Device> m_Device;

	VkDescriptorSetLayout m_Layout;
};

class DescriptorPool {
public:
	DescriptorPool(RefPtr<Device> device, uint32_t numSwapcbainImages, std::vector<VkDescriptorPoolSize>& sizes);
	~DescriptorPool();

	inline const VkDescriptorPool& GetPoolHandle() const { return m_Pool; }
private:
	RefPtr<Device> m_Device;
	VkDescriptorPool m_Pool;
};

class DescriptorSet {
public:
	DescriptorSet();
};