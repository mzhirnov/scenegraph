#pragma once

#include <array>
#include <memory>
#include <cstddef>

template <typename T, size_t Size, size_t Align = alignof(void*)>
class StaticImpl final {
public:
	static_assert(Size > 0);
	static_assert(Align > 0);
	
	template <typename... Args>
	constexpr StaticImpl(Args&&... args) noexcept(noexcept(T{std::forward<Args>(args)...})) {
		static_assert(sizeof(T) == Size);
		static_assert(alignof(T) == Align);
		std::construct_at(reinterpret_cast<T*>(_storage.data()), std::forward<Args>(args)...);
	}
	
	constexpr ~StaticImpl() {
		std::destroy_at(reinterpret_cast<T*>(_storage.data()));
	}
	
	constexpr T* operator->() noexcept { return reinterpret_cast<T*>(_storage.data()); }
	constexpr const T* operator->() const noexcept { return reinterpret_cast<const T*>(_storage.data()); }
	
	constexpr T& operator*() noexcept { return *reinterpret_cast<T*>(_storage.data()); }
	constexpr const T& operator*() const noexcept { return *reinterpret_cast<const T*>(_storage.data()); }

private:
	alignas(Align) std::array<std::byte, Size> _storage;
};
