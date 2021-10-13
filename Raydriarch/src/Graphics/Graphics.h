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
	RefPtr<SwapChain> SC;
};

class Graphics {
public:
	static void Init(ScopedPtr<class Window>& window);
	static void Present(float deltaTime);
	static void Shutdown();

};