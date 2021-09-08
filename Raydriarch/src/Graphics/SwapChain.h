#pragma once

#include "GraphicsCore.h"

#include "Surface.h"
#include "Device.h"
#include "RenderPass.h"

class SwapChain {
public:
	SwapChain(RefPtr<Device> device, VkSurfaceKHR& surface, 
		uint32_t framebufferWidth, uint32_t framebufferHeight);
	~SwapChain();

	inline size_t GetNumFramebuffers() const { return m_Framebuffers.size(); }
	inline const VkSwapchainKHR& GetSwapChainHandle() const { return m_SwapChain; }
	inline const RenderPass& GetRenderPass() const { return *m_RenderPass; }

	inline VkExtent2D GetExtent() { return m_Extent; }
	inline VkFormat GetFormat() { return m_Format; }

	inline const std::vector<VkImage>& GetImages() const { return m_Images; }
	inline const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
	inline const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }
private:
	VkSurfaceFormatKHR FindSurfaceFormat(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkPresentModeKHR FindPresentMode(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkExtent2D FindSwapExtent(const SwapChainSupportDetails& details, uint32_t framebufferWidth, uint32_t framebufferHeight);

private:
	RefPtr<Device> m_Device;

	VkSwapchainKHR m_SwapChain;
	VkFormat m_Format;
	VkExtent2D m_Extent;

	ScopedPtr<RenderPass> m_RenderPass;

	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
	std::vector<VkFramebuffer> m_Framebuffers;
};