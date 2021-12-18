#pragma once

#include "GraphicsCore.h"
#include "Buffer.h"

class Sampler {
public:
	Sampler(RefPtr<Device> device);
	~Sampler();
	inline const VkSampler& GetHandle() { return m_Sampler; }
private:
	RefPtr<Device> m_Device;
	VkSampler m_Sampler;
};

class Image : public Buffer {
public:
	Image(RefPtr<Device> device, const std::string& filepath);
	Image(RefPtr<Device> device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageAspectFlags aspect);
	~Image();
	static VkImageView CreateImageView(RefPtr<Device> device, VkImage& img, VkFormat fmt, VkImageAspectFlags aspect);
	static VkFormat GetSupportedFormat(RefPtr<Device> device, const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features);
	static VkFormat GetDepthFormat(RefPtr<Device> device) {
		return GetSupportedFormat(device, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	inline const VkImage& GetImageHandle() { return m_Image; }
	inline const VkImageView& GetViewHandle() { return m_View; }
protected:
	void Create(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
	
	void CopyFromBuffer(VkBuffer buffer);
	void Transition(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
protected:
	VkImage m_Image;
	VkImageView m_View;
	uint32_t m_Width;
	uint32_t m_Height;
};