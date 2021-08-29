#pragma once

#include "Core/Window.h"

#include "Device.h"
#include "SwapChain.h"

class Graphics {
public:
	static void Init(ScopedPtr<class Window>& window);
	static void Present();
	static void Shutdown();
private:
	
};