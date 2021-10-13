#include "raydpch.h"
#include "Graphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static SceneData* s_Data = new SceneData;
static GraphicsObjects* s_Objects = new GraphicsObjects;

struct Vertex {
	glm::vec2 Position;
	glm::vec3 Color;
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

void Graphics::Init(ScopedPtr<Window>& window)
{
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = 1;

	RefPtr<Device> GPU = MakeRefPtr<Device>(window->GetGraphicsContext().GetInstance(), window->GetSurface().GetSurfaceHandle(), deviceFeatures);

	auto [width, height] = window->GetFramebufferSize();
	s_Objects->SC = MakeScopedPtr<SwapChain>(GPU, window->GetSurface().GetSurfaceHandle(), width, height);

	Command::Init(GPU, s_Objects->SC->GetNumFramebuffers());

	VertexLayout layout;
	layout.AddAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT);
	layout.AddAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT);
	layout.AddBinding(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

	s_Data->VBuffer = MakeRefPtr<VertexBuffer>(GPU, vertices.size(), vertices.size() * sizeof(Vertex), vertices.data());
	s_Data->IBuffer = MakeRefPtr<IndexBuffer>(GPU, indices.size(), indices.size() * sizeof(uint16_t), indices.data());

	ScopedPtr<Shader> shader = MakeScopedPtr<Shader>(GPU->GetDeviceHandle(), "res/shaders/vert.spv", "res/shaders/frag.spv");
	s_Data->Pipeline = MakeScopedPtr<GraphicsPipeline>(window, GPU, s_Objects->SC, shader, layout);

	Command::DrawIndexed(s_Data, s_Objects);
}

void Graphics::Present(float deltaTime)
{
	s_Data->Pipeline->Present(deltaTime);
}

void Graphics::Shutdown()
{
	Command::Shutdown();
	s_Data->Pipeline->Shutdown();
	delete s_Data;
	delete s_Objects;
}
