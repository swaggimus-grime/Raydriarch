#pragma once

#include "GraphicsCore.h"

#include "Surface.h"
#include "Device.h"

class SwapChain {
public:
	SwapChain(Device* device, VkSurfaceKHR& surface, 
		uint32_t framebufferWidth, uint32_t framebufferHeight);
	~SwapChain();

	inline const VkSwapchainKHR& GetSwapChainHandle() const { return m_SwapChain; }

	inline VkExtent2D GetExtent() { return m_Extent; }
	inline VkFormat GetFormat() { return m_Format; }

	inline const std::vector<VkImage>& GetImages() const { return m_Images; }
	inline const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
private:
	VkSurfaceFormatKHR FindSurfaceFormat(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkPresentModeKHR FindPresentMode(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkExtent2D FindSwapExtent(const SwapChainSupportDetails& details, uint32_t framebufferWidth, uint32_t framebufferHeight);

private:
	VkSwapchainKHR m_SwapChain;
	VkFormat m_Format;
	VkExtent2D m_Extent;

	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
};