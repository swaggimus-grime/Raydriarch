#pragma once

#include "Core.h"
#include "GLFW/glfw3.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Surface.h"

struct WindowProps {
	std::string Title;
	uint32_t Width = 1280;
	uint32_t Height = 720;
};

class Window {
public:
	Window(const WindowProps& props);
	~Window();
	Window(const Window&) = delete;

	inline uint32_t GetWidth() const { return m_Props.Width; }
	inline uint32_t GetHeight() const { return m_Props.Height; }

	std::pair<uint32_t, uint32_t> GetFramebufferSize() const;
	inline uint32_t GetFramebufferWidth() const { return GetFramebufferSize().first; }
	inline uint32_t GetFramebufferHeight() const { return GetFramebufferSize().second; }

	void Update();
	inline int IsClosed() const { return glfwWindowShouldClose(m_Window); }

	inline GLFWwindow* GetHandle() const { return m_Window; }
	inline GraphicsContext& GetGraphicsContext() const { return *m_Context; }

	ScopedPtr<Surface>& GetSurface();

private:
	static std::vector<const char*>& GetRequiredVulkanExtensions();
private:
	GLFWwindow* m_Window;
	WindowProps m_Props;

	ScopedPtr<GraphicsContext> m_Context;
	ScopedPtr<Surface> m_Surface;

	friend class Graphics;
	bool m_Resized = false;
};