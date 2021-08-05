#pragma once

#include <vulkan/vulkan.h>

#include "GraphicsCore.h"

class GraphicsContext {
public:
	GraphicsContext(uint32_t requiredExtensionCount, const char** requiredExtensionNames);
	~GraphicsContext();

	inline VkInstance& GetInstance() { return m_Instance; }
private:
	void VerifyValidationLayers();
	void CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo);
private:
	VkInstance m_Instance;

	VkDebugUtilsMessengerEXT m_DebugMessenger;
	const std::vector<const char*> m_ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
};