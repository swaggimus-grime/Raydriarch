#include "raydpch.h"
#include "Graphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static SceneData* s_Data = new SceneData;
static GraphicsObjects* s_Objects = new GraphicsObjects;

struct Vertex {
	glm::vec2 Position;
	glm::vec3 Color;
	glm::vec2 TexCoord;
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

void Graphics::Init(ScopedPtr<Window>& window)
{
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	s_Objects->GPU = MakeRefPtr<Device>(window->GetGraphicsContext().GetInstance(), window->GetSurface().GetSurfaceHandle(), deviceFeatures);

	auto [width, height] = window->GetFramebufferSize();
	s_Objects->SC = MakeScopedPtr<SwapChain>(s_Objects->GPU, window->GetSurface().GetSurfaceHandle(), width, height);
	
	Command::Init(s_Objects->GPU, s_Objects->SC->GetNumFramebuffers());

	VertexLayout layout;
	layout.AddAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT);
	layout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT);
	layout.AddAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT);
	layout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

	s_Data->VBuffer = MakeRefPtr<VertexBuffer>(s_Objects->GPU, vertices.size(), vertices.size() * sizeof(Vertex), vertices.data());
	s_Data->IBuffer = MakeRefPtr<IndexBuffer>(s_Objects->GPU, indices.size(), indices.size() * sizeof(uint16_t), indices.data());

	RefPtr<Shader> shader = MakeRefPtr<Shader>(s_Objects->GPU->GetDeviceHandle(), "res/shaders/vert.spv", "res/shaders/frag.spv");
	s_Data->Pipeline = MakeScopedPtr<GraphicsPipeline>(window, s_Objects->GPU, s_Objects->SC, shader, layout);

	Command::DrawIndexed(s_Data, s_Objects);
}

void Graphics::Present(ScopedPtr<class Window>& window, float deltaTime)
{
	if (s_Data->Pipeline->Present(deltaTime) == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapChain(window);
		Command::DrawIndexed(s_Data, s_Objects);
	}
}

void Graphics::RecreateSwapChain(ScopedPtr<class Window>& window)
{
	s_Objects->GPU->Join();

	auto [width, height] = window->GetFramebufferSize();
	while (width == 0 || height == 0) {
		auto [width, height] = window->GetFramebufferSize();
		glfwWaitEvents();
	}
	s_Objects->SC.reset();
	s_Objects->SC = MakeScopedPtr<SwapChain>(s_Objects->GPU, window->GetSurface().GetSurfaceHandle(), width, height);
	Command::Shutdown();
	Command::Init(s_Objects->GPU, s_Objects->SC->GetNumFramebuffers());

	VertexLayout layout;
	layout.AddAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT);
	layout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT);
	layout.AddAttribute(2, 0, VK_FORMAT_R32G32_SFLOAT);
	layout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

	s_Data->VBuffer = MakeRefPtr<VertexBuffer>(s_Objects->GPU, vertices.size(), vertices.size() * sizeof(Vertex), vertices.data());
	s_Data->IBuffer = MakeRefPtr<IndexBuffer>(s_Objects->GPU, indices.size(), indices.size() * sizeof(uint16_t), indices.data());

	RefPtr<Shader> shader = MakeScopedPtr<Shader>(s_Objects->GPU->GetDeviceHandle(), "res/shaders/vert.spv", "res/shaders/frag.spv");
	s_Data->Pipeline.reset();
	s_Data->Pipeline = MakeScopedPtr<GraphicsPipeline>(window, s_Objects->GPU, s_Objects->SC, shader, layout);
}

void Graphics::Shutdown()
{
	Command::Shutdown();
	s_Data->Pipeline->Shutdown();
	delete s_Data;
	delete s_Objects;
}
