#pragma once

#include "GraphicsCore.h"

#include "Surface.h"
#include "Device.h"

class SwapChain {
public:
	SwapChain(Device* device, VkSurfaceKHR& surface, 
		uint32_t framebufferWidth, uint32_t framebufferHeight);
	~SwapChain();

private:
	VkSurfaceFormatKHR FindSurfaceFormat(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkPresentModeKHR FindPresentMode(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkExtent2D FindSwapExtent(const SwapChainSupportDetails& details, uint32_t framebufferWidth, uint32_t framebufferHeight);

private:
	VkSwapchainKHR m_SwapChain;
	Surface* m_Surface;

};