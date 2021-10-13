#pragma once

#include "GraphicsCore.h"
#include "Graphics.h"
#include "Device.h"

class Command {
public:
	Command() = delete;
	static void Init(RefPtr<Device> device, uint8_t numBuffers);
	static void CopyBuffer(VkDeviceSize size, VkBuffer& srcBuffer, VkBuffer& dstBuffer);
	static void DrawIndexed(struct SceneData* sceneData, struct GraphicsObjects* graphicsObjects);
	static void Shutdown();
	static VkCommandBuffer* GetCommandBuffers();
};