#pragma once

#include "GraphicsCore.h"

class Surface {
public:
	Surface(VkInstance& instance, VkSurfaceKHR& surfaceHandle/*, uint32_t width, uint32_t height*/)
		:m_Instance(instance), m_Surface(surfaceHandle)/*, m_Width(width), m_Height(height)*/
	{
	}

	~Surface() {
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	}

	inline VkSurfaceKHR& GetSurfaceHandle() { return m_Surface; }

	//inline uint32_t GetWidth() const { return m_Width; }
	//inline uint32_t GetHeight() const { return m_Height; }
private:
	VkSurfaceKHR m_Surface;
	//uint32_t m_Width, m_Height;

	VkInstance& m_Instance;
};