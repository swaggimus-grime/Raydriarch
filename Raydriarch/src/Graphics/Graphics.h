#pragma once

#include "Core/Window.h"

#include "GraphicsContext.h"
#include "Device.h"

class Graphics {
public:
	Graphics(const GLFWwindow* window);
	~Graphics();

private:
	ScopedPtr<GraphicsContext> m_Context;
	ScopedPtr<Device> m_Device;
};