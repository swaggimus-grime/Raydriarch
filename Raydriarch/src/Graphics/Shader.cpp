#include "raydpch.h"
#include "Shader.h"

#include <shaderc/shaderc.hpp>

static VkDevice logicalDevice;

Shader::Shader(const VkDevice& device, const std::string& vertPath, const std::string& fragPath)
{
	logicalDevice = device;

	m_VertModule = CreateModule(logicalDevice, vertPath);
	m_FragModule = CreateModule(logicalDevice, fragPath);
}

Shader::~Shader()
{
	vkDestroyShaderModule(logicalDevice, m_VertModule, nullptr);
	vkDestroyShaderModule(logicalDevice, m_FragModule, nullptr);
}

std::optional<std::string> Shader::ReadFile(const std::string& filePath)
{
	std::ifstream input(filePath, std::ios::in | std::ios::binary);
	if(input)
		return std::string((std::istreambuf_iterator<char>(input)),
			std::istreambuf_iterator<char>());

	return {};
}

VkShaderModule Shader::CreateModule(VkDevice& device, const std::string& filePath)
{
	auto source = ReadFile(filePath);
	RAYD_ASSERT(source, "Failed to find shader file {0}", filePath.c_str());

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = source->size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(source->c_str());

	VkShaderModule shaderModule;
	RAYD_VK_VALIDATE(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

	return shaderModule;
}
