#pragma once

#include "GraphicsCore.h"

struct QueueFamily {
	uint32_t Index = -1;
	VkQueueFamilyProperties Properties;
};

struct QueueFamilies {
	QueueFamily Graphics;
	QueueFamily Present;

	inline bool IsComplete() {
		return Graphics.Index >= 0 && Present.Index >= 0;
	}
};

class Device {
public:
	Device(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures);
	~Device();
private:
	QueueFamilies& FindQueueFamilies(VkPhysicalDevice& physicalDevice);

	VkPhysicalDevice FindPhysicalDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures);
	VkDevice FindDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures);
private:
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;

	QueueFamilies m_QueueFamilies;
};