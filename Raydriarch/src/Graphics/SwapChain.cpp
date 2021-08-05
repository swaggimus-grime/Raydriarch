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
    m_Format = surfaceFormat.format;
	VkPresentModeKHR presentMode = FindPresentMode(physicalDevice, details, surface);
    m_Extent = FindSwapExtent(details, framebufferWidth, framebufferHeight);

    uint32_t imageCount = details.Capabilities.minImageCount + 1;
    imageCount = std::clamp(imageCount, details.Capabilities.minImageCount, details.Capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = m_Format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = m_Extent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto& queueFamilies = device->GetQueueFamilies();
    uint32_t queueFamilyIndices[] = { *queueFamilies.Graphics.Index, *queueFamilies.Present.Index };

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else 
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
       
    swapChainInfo.preTransform = details.Capabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = true;
    swapChainInfo.oldSwapchain = nullptr;

    RAYD_VK_VALIDATE(vkCreateSwapchainKHR(logicalDevice, &swapChainInfo, nullptr, &m_SwapChain), 
        "Failed to create swap chain!");

    vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &imageCount, m_Images.data());

    m_ImageViews.resize(m_Images.size());

    for (uint32_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = m_Images[i];
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = m_Format;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        RAYD_VK_VALIDATE(vkCreateImageView(logicalDevice, &imageViewInfo, nullptr, &m_ImageViews[i]), "Failed to create image view!");
    }
}

SwapChain::~SwapChain()
{
    for (auto& imageView : m_ImageViews) 
        vkDestroyImageView(logicalDevice, imageView, nullptr);

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
