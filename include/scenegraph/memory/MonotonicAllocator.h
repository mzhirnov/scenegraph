#pragma once

#include <memory>
#include <array>
#include <cstddef>

///
/// Monotonic paged allocator
///
template <size_t PageBytes>
class MonotonicAllocator {
public:
	MonotonicAllocator() = default;
	
	[[nodiscard]]
	static MonotonicAllocator* GetAllocator(void* p) noexcept {
		auto page = GetPage(p);
		return page->Allocator();
	}
	
	template <typename T>
	[[nodiscard]] T* Allocate() noexcept {
		// Type must be complete
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		
		return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
	}
	
	[[nodiscard]]
	void* Allocate(size_t size, size_t align) noexcept {
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
	
	void Deallocate(void* p) noexcept {
		// Deallocating nullptr must be ok
		if (!p) {
			return;
		}
		
		auto page = GetPage(p);
		
		page->Deallocate(p);
		
		// If deallocated from extra page and the page got empty, move it to free list
		if (!page->Single() && page->Empty()) {
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
	
	void DisposeFreePages() noexcept { _firstFreePage.reset(); }
	
private:
	static constexpr uint32_t kSignature = 0xaaaa5555;
	
	struct ItemHeader {
#ifndef NDEBUG
		uint32_t signature = kSignature;
#endif
		uint32_t offset;
		uint32_t size;
	};
	
	template <typename T>
	class PageHeader {
	public:
		explicit PageHeader(MonotonicAllocator* allocator) noexcept
			: _allocator(allocator)
		{
		}
		
	protected:
		std::unique_ptr<T> nextPage;
		T* prevPage = static_cast<T*>(this);
		
		MonotonicAllocator* _allocator = nullptr;
		int _allocatedSize = 0;
		uint32_t _currentOffset = 0;
	};
	
	class Page : public PageHeader<Page> {
	public:
		// Public fields
		using PageHeader<Page>::nextPage;
		using PageHeader<Page>::prevPage;
		
		explicit Page(MonotonicAllocator* allocator) noexcept
			: PageHeader<Page>(allocator)
		{
		}
		
		~Page() {
			// Destroy list inplace in the loop instead of auto recursion
			while (nextPage) {
				nextPage = std::move(nextPage->nextPage);
			}
		}
		
		[[nodiscard]]
		void* TryAllocate(size_t size, size_t align) noexcept {
			// Align must be non zero power of two
			assert(align && !(align & (align - 1)));
			
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
		MonotonicAllocator* Allocator() const noexcept { return this->_allocator; }
	
	private:
		enum {
			kMaxAlignFillingBits = alignof(std::max_align_t) - 1,
			kDataSizeUnaligned = PageBytes - sizeof(PageHeader<Page>),
			kDataSize = (kDataSizeUnaligned + kMaxAlignFillingBits) & ~kMaxAlignFillingBits
		};
		
		static_assert(static_cast<int>(kDataSize) > 0);
		
		std::array<std::byte, kDataSize> _bytes;
	};
	
	static_assert(sizeof(Page) == PageBytes);
	
	[[nodiscard]]
	static Page* GetPage(void* p) noexcept {
		auto header = static_cast<ItemHeader*>(p) - 1;
		assert(header->signature == kSignature);
		auto page = reinterpret_cast<Page*>(static_cast<std::byte*>(p) - header->offset);
		return page;
	}
	
private:
	std::unique_ptr<Page> _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};
