#include "raydpch.h"
#include "SwapChain.h"

#include "Image.h"
#include "Surface.h"

SwapChain::SwapChain(RefPtr<Device> device, ScopedPtr<Surface>& surface,
    uint32_t framebufferWidth, uint32_t framebufferHeight)
    :m_Device(device)
{
    m_Device->UpdateSwapChainSupportDetails(surface->GetSurfaceHandle());
    auto& physicalDevice = device->GetPhysicalDeviceHandle();
    auto& details = device->GetSwapChainSupportDetails();
	VkSurfaceFormatKHR& surfaceFormat = FindSurfaceFormat(physicalDevice, details, surface->GetSurfaceHandle());
    m_Format = surfaceFormat.format;
	VkPresentModeKHR presentMode = FindPresentMode(physicalDevice, details, surface->GetSurfaceHandle());
    m_Extent = FindSwapExtent(details, framebufferWidth, framebufferHeight);

    uint32_t imageCount = details.Capabilities.minImageCount + 1;
    imageCount = std::clamp(imageCount, details.Capabilities.minImageCount, details.Capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapChainInfo{};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface->GetSurfaceHandle();
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
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = nullptr;

    RAYD_VK_VALIDATE(vkCreateSwapchainKHR(m_Device->GetDeviceHandle(), &swapChainInfo, nullptr, &m_SwapChain),
        "Failed to create swap chain!");

    vkGetSwapchainImagesKHR(m_Device->GetDeviceHandle(), m_SwapChain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device->GetDeviceHandle(), m_SwapChain, &imageCount, m_Images.data());

    m_Format = surfaceFormat.format;

    m_ImageViews.resize(m_Images.size());

    for (size_t i = 0; i < m_Images.size(); i++) 
        m_ImageViews[i] = Image::CreateImageView(m_Device, m_Images[i], m_Format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    m_SampleCount = VK_SAMPLE_COUNT_1_BIT;
    if(counts & VK_SAMPLE_COUNT_64_BIT) 
        m_SampleCount = VK_SAMPLE_COUNT_64_BIT;
    else if(counts & VK_SAMPLE_COUNT_32_BIT)
        m_SampleCount = VK_SAMPLE_COUNT_32_BIT;
    else if (counts & VK_SAMPLE_COUNT_16_BIT)
        m_SampleCount = VK_SAMPLE_COUNT_16_BIT;
    else if (counts & VK_SAMPLE_COUNT_8_BIT)
        m_SampleCount = VK_SAMPLE_COUNT_8_BIT;
    else if (counts & VK_SAMPLE_COUNT_4_BIT)
        m_SampleCount = VK_SAMPLE_COUNT_4_BIT;
    else if (counts & VK_SAMPLE_COUNT_2_BIT)
        m_SampleCount = VK_SAMPLE_COUNT_2_BIT;

    m_ColorBuffer = MakeScopedPtr<Image>(m_Device, m_Extent.width, m_Extent.height, m_Format, 
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, m_SampleCount);
    m_DepthBuffer = MakeScopedPtr<Image>(m_Device, m_Extent.width, m_Extent.height, Image::GetDepthFormat(m_Device),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, m_SampleCount);
    CreateRenderPass();
    CreateFramebuffers();
}

SwapChain::~SwapChain()
{
    for (auto& framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(m_Device->GetDeviceHandle(), framebuffer, nullptr);

    vkDestroyRenderPass(m_Device->GetDeviceHandle(), m_RenderPass, nullptr);

    for (auto& imageView : m_ImageViews) 
        vkDestroyImageView(m_Device->GetDeviceHandle(), imageView, nullptr);

    vkDestroySwapchainKHR(m_Device->GetDeviceHandle(), m_SwapChain, nullptr);
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

void SwapChain::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_Format;
    colorAttachment.samples = m_SampleCount;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_Format;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = Image::GetDepthFormat(m_Device);
    depthAttachment.samples = m_SampleCount;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    RAYD_VK_VALIDATE(vkCreateRenderPass(m_Device->GetDeviceHandle(), &renderPassInfo, nullptr, &m_RenderPass), "Failed to create render pass!");
}

void SwapChain::CreateFramebuffers()
{
    m_Framebuffers.resize(m_ImageViews.size());

    for (size_t i = 0; i < m_ImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            m_ColorBuffer->GetViewHandle(),
            m_DepthBuffer->GetViewHandle(),
            m_ImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_Extent.width;
        framebufferInfo.height = m_Extent.height;
        framebufferInfo.layers = 1;

        RAYD_VK_VALIDATE(vkCreateFramebuffer(m_Device->GetDeviceHandle(), &framebufferInfo, nullptr, &m_Framebuffers[i]), "Failed to create framebuffer!");
    }
}
