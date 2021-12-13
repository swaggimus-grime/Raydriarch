#pragma once

#include "GraphicsCore.h"
#include "Buffer.h"

class ImageView {
public:
	ImageView(RefPtr<Device> device, VkImage& image, VkFormat format);
	~ImageView();

	inline const VkImageView& GetHandle() { return m_View; }
private:
	RefPtr<Device> m_Device;
	VkImageView m_View;
};

class Sampler2D {
public:
	Sampler2D(RefPtr<Device> device);
	~Sampler2D();
	inline const VkSampler& GetHandle() { return m_Sampler; }
private:
	RefPtr<Device> m_Device;
	VkSampler m_Sampler;
};

class Texture2D : public Buffer {
public:
	Texture2D(RefPtr<Device> device, const std::string& filepath);
	~Texture2D();
	void CopyFromBuffer(VkBuffer buffer);
	void Transition(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	inline const VkImage& GetImageHandle() { return m_Image; }
	inline const VkImageView& GetViewHandle() { return m_View->GetHandle(); }
private:
	VkImage m_Image;
	ScopedPtr<ImageView> m_View;
	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_Channels;
};