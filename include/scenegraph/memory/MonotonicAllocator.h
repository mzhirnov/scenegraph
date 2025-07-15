#pragma once

#include <scenegraph/memory/BasicAllocator.h>

#include <array>

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
	int _allocatedSize = 0;
	uint32_t _currentOffset = 0;
};

///
/// Monotonic page
///
template <std::size_t PageBytes>
class MonotonicPage : public MonotonicPageHeader<MonotonicPage<PageBytes>> {
public:
	enum {
		kMaxAlignFillingBits = alignof(std::max_align_t) - 1,
		kDataSizeUnaligned = PageBytes - sizeof(MonotonicPageHeader<MonotonicPage>),
		kDataSize = (kDataSizeUnaligned + kMaxAlignFillingBits) & ~kMaxAlignFillingBits
	};
	
	static constexpr std::size_t kMaxSize = kDataSize;
	static constexpr std::size_t kMaxAlign = 16384; // Just some big power of two
	
	// Public fields
	using MonotonicPageHeader<MonotonicPage>::nextPage;
	using MonotonicPageHeader<MonotonicPage>::prevPage;
	
	explicit MonotonicPage(void* allocator) noexcept
		: MonotonicPageHeader<MonotonicPage>(allocator)
	{
		static_assert(sizeof(MonotonicPage) == PageBytes);
	}
	
	~MonotonicPage() {
		assert(this->_allocatedSize == 0 && "Destroying items storage with external pointers to it");
		
		// Destroy list inplace in the loop instead of auto recursion
		while (nextPage) {
			nextPage = std::move(nextPage->nextPage);
		}
	}
	
	[[nodiscard]]
	void* TryAllocate(std::size_t size, std::size_t align) noexcept {
		assert(align && !(align & (align - 1)) && "Align must be non zero power of two");
		
		auto fnAlignPtr = [](std::byte* p, std::size_t align) {
			const auto mask = align - 1;
			return reinterpret_cast<std::byte*>((reinterpret_cast<uintptr_t>(p) + mask) & ~mask);
		};
		
		// Allocated block starts here
		auto p = _bytes.data() + this->_currentOffset;
		
		// Skip aligned header
		p = fnAlignPtr(p, alignof(ItemHeader)) + sizeof(ItemHeader);
		
		// Align block (larger alignment would be ok because of power of two)
		p = fnAlignPtr(p, align);
		
		auto newOffset = p + size - _bytes.data();
		
		// No free space
		if (static_cast<size_t>(newOffset) > _bytes.size()) {
			return nullptr;
		}
		
		auto header = std::construct_at(reinterpret_cast<ItemHeader*>(p) - 1);
		
		// Offset from allocated block to page start
		header->offset = static_cast<uint32_t>(p - reinterpret_cast<std::byte*>(this));
		header->size = static_cast<uint32_t>(size);
		
		this->_allocatedSize += static_cast<int>(size);
		this->_currentOffset = static_cast<uint32_t>(newOffset);
		
		return p;
	}
	
	void Deallocate(void* p) noexcept {
		auto header = static_cast<ItemHeader*>(p) - 1;
		
		this->_allocatedSize -= static_cast<int>(header->size);
		
		assert(this->_allocatedSize >= 0);
		
		if (this->_allocatedSize == 0) {
			this->_currentOffset = 0;
		}
	}
	
	[[nodiscard]]
	bool Empty() const noexcept { return this->_allocatedSize == 0; }
	
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
	
	[[nodiscard]]
	static std::size_t GetSize(const void* p) noexcept {
		auto header = static_cast<const ItemHeader*>(p) - 1;
		assert(header->signature == kSignature);
		return header->size;
	}
	
private:
	static constexpr uint32_t kSignature = 0xaaaa5555;

	struct ItemHeader {
	#ifndef NDEBUG
		uint32_t signature = kSignature;
	#endif
		uint32_t offset;
		uint32_t size;
	};
	
	static_assert(static_cast<int>(kDataSize) > 0);
	
	std::array<std::byte, kDataSize> _bytes;
};

template <std::size_t PageBytes>
using MonotonicAllocator = BasicAllocator<MonotonicPage<PageBytes>>;
