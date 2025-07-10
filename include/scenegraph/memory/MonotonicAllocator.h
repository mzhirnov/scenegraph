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
	
	static constexpr std::size_t MaxSize = kDataSize;
	static constexpr std::size_t MaxAlign = 16384; // Just some big power of two
	
	// Public fields
	using MonotonicPageHeader<MonotonicPage>::nextPage;
	using MonotonicPageHeader<MonotonicPage>::prevPage;
	
	explicit MonotonicPage(void* allocator) noexcept
		: MonotonicPageHeader<MonotonicPage>(allocator)
	{
		static_assert(sizeof(MonotonicPage) == PageBytes);
	}
	
	~MonotonicPage() {
		// Destroy list inplace in the loop instead of auto recursion
		while (nextPage) {
			nextPage = std::move(nextPage->nextPage);
		}
	}
	
	[[nodiscard]]
	void* TryAllocate(std::size_t size, std::size_t align) noexcept {
		assert(align && !(align & (align - 1)) && "Align must be non zero power of two");
		
		// Allocated block begins here
		const auto baseOffset = this->_currentOffset;
		
		// Aligned header
		const auto headerAlignMask = alignof(ItemHeader) - 1;
		const auto headerOffset = (this->_currentOffset + headerAlignMask) & ~headerAlignMask;
		this->_currentOffset = static_cast<uint32_t>(headerOffset + sizeof(ItemHeader));
		
		// Aligned block
		const auto blockAlignMask = align - 1;
		const auto blockOffset = (this->_currentOffset + blockAlignMask) & ~blockAlignMask;
		this->_currentOffset = static_cast<uint32_t>(blockOffset + size);
		
		// No free space
		if (this->_currentOffset > _bytes.size()) {
			this->_currentOffset = baseOffset;
			return nullptr;
		}
		
		auto p = _bytes.data() + blockOffset;
		auto header = std::construct_at(reinterpret_cast<ItemHeader*>(p) - 1);
		
		// Offset from allocated block to page start
		header->offset = static_cast<decltype(ItemHeader::offset)>(p - reinterpret_cast<std::byte*>(this));
		header->size = static_cast<decltype(ItemHeader::size)>(size);
		
		this->_allocatedSize += static_cast<int>(size);
		
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
