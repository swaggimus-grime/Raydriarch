#include "raydpch.h"
#include "Device.h"

Device::Device(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures)
{
	m_PhysicalDevice = FindPhysicalDevice(instance, desiredFeatures);
	m_Device = FindDevice(instance, desiredFeatures);
}

Device::~Device()
{
	vkDestroyDevice(m_Device, nullptr);
}

VkPhysicalDevice Device::FindPhysicalDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures)
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	RAYD_ASSERT(physicalDeviceCount, "Failed to find any physical devices!");

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	for (auto& physicalDevice : physicalDevices) {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {

			bool featuresFound = true;
			for (uint32_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
				auto desiredFeature = *(VkBool32*)((char*)&desiredFeatures + i * sizeof(VkBool32));
				auto feature = *(VkBool32*)((char*)&physicalDeviceFeatures + i * sizeof(VkBool32));
				if (desiredFeature > 0 && desiredFeature != feature) {
					featuresFound = false;
					break;
				}
			}

			if (featuresFound) {
				auto& queueFamilies = FindQueueFamilies(physicalDevice);
				if (queueFamilies.IsComplete()) {
					m_QueueFamilies = queueFamilies;
					return physicalDevice;
				}
			}
			
		}	
	}

	RAYD_ERROR("Failed to find physical device with given features!");
}

VkDevice Device::FindDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures)
{
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = m_QueueFamilies.Graphics.Index;
	queueInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.queueCreateInfoCount = 1;

	deviceInfo.pEnabledFeatures = &desiredFeatures;

	VkDevice device;
	RAYD_VK_VALIDATE(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &device), "Failed to create device!");
	return device;
}

QueueFamilies& Device::FindQueueFamilies(VkPhysicalDevice& physicalDevice)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	QueueFamilies queueFamilies;
	bool complete = false;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {

		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			QueueFamily graphicsQueueFamily;
			graphicsQueueFamily.Index = i;
			graphicsQueueFamily.Properties = queueFamilyProperties[i];
			queueFamilies.Graphics = graphicsQueueFamily;
		}

		if (queueFamilies.IsComplete()) {
			complete = true;
			break;
		}
	}

	RAYD_ASSERT(complete, "Failed to find necessary queue families!");

	return queueFamilies;
}
