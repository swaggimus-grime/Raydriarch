#pragma once

#include "Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Log
{
public:
	static void Init();

	inline static RefPtr<spdlog::logger>& GetLogger() { return m_Logger; }
private:
	static RefPtr<spdlog::logger> m_Logger;
};


// Client log macros
#define RAYD_TRACE(...)	   ::Log::GetLogger()->trace(__VA_ARGS__)
#define RAYD_INFO(...)	   ::Log::GetLogger()->info(__VA_ARGS__)
#define RAYD_WARN(...)	   ::Log::GetLogger()->warn(__VA_ARGS__)
#define RAYD_ERROR(...)	   ::Log::GetLogger()->error(__VA_ARGS__)