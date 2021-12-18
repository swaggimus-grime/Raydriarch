#pragma once

#include "GraphicsCore.h"

struct QueueFamily {
	std::optional<uint32_t> Index;
	VkQueueFamilyProperties Properties;
	VkQueue Queue;
};

struct QueueFamilies {
	QueueFamily Graphics;
	QueueFamily Present;

	inline bool IsComplete() {
		return Graphics.Index && Present.Index;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

class Device {
public:
	Device(VkInstance& instance, ScopedPtr<class Surface>& surface, VkPhysicalDeviceFeatures& desiredFeatures);
	~Device();

	inline const VkPhysicalDevice& GetPhysicalDeviceHandle() const { return m_PhysicalDevice; }
	inline const VkDevice& GetDeviceHandle() const { return m_Device; }

	void UpdateSwapChainSupportDetails(VkSurfaceKHR& surface);
	inline const SwapChainSupportDetails& GetSwapChainSupportDetails() const { return m_SwapChainSupportDetails; }

	inline const QueueFamilies& GetQueueFamilies() const { return m_QueueFamilies; }

	inline void Join() const { vkDeviceWaitIdle(m_Device); }
private:
	VkPhysicalDevice FindPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface, VkPhysicalDeviceFeatures& desiredFeatures);
	bool ExtensionsSupported(VkPhysicalDevice& physicalDevice);
	bool FeaturesSupported(VkPhysicalDevice& physicalDevice, VkPhysicalDeviceFeatures& desiredFeatures);
	bool SwapChainSupported(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

	VkDevice FindDevice(VkInstance& instance, VkPhysicalDeviceFeatures& desiredFeatures);

	QueueFamilies& FindQueueFamilies(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
private:
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;

	const std::vector<const char*> m_Extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	SwapChainSupportDetails m_SwapChainSupportDetails;

	QueueFamilies m_QueueFamilies;
};