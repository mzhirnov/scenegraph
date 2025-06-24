#pragma once

#include <array>
#include <cstddef>

///
/// Monotonic paged allocator
///
template <size_t PageSize>
class MonotonicAllocator {
public:
	constexpr MonotonicAllocator() = default;
	
	struct ItemHeader {
		uint32_t size;
		uint32_t offset;
	};
	
	template <typename T>
	[[nodiscard]]
	constexpr T* Allocate() noexcept {
		return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
	}
	
	[[nodiscard]]
	constexpr void* Allocate(size_t size, size_t align) noexcept {
		// Try allocate from occupied pages, from newer to older
		for (auto page = _firstPage.get(); page; page = page->nextPage.get()) {
			if (auto p = page->TryAllocate(size, align)) {
				return p;
			}
		}
		
		// No free space. Take last free page or create new one
		auto newPage = _firstFreePage ?
			std::exchange(_firstFreePage, std::move(_firstFreePage->nextPage)) :
			std::make_unique<Page>(this);
		
		// Insert new page to the front of the list
		if (_firstPage) {
			newPage->prevPage = _firstPage->prevPage;
			_firstPage->prevPage = newPage.get();
			newPage->nextPage = std::move(_firstPage);
		}
		else {
			newPage->prevPage = newPage.get();
		}
		
		_firstPage = std::move(newPage);
		
		// Allocation from empty page must succeed
		auto p = _firstPage->TryAllocate(size, align);
		assert(p != nullptr);
		return p;
	}
	
	constexpr void Deallocate(void* p) noexcept {
		// Deallocating nullptr must be ok
		if (!p) {
			return;
		}
		
		ItemHeader* header;
		auto page = GetPage(p, &header);
		
		page->Deallocate(header->size);
		
		// If deallocated from extra page and the page got empty, move it to free list
		if (page->prevPage != page && page->Empty()) {
			if (page->nextPage) {
				page->nextPage->prevPage = page->prevPage;
			}
			else {
				// If removing the tail, update last page
				_firstPage->prevPage = page->prevPage;
			}
			
			// Remove from busy list
			auto emptyPage = std::move(page->prevPage->nextPage);
			page->prevPage->nextPage = std::move(page->nextPage);
			
			// Add to free list
			emptyPage->nextPage = std::move(_firstFreePage);
			_firstFreePage = std::move(emptyPage);
		}
	}
	
	[[nodiscard]]
	static constexpr MonotonicAllocator* GetAllocator(void* p) noexcept {
		ItemHeader* header;
		auto page = GetPage(p, &header);
		return page->Allocator();
	}
	
private:
	class Page {
	public:
		// Public fields
		std::unique_ptr<Page> nextPage;
		Page* prevPage = this;
		
		constexpr explicit Page(MonotonicAllocator* allocator) noexcept
			: _header(allocator)
		{
		}
		
		constexpr ~Page() {
			// Destroy list inplace in the loop instead of auto recursion
			while (nextPage) {
				nextPage = std::move(nextPage->nextPage);
			}
		}
		
		[[nodiscard]]
		constexpr void* TryAllocate(size_t size, size_t align) noexcept {
			// Align must be non zero power of two
			assert(align && !(align & (align - 1)));
			
			// Header must be located adjacent to allocated block, align it according to the block alignment
			const auto headerAlignMask = std::max(alignof(ItemHeader), align) - 1;
			const auto blockAlignMask = align - 1;
			
			// Total allocation block begins here
			const auto baseOffset = _header.allocatedSize;
			// Aligned header
			const auto headerOffset = (_header.allocatedSize + headerAlignMask) & ~headerAlignMask;
			_header.allocatedSize = static_cast<uint32_t>(headerOffset + sizeof(ItemHeader));
			// Aligned block
			const auto blockOffset = (_header.allocatedSize + blockAlignMask) & ~blockAlignMask;
			_header.allocatedSize = static_cast<uint32_t>(blockOffset + size);
			
			if (_header.allocatedSize > _bytes.size()) {
				_header.allocatedSize = baseOffset;
				return nullptr;
			}
			
			auto p = _bytes.data() + blockOffset;
			auto header = std::launder(reinterpret_cast<ItemHeader*>(p) - 1);
			
			// Totally allocated bytes
			header->size = _header.allocatedSize - baseOffset;
			// Offset from allocated block to page start
			header->offset = static_cast<uint32_t>(p - reinterpret_cast<std::byte*>(this));
			
			return p;
		}
		
		constexpr void Deallocate(size_t size) noexcept {
			_header.deallocatedSize += size;
			
			assert(_header.deallocatedSize <= _header.allocatedSize);
			
			if (_header.allocatedSize == _header.deallocatedSize) {
				_header.allocatedSize = 0;
				_header.deallocatedSize = 0;
			}
		}
		
		[[nodiscard]]
		constexpr bool Empty() const noexcept { return _header.allocatedSize == 0; }
		
		[[nodiscard]]
		constexpr MonotonicAllocator* Allocator() const noexcept { return _header.allocator; }
		
		[[nodiscard]]
		constexpr size_t MaxSize() const noexcept { return _bytes.size(); }
	
	private:
		struct PageHeader {
			constexpr explicit PageHeader(MonotonicAllocator* allocator_) noexcept
				: allocator(allocator_)
			{
			}
			
			MonotonicAllocator* allocator = nullptr;
			
			uint32_t allocatedSize = 0;
			uint32_t deallocatedSize = 0;
		};
		
		PageHeader _header;
		
		enum {
			kAlignFillingBits = alignof(void*) - 1,
			kDataSizeUnaligned = PageSize - sizeof(PageHeader) - sizeof(std::unique_ptr<Page>) - sizeof(Page*),
			kDataSize = (kDataSizeUnaligned + kAlignFillingBits) & ~kAlignFillingBits
		};
		
		static_assert(static_cast<int>(kDataSize) > 0);
		
		std::array<std::byte, kDataSize> _bytes;
	};
	
	static_assert(sizeof(Page) == PageSize);
	
	[[nodiscard]]
	static constexpr Page* GetPage(void* p, ItemHeader** header) noexcept {
		*header = static_cast<ItemHeader*>(p) - 1;
		auto page = reinterpret_cast<Page*>(static_cast<std::byte*>(p) - (*header)->offset);
		return page;
	}
	
private:
	std::unique_ptr<Page> _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};
