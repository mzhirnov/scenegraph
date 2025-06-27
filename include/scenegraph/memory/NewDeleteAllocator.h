#pragma once

#include <new>

///
/// New delete allocator
///
class NewDeleteAllocator {
public:
	NewDeleteAllocator() = default;
	
	[[nodiscard]]
	constexpr void* Allocate(size_t count, size_t align) noexcept {
		return ::operator new[] (count, static_cast<std::align_val_t>(align), std::nothrow);
	}
	
	constexpr void Deallocate(void* p, size_t count) noexcept {
		::operator delete[] (p, count);
	}
};

