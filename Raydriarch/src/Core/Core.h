#pragma once

template<typename T>
using ScopedPtr = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr ScopedPtr<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using RefPtr = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr RefPtr<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}