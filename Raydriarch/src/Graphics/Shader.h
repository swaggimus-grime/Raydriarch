#pragma once

#include "GraphicsCore.h"

class Shader {
public:
	Shader(RefPtr<class Device> device, const std::string& vertPath, const std::string& fragPath);
	~Shader();

	inline const VkShaderModule& GetVertexShaderModule() const { return m_VertModule; }
	inline const VkShaderModule& GetFragmentShaderModule() const { return m_FragModule; }
private:
	std::optional<std::string> ReadFile(const std::string& filePath);
	VkShaderModule CreateModule(const std::string& filePath);

private:
	RefPtr<Device> m_Device;
	VkShaderModule m_VertModule;
	VkShaderModule m_FragModule;
};