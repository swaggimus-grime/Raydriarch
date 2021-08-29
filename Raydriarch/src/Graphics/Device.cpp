#include "raydpch.h"
#include "Device.h"

Device::Device(VkInstance& instance, VkSurfaceKHR& surface, VkPhysicalDeviceFeatures& desiredFeatures)
{
	m_PhysicalDevice = FindPhysicalDevice(instance, surface, desiredFeatures);
	m_Device = FindDevice(instance, desiredFeatures);

	vkGetDeviceQueue(m_Device, *m_QueueFamilies.Graphics.Index, 0, &m_QueueFamilies.Graphics.Queue);
	vkGetDeviceQueue(m_Device, *m_QueueFamilies.Present.Index, 0, &m_QueueFamilies.Present.Queue);
}

Device::~Device()
{
	vkDestroyDevice(m_Device, nullptr);
}

void Device::UpdateSwapChainSupportDetails(VkSurfaceKHR& surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, surface,
		&m_SwapChainSupportDetails.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, nullptr);
	if (formatCount > 0) {
		m_SwapChainSupportDetails.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount,
			m_SwapChainSupportDetails.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount > 0) {
		m_SwapChainSupportDetails.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount,
			m_SwapChainSupportDetails.PresentModes.data());
	}
}

VkPhysicalDevice Device::FindPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface, VkPhysicalDeviceFeatures& desiredFeatures)
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	RAYD_ASSERT(physicalDeviceCount, "Failed to find any physical devices!");
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	for (auto& physicalDevice : physicalDevices) {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			if (FeaturesSupported(physicalDevice, desiredFeatures) && ExtensionsSupported(physicalDevice) && SwapChainSupported(physicalDevice, surface)) {
				m_QueueFamilies = FindQueueFamilies(physicalDevice, surface);
				return physicalDevice;
			}
		}	
	}

	RAYD_ERROR("Failed to find physical device with given features!");
}

bool Device::FeaturesSupported(VkPhysicalDevice& physicalDevice, VkPhysicalDeviceFeatures& desiredFeatures)
{
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	for (uint32_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
		VkBool32 desiredFeature = *(VkBool32*)((char*)&desiredFeatures + i * sizeof(VkBool32));
		VkBool32 feature = *(VkBool32*)((char*)&physicalDeviceFeatures + i * sizeof(VkBool32));
		if (desiredFeature > 0 && desiredFeature != feature) {
			return false;
		}
	}

	return true;
}

bool Device::ExtensionsSupported(VkPhysicalDevice& physicalDevice)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	bool extensionFound = false;
	for (auto& reqExtension : m_Extensions) {
		extensionFound = false;

		for (auto& availableExtension : availableExtensions) {
			if (strcmp(reqExtension, availableExtension.extensionName) == 0) {
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound)
			return false;
	}

	return true;
}

bool Device::SwapChainSupported(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, 
		&m_SwapChainSupportDetails.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount > 0) {
		m_SwapChainSupportDetails.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
			m_SwapChainSupportDetails.Formats.data());
	}
		
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount > 0) {
		m_SwapChainSupportDetails.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,
			m_SwapChainSupportDetails.PresentModes.data());
	}
		
	return !m_SwapChainSupportDetails.Formats.empty() && !m_SwapChainSupportDetails.PresentModes.empty();
}

VkDevice Device::FindDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures)
{
	const uint32_t numQueueFamilies = sizeof(QueueFamilies) / sizeof(QueueFamily);
	std::array<VkDeviceQueueCreateInfo, numQueueFamilies> queueInfos;
	uint32_t queueFamilyIndices[numQueueFamilies] = { *m_QueueFamilies.Graphics.Index, *m_QueueFamilies.Present.Index };

	float queuePriority = 1.0f;
	for (uint32_t i = 0; i < queueInfos.size(); i++) {
		queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[i].queueFamilyIndex = queueFamilyIndices[i];
		queueInfos[i].queueCount = 1;
		queueInfos[i].pQueuePriorities = &queuePriority;

		queueInfos[i].pNext = nullptr;
		queueInfos[i].flags = NULL;
	}
	
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = queueInfos.data();
	deviceInfo.queueCreateInfoCount = queueInfos.size();
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_Extensions.size());
	deviceInfo.ppEnabledExtensionNames = m_Extensions.data();

	//Deprecated device-specific validation layers
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	deviceInfo.pEnabledFeatures = &desiredFeatures;

	VkDevice device;
	RAYD_VK_VALIDATE(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &device), "Failed to create device!");
	return device;
}

QueueFamilies& Device::FindQueueFamilies(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	QueueFamilies queueFamilies{};
	bool complete = false;
	for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {

		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			QueueFamily graphicsQueueFamily{ i, queueFamilyProperties[i] };
			queueFamilies.Graphics = graphicsQueueFamily;
			continue;
		}

		VkBool32 presentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported);
		if (presentSupported) {
			QueueFamily presentQueueFamily{ i, queueFamilyProperties[i] };
			queueFamilies.Present = presentQueueFamily;
		}

		if (queueFamilies.IsComplete()) {
			complete = true;
			break;
		}
	}

	RAYD_ASSERT(complete, "Failed to find necessary queue families!");

	return queueFamilies;
}
