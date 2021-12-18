#pragma once

#include "Core/Window.h"

#include "Device.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "Command.h"
#include "Buffer.h"
#include "Image.h"

struct SceneData {
	RefPtr<class GraphicsPipeline> Pipeline;
	VertexLayout VLayout;
	RefPtr<VertexBuffer > VBuffer;
	RefPtr<IndexBuffer> IBuffer;
	RefPtr<Image> Texture;
	RefPtr<Sampler> Sampler;
	std::vector<ScopedPtr<UniformBuffer>> UBuffers;
	VkDescriptorSetLayout DescSetLayout;
	RefPtr<class DescriptorPool> DescPool;
	std::vector<VkDescriptorSet> DescSets;
};

struct GraphicsObjects {
	RefPtr<Device> GPU;
	ScopedPtr<SwapChain> SC;
	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CBuffers;
	std::vector<VkSemaphore> ImageAvailSemaphores;
	std::vector<VkSemaphore> RenderFinishSemaphores;
	std::vector<VkFence> InFlightFences;
	std::vector<VkFence> ImagesInFlightFenches;
};

class Graphics {
public:
	static void Init(ScopedPtr<class Window>& window);
	static void Present(ScopedPtr<class Window>& window, float deltaTime);
	static void Shutdown();

private:
	static void RecreateSwapChain(ScopedPtr<class Window>& window);
	static void CleanupSwapChain();
};