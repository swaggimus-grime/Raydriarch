#include "raydpch.h"
#include "SwapChain.h"

static VkDevice logicalDevice;

SwapChain::SwapChain(Device* device, VkSurfaceKHR& surface, 
    uint32_t framebufferWidth, uint32_t framebufferHeight)
{
    logicalDevice = device->GetDeviceHandle();
    auto& physicalDevice = device->GetPhysicalDeviceHandle();
    auto& details = device->GetSwapChainSupportDetails();
	VkSurfaceFormatKHR& surfaceFormat = FindSurfaceFormat(physicalDevice, details, surface);
	VkPresentModeKHR presentMode = FindPresentMode(physicalDevice, details, surface);
    VkExtent2D& swapExtent = FindSwapExtent(details, framebufferWidth, framebufferHeight);

    uint32_t imageCount = details.Capabilities.minImageCount + 1;
    imageCount = std::clamp(imageCount, details.Capabilities.minImageCount, details.Capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = swapExtent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto& queueFamilies = device->GetQueueFamilies();
    uint32_t queueFamilyIndices[] = { *queueFamilies.Graphics.Index, *queueFamilies.Present.Index };

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.queueFamilyIndexCount = 0;
        swapChainInfo.pQueueFamilyIndices = nullptr;
    }
       
    swapChainInfo.preTransform = details.Capabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = true;
    swapChainInfo.oldSwapchain = nullptr;

    RAYD_VK_VALIDATE(vkCreateSwapchainKHR(logicalDevice, &swapChainInfo, nullptr, &m_SwapChain), 
        "Failed to create swap chain!");
}

SwapChain::~SwapChain()
{
    vkDestroySwapchainKHR(logicalDevice, m_SwapChain, nullptr);
}

VkSurfaceFormatKHR SwapChain::FindSurfaceFormat(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface)
{
	for (const auto& availableFormat : details.Formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            return availableFormat;
    }

    return details.Formats[0];
}

VkPresentModeKHR SwapChain::FindPresentMode(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface)
{
	for (const auto& availablePresentMode : details.PresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
			return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::FindSwapExtent(const SwapChainSupportDetails& details, uint32_t framebufferWidth, uint32_t framebufferHeight)
{
    if (details.Capabilities.currentExtent.width != UINT32_MAX) 
        return details.Capabilities.currentExtent;

    VkExtent2D actualExtent = {
        framebufferWidth,
        framebufferHeight
    };

    auto& minExtent = details.Capabilities.minImageExtent;
    auto& maxExtent = details.Capabilities.maxImageExtent;
    actualExtent.width = std::clamp(actualExtent.width, minExtent.width, maxExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, minExtent.height, maxExtent.height);

    return actualExtent;
}
