#pragma once

#include "Core/Window.h"

#include "Device.h"
#include "SwapChain.h"
#include "Surface.h"
#include "GraphicsPipeline.h"
#include "Command.h"
#include "Buffer.h"

struct SceneData {
	const char* VertShaderPath;
	const char* FragShaderPath;
	RefPtr < class GraphicsPipeline > Pipeline;
	RefPtr<VertexBuffer > VBuffer;
	RefPtr<IndexBuffer> IBuffer;
};

struct GraphicsObjects {
	RefPtr<Device> GPU;
	ScopedPtr<SwapChain> SC;
};

class Graphics {
public:
	static void Init(ScopedPtr<class Window>& window);
	static void Present(ScopedPtr<class Window>& window, float deltaTime);
	static void Shutdown();

private:
	static void RecreateSwapChain(ScopedPtr<class Window>& window);
};