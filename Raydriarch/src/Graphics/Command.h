#pragma once

#include "GraphicsCore.h"
#include "Graphics.h"
#include "Device.h"

class Command {
public:
	Command() = delete;
	static void Init(RefPtr<Device> device, VkCommandPool& cmdPool);
	static void Shutdown();

	static VkCommandBuffer BeginSingleTimeCommands();
	static void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	
};