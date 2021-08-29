#pragma once

#include "GraphicsCore.h"

#include "Device.h"

class DescriptorPool {
public:
	DescriptorPool(Device* device, VkDescriptorType type, uint32_t descriptorCount);
	~DescriptorPool();
private:
	VkDescriptorPool m_Pool;


};