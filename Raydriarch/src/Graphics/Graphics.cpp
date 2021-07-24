#include "raydpch.h"
#include "Graphics.h"

Graphics::Graphics(const GLFWwindow* window)
{
	auto& glfwExtensions = Window::GetRequiredVulkanExtensions();
	m_Context = MakeScopedPtr<GraphicsContext>(glfwExtensions.size(), glfwExtensions.data());

	auto& instance = m_Context->GetInstance();

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;
	m_Device = MakeScopedPtr<Device>(instance, deviceFeatures);
}

Graphics::~Graphics()
{
}
