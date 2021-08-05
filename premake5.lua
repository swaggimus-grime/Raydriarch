workspace "Raydriarch"
	architecture "x86_64"
	startproject "Raydriarch"

	configurations
	{
		"Debug",
		"Release"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Rendering/Display includes
IncludeDir = {}
IncludeDir["GLFW"] = "Raydriarch/vendor/GLFW/include"
IncludeDir["Vulkan"] = "Raydriarch/vendor/Vulkan/include"
IncludeDir["spdlog"] = "Raydriarch/vendor/spdlog/include"
IncludeDir["libshaderc"] = "Raydriarch/vendor/Shaderc/libshaderc/include"

group "Dependencies"
	include "Raydriarch/vendor/GLFW"
group ""

project "Raydriarch"
	location "Raydriarch"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "raydpch.h"
	pchsource "Raydriarch/src/raydpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"GLFW_INCLUDE_VULKAN"
	}

	libdirs { "Raydriarch/vendor/Vulkan/lib" }

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.libshaderc}"
	}

	links 
	{ 
		"GLFW",
		"vulkan-1.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

	filter "configurations:Debug"
		defines "RAYD_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RAYD_RELEASE"
		runtime "Release"
		optimize "on"