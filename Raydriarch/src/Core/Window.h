#pragma once

#include "Core.h"
#include "GLFW/glfw3.h"

#include "Graphics/Graphics.h"

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

	void Update();
	inline int IsClosed() const { return glfwWindowShouldClose(m_Window); }

	inline GLFWwindow* GetHandle() const { return m_Window; }

	static std::vector<const char*>& GetRequiredVulkanExtensions();
private:
	GLFWwindow* m_Window;
	WindowProps m_Props;

	ScopedPtr<class Graphics> m_Graphics;
};