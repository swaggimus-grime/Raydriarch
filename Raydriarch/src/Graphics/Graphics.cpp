#include "raydpch.h"
#include "Graphics.h"

#include "Surface.h"
#include "GraphicsPipeline.h"

static struct GraphicsObjects {
	ScopedPtr<Device> GPU;
	ScopedPtr<GraphicsPipeline> Pipeline;
}* s_Objects = new GraphicsObjects;

void Graphics::Init(Window* window)
{
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;

	s_Objects->GPU = MakeScopedPtr<Device>(window->GetGraphicsContext().GetInstance(), window->GetSurface().GetSurfaceHandle(), deviceFeatures);
	s_Objects->Pipeline = MakeScopedPtr<GraphicsPipeline>(window, s_Objects->GPU.get());
}

void Graphics::Present()
{
	s_Objects->Pipeline->Present();
}

void Graphics::Shutdown()
{
	s_Objects->Pipeline->Shutdown();
	delete s_Objects;
}
