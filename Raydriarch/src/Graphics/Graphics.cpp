#include "raydpch.h"
#include "Graphics.h"

#include "Surface.h"

struct GraphicsObjects {
	ScopedPtr<Device> GPU;
	ScopedPtr<SwapChain> SwapChain;

} *s_Objects = new GraphicsObjects;

void Graphics::Init(Window* window)
{
	auto& instance = window->GetGraphicsContext().GetInstance();

	Surface& surface = window->GetSurface();

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;
	s_Objects->GPU = MakeScopedPtr<Device>(instance, surface.GetSurfaceHandle(), deviceFeatures);

	//auto [width, height] = window->GetFramebufferSize();
	//s_Objects->SwapChain = MakeScopedPtr<SwapChain>(s_Objects->GPU.get(), surface.GetSurfaceHandle(), width, height);
}

void Graphics::Shutdown()
{
	delete s_Objects;
}
