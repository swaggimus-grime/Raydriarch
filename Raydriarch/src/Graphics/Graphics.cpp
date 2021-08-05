#include "raydpch.h"
#include "Graphics.h"

#include "Surface.h"
#include "GraphicsPipeline.h"

static struct GraphicsObjects {
	ScopedPtr<GraphicsPipeline> Pipeline;
}* s_Objects = new GraphicsObjects;

void Graphics::Init(Window* window)
{
	s_Objects->Pipeline = MakeScopedPtr<GraphicsPipeline>(window);
}

void Graphics::Present()
{

}

void Graphics::Shutdown()
{
	s_Objects->Pipeline->Shutdown();
	delete s_Objects;
}
