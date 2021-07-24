#include "raydpch.h"
#include "Log.h"

RefPtr<spdlog::logger> Log::m_Logger = nullptr;

void Log::Init() {
	spdlog::set_pattern("%^[%T] %n: %v%$");
	m_Logger = spdlog::stdout_color_mt("RAYDRIARCH");
	m_Logger->set_level(spdlog::level::trace);
}