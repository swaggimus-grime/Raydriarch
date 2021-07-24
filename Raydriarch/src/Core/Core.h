#pragma once

/* Windows x64/x86 */
#ifdef _WIN32
	/* Windows x64  */
	#ifdef _WIN64
		#define RAYD_PLATFORM_WINDOWS
	#else
		/* Windows x86 */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define RAYD_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define RAYD_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
	 /* We also have to check __ANDROID__ before __linux__
	  * since android is based on the linux kernel
	  * it has __linux__ defined */
	#elif defined(__ANDROID__)
		#define RAYD_PLATFORM_ANDROID
		#error "Android is not supported!"
	#elif defined(__linux__)
		#define RAYD_PLATFORM_LINUX
		#error "Linux is not supported!"
	#else
		/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif

#ifdef RAYD_DEBUG
	#if defined(RAYD_PLATFORM_WINDOWS)
		#define RAYD_DEBUGBREAK() __debugbreak()
	#elif defined(RAYD_PLATFORM_LINUX)
		#include <signal.h>
		#define RAYD_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif

	#define RAYD_ENABLE_ASSERTS
#else
	#define RAYD_DEBUGBREAK()
#endif

#ifdef RAYD_ENABLE_ASSERTS
	#define RAYD_ASSERT(x, ...) { if(!(x)) { RAYD_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define RAYD_ASSERT(x, ...)
#endif

template<typename T>
using ScopedPtr = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr ScopedPtr<T> MakeScopedPtr(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using RefPtr = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr RefPtr<T> MakeRefPtr(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

