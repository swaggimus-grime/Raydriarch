#pragma once

#include "GraphicsCore.h"

class Shader {
public:
	Shader(const VkDevice& device, const std::string& vertPath, const std::string& fragPath);
	~Shader();

	inline const VkShaderModule& GetVertexShaderModule() const { return m_VertModule; }
	inline const VkShaderModule& GetFragmentShaderModule() const { return m_FragModule; }
private:
	std::optional<std::string> ReadFile(const std::string& filePath);
	VkShaderModule CreateModule(VkDevice& device, const std::string& filePath);

private:
	VkShaderModule m_VertModule;
	VkShaderModule m_FragModule;
};