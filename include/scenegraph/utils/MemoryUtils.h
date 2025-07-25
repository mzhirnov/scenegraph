#pragma once

#include <type_traits>
#include <cstddef>

inline std::byte* AlignPointerDownwards(std::byte* p, std::size_t align) noexcept {
	const auto mask = align - 1;
	return reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(p) & ~mask);
};

inline std::byte* AlignPointerUpwards(std::byte* p, std::size_t align) noexcept {
	const auto mask = align - 1;
	return reinterpret_cast<std::byte*>((reinterpret_cast<uintptr_t>(p) + mask) & ~mask);
};

// Returns aligned downwards pointer to the end of data
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T* GetStackPointer(std::byte* data, std::size_t size) noexcept {
	return reinterpret_cast<T*>(AlignPointerDownwards(data + size, alignof(T)));
}
