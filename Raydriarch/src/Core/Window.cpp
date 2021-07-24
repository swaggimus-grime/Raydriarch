#include "raydpch.h"
#include "Window.h"

static bool s_GLFWInitialized = false;

Window::Window(const WindowProps& props)
	:m_Props(props), m_Window(nullptr)
{
	if (!s_GLFWInitialized)
	{
		int success = glfwInit();
		RAYD_ASSERT(success, "Could not intialize GLFW!");
		glfwSetErrorCallback([](int error, const char* desc) {
			RAYD_ERROR("GLFW ERROR ({0}): {1}", error, desc);
			});
		s_GLFWInitialized = true;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(props.Width, props.Height, m_Props.Title.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, &m_Props);

	m_Graphics = MakeScopedPtr<Graphics>(m_Window);

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
			props.Width = width;
			props.Height = height;
		});
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

std::pair<uint32_t, uint32_t> Window::GetFramebufferSize() const
{
	int width, height;
	glfwGetFramebufferSize(m_Window, &width, &height);

	return { width, height };
}

void Window::Update()
{
	glfwPollEvents();
}

std::vector<const char*>& Window::GetRequiredVulkanExtensions()
{
	uint32_t glfwExtensionCount;
	const char** glfwExtensionNames;
	glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	static std::vector<const char*> extensions(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);

#ifdef RAYD_DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return extensions;
}
