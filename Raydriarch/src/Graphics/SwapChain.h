#pragma once

#include "GraphicsCore.h"

#include "Device.h"
#include "RenderPass.h"
#include "Image.h"

class SwapChain {
public:
	SwapChain(RefPtr<Device> device, ScopedPtr<class Surface>& surface, 
		uint32_t framebufferWidth, uint32_t framebufferHeight);
	~SwapChain();

	inline const VkSwapchainKHR& GetSwapChainHandle() const { return m_SwapChain; }
	inline const VkRenderPass& GetRenderPass() const { return m_RenderPass; }

	inline VkExtent2D GetExtent() const { return m_Extent; }
	inline VkFormat GetFormat() const { return m_Format; }
	inline VkSampleCountFlagBits GetSampleCount() const { return m_SampleCount; }

	inline const std::vector<VkImage>& GetImages() const { return m_Images; }
	inline const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
	inline const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_Framebuffers; }
private:
	VkSurfaceFormatKHR FindSurfaceFormat(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkPresentModeKHR FindPresentMode(const VkPhysicalDevice& physicalDevice, const SwapChainSupportDetails& details, VkSurfaceKHR& surface);
	VkExtent2D FindSwapExtent(const SwapChainSupportDetails& details, uint32_t framebufferWidth, uint32_t framebufferHeight);
	void CreateRenderPass();
	void CreateFramebuffers();
private:
	RefPtr<Device> m_Device;

	VkSwapchainKHR m_SwapChain;
	VkFormat m_Format;
	VkExtent2D m_Extent;
	VkSampleCountFlagBits m_SampleCount;

	VkRenderPass m_RenderPass;

	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
	std::vector<VkFramebuffer> m_Framebuffers;
	ScopedPtr<Image> m_ColorBuffer;
	ScopedPtr<Image> m_DepthBuffer;
};