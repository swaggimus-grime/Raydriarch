#pragma once

#include <Vulkan/vulkan.h>
#define RAYD_VK_VALIDATE(result, errMsg) RAYD_ASSERT(result == VK_SUCCESS, errMsg)