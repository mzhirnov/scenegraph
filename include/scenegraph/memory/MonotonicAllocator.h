#pragma once

#include <scenegraph/memory/BasicAllocator.h>
#include <scenegraph/utils/MemoryUtils.h>

#include <array>
#include <type_traits>

#define DEBUG_ALLOCATORS_ENABLED 0

#if DEBUG_ALLOCATORS_ENABLED
#include <iostream>
#endif

// Data members moved to separate struct in order to ease getting their total size
template <typename T>
class MonotonicPageHeader {
public:
	explicit MonotonicPageHeader(void* allocator) noexcept
		: _allocator(allocator)
	{
	}
	
protected:
	std::unique_ptr<T> nextPage;
	T* prevPage = static_cast<T*>(this);
	
	void* _allocator = nullptr;
	int _stackIndex = 0;
};

///
/// Monotonic page
///
template <std::size_t PageBytes>
class MonotonicPage : public MonotonicPageHeader<MonotonicPage<PageBytes>> {
public:
	static constexpr std::size_t kAlignMask = alignof(MonotonicPageHeader<MonotonicPage>) - 1;
	static constexpr std::size_t kMaxSize = (PageBytes - sizeof(MonotonicPageHeader<MonotonicPage>) + kAlignMask) & ~kAlignMask;
	static constexpr std::size_t kMaxAlign = 16384; // Just some big power of two
	
	static_assert(static_cast<int>(kMaxSize) > 0, "Not enought PageBytes to hold page header and data");
	
	// Public fields
	using MonotonicPageHeader<MonotonicPage>::nextPage;
	using MonotonicPageHeader<MonotonicPage>::prevPage;
	
	explicit MonotonicPage(void* allocator) noexcept
		: MonotonicPageHeader<MonotonicPage>(allocator)
	{
		static_assert(sizeof(MonotonicPage) == PageBytes);
	}
	
	~MonotonicPage() {
		assert(this->_stackIndex == 0 && "Destroying items storage with external pointers to it");
		
		// Destroy list inplace in the loop instead of auto recursion
		while (nextPage) {
			nextPage = std::move(nextPage->nextPage);
		}
	}
	
	[[nodiscard]]
	void* TryAllocate(std::size_t size, std::size_t align) noexcept {
		assert(align && !(align & (align - 1)) && "Align must be non zero power of two");
		
		const auto stackTop = GetStackPointer<uint32_t>(_bytes.data(), _bytes.size());
		const auto sp = stackTop - this->_stackIndex;
		const auto begin = _bytes.data();
		
		// Allocation starts here
		auto p = begin + (this->_stackIndex ? *sp : 0);
		
		// Skip aligned header
		p = AlignPointerUpwards(p, alignof(ItemHeader)) + sizeof(ItemHeader);
		
		// Align block (larger alignment would be ok because of power of two)
		p = AlignPointerUpwards(p, align);
		
		auto newWatermark = p + size - begin;
		
		// No room for allocating block
		if (newWatermark > reinterpret_cast<std::byte*>(sp - 1) - begin) {
			return nullptr;
		}
		
		auto header = std::construct_at(reinterpret_cast<ItemHeader*>(p) - 1);
		
#if DEBUG_ALLOCATORS_ENABLED
		std::cout << "+ WM " << (this->_stackIndex ? *sp : 0) << " -> " << newWatermark << '\n';
		std::cout << "+ SP " << this->_stackIndex << " -> " << this->_stackIndex + 1 << '\n';
#endif
		
		this->_stackIndex++;
		
		// Offset from allocated block to page
		header->offset = static_cast<uint32_t>(p - reinterpret_cast<std::byte*>(this));
		header->stackIndex = this->_stackIndex;
		
		// Push new watermark on the stack
		sp[-1] = static_cast<uint32_t>(newWatermark);
		
		return p;
	}
	
	void Deallocate(void* p) noexcept {
		const auto stackTop = GetStackPointer<uint32_t>(_bytes.data(), _bytes.size());
		const auto header = static_cast<ItemHeader*>(p) - 1;
		
#if DEBUG_ALLOCATORS_ENABLED
		std::size_t watermarkStart = stackTop[-header->stackIndex];
#endif
		stackTop[-header->stackIndex] = 0;
		
		if (header->stackIndex == this->_stackIndex) {
			auto sp = stackTop - this->_stackIndex;
			for (/**/; sp != stackTop && !*sp; ++sp) {
#if DEBUG_ALLOCATORS_ENABLED
				std::cout << "- SP " << this->_stackIndex << " -> " << this->_stackIndex - 1 << '\n';
#endif
				this->_stackIndex--;
			}
			
#if DEBUG_ALLOCATORS_ENABLED
			std::cout << "- WM " << watermarkStart << " -> " << (this->_stackIndex ? *sp : 0) << '\n';
#endif
			assert(sp != stackTop || this->_stackIndex == 0);
		}
	}
	
	[[nodiscard]]
	bool Empty() const noexcept { return this->_stackIndex == 0; }
	
	[[nodiscard]]
	bool Single() const noexcept { return this->prevPage == this; }
	
	[[nodiscard]]
	void* Allocator() const noexcept { return this->_allocator; }
	
	[[nodiscard]]
	static MonotonicPage* GetPage(void* p) noexcept {
		auto header = static_cast<ItemHeader*>(p) - 1;
		assert(header->signature == kSignature);
		auto page = reinterpret_cast<MonotonicPage*>(static_cast<std::byte*>(p) - header->offset);
		return page;
	}
	
private:
	static constexpr uint32_t kSignature = 0xaaaa5555;

	struct ItemHeader {
	#ifndef NDEBUG
		uint32_t signature = kSignature;
	#endif
		uint32_t offset;
		int stackIndex;
	};
	
	// Memory map
	// +------------------------------------+--------/ /--------+-----------+
	// | padding & header | allocated block | ... -> \ \ <- ... | watermark |
	// +------------------------------------+--------/ /--------+-----------+
	// ^           offset ^                 ^                   ^
	// | prev watermark           watermark |     stack pointer |
	
	std::array<std::byte, kMaxSize> _bytes;
};

template <std::size_t PageBytes>
using MonotonicAllocator = BasicAllocator<MonotonicPage<PageBytes>>;

template <std::size_t Bytes>
using StaticMonotonicAllocator = BasicStaticAllocator<MonotonicPage<Bytes>>;
