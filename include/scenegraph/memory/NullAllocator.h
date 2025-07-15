#pragma once

#include <cassert>

///
/// Null allocator
///
class NullAllocator {
public:
	NullAllocator() = default;
	
	explicit NullAllocator(bool assert) : _assert(assert) {}
	
	[[nodiscard]]
	constexpr void* Allocate(std::size_t, std::size_t) noexcept {
		assert(!_assert && "Should not be called");
		return nullptr;
	}
	
	constexpr void Deallocate(void*) noexcept {
		assert(!_assert && "Should not be called");
	}
	
private:
	bool _assert = false;
};
