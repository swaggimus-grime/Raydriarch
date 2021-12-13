#include "raydpch.h"
#include "SwapChain.h"

static VkDevice logicalDevice;

SwapChain::SwapChain(RefPtr<Device> device, VkSurfaceKHR& surface, 
    uint32_t framebufferWidth, uint32_t framebufferHeight)
    :m_Device(device)
{
    m_Device->UpdateSwapChainSupportDetails(surface);

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

    for (uint32_t i = 0; i < m_Images.size(); i++) 
        m_ImageViews[i] = MakeScopedPtr<ImageView>(m_Device, m_Images[i], m_Format);

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_Format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_RenderPass = MakeScopedPtr<RenderPass>(m_Device, &colorAttachment, &subpass, &dependency);

    m_Framebuffers.resize(m_ImageViews.size());

    for (size_t i = 0; i < m_Framebuffers.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass->GetRenderPassHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_ImageViews[i]->GetHandle();
        framebufferInfo.width = m_Extent.width;
        framebufferInfo.height = m_Extent.height;
        framebufferInfo.layers = 1;

        RAYD_VK_VALIDATE(vkCreateFramebuffer(m_Device->GetDeviceHandle(), &framebufferInfo, nullptr, &m_Framebuffers[i]),
            "Failed to create framebuffer!");
    }
}

SwapChain::~SwapChain()
{
    for (auto& framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);

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
