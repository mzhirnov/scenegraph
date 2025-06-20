#pragma once

#include <array>
#include <cstddef>

///
/// Free list paged pool allocator
///
template <size_t Size, size_t Align, size_t PageSize>
class BasicPoolAllocator {
public:
	constexpr BasicPoolAllocator() = default;
	
	[[nodiscard]]
	constexpr void* Allocate() noexcept {
		// Try allocate from occupied pages, from newer to older
		for (auto page = &_firstPage; page; page = page->nextPage.get()) {
			if (auto p = page->TryAllocate()) {
				return p;
			}
		}
		
		// No free space. Take last free page or create new one
		auto newPage = _firstFreePage ?
			std::exchange(_firstFreePage, std::move(_firstFreePage->nextPage)) :
			std::make_unique<Page>();
		
		// Insert new page to the end of the list
		assert(_firstPage.prevPage->nextPage == nullptr);
		newPage->prevPage = _firstPage.prevPage;
		_firstPage.prevPage->nextPage = std::move(newPage);
		_firstPage.prevPage = _firstPage.prevPage->nextPage.get();
		
		// Allocation from empty page must succeed
		auto p = _firstPage.prevPage->TryAllocate();
		assert(p != nullptr);
		return p;
	}
	
	constexpr void Deallocate(void* p) noexcept {
		// Deallocating nullptr must be ok
		if (!p) {
			return;
		}
		
		if (_firstPage.TryDeallocate(p)) {
			return;
		}
		
		// Try deallocate from extra pages, older down to newer
		for (auto page = _firstPage.nextPage.get(); page; page = page->nextPage.get()) {
			if (page->TryDeallocate(p)) {
				// If deallocated and the page got empty, move it to free list
				if (page->Empty()) {
					if (page->nextPage) {
						page->nextPage->prevPage = page->prevPage;
					}
					else {
						// If removing the tail, update last page
						_firstPage.prevPage = page->prevPage;
					}
					
					// Remove from busy list
					auto emptyPage = std::move(page->prevPage->nextPage);
					page->prevPage->nextPage = std::move(page->nextPage);
					
					// Add to free list
					emptyPage->nextPage = std::move(_firstFreePage);
					_firstFreePage = std::move(emptyPage);
				}
				return;
			}
		}
		
		assert(false && "Invalid pointer");
	}
	
private:
	class Page {
	public:
		using IndexType = uint16_t;
		
		static_assert(std::numeric_limits<IndexType>::max() >= PageSize);
		static_assert(sizeof(IndexType) <= Size);
		
		// Public fields
		std::unique_ptr<Page> nextPage;
		Page* prevPage = this;
		
		constexpr Page() noexcept {
			// Format page by setting up free list indices
			for (size_t i = 0; i < PageSize; ++i) {
				_items[i].nextFreeIndex = static_cast<IndexType>(i + 1);
			}
		}
		
		constexpr Page(const Page&) = delete;
		constexpr Page& operator=(const Page&) = delete;
		
		constexpr Page(Page&&) = delete;
		constexpr Page& operator=(Page&&) = delete;
		
		constexpr ~Page() {
			assert(_size == 0 && "Destroying items storage with external pointers to it");
			
			// Destroy list inplace in the loop instead of auto recursion
			while (nextPage) {
				nextPage = std::move(nextPage->nextPage);
			}
		}
		
		[[nodiscard]]
		constexpr void* TryAllocate() noexcept {
			// The page is full
			if (_freeListHead >= PageSize) {
				return nullptr;
			}
			
			// Remove item from free list
			auto& storage = _items[_freeListHead];
			_freeListHead = storage.nextFreeIndex;
			
			_size++;
			
			return storage.bytes.data();
		}
		
		constexpr bool TryDeallocate(void* p) noexcept {
			// The pointer doesn't belong to this page
			if (!IsOwnPointer(p)) {
				return false;
			}
			
			// Add item to free list
			auto storage = StorageFromPointer(p);
			storage->nextFreeIndex = _freeListHead;
			_freeListHead = GetPointerIndex(p);
			
			assert(_freeListHead <= PageSize);
			assert(_size > 0);
			
			_size--;
			
			return true;
		}
		
		[[nodiscard]]
		constexpr bool IsOwnPointer(const void* p) const noexcept {
			return p >= _items.front().bytes.data() && p <= _items.back().bytes.data();
		}
		
		[[nodiscard]]
		constexpr bool Empty() const noexcept { return _size == 0; }
		
	private:
		struct ItemStorage {
			union {
				IndexType nextFreeIndex;
				alignas(Align) std::array<std::byte, Size> bytes;
			};
		};
		
		static constexpr ItemStorage* StorageFromPointer(void* p) {
			return static_cast<ItemStorage*>(
				static_cast<void*>(
					static_cast<std::byte*>(p) -
					offsetof(ItemStorage, bytes)));
		}
		
		constexpr IndexType GetPointerIndex(const void* p) const noexcept {
			return static_cast<IndexType>(
				static_cast<const ItemStorage*>(p) -
				static_cast<const ItemStorage*>(static_cast<const void*>(&_items)));
		}
		
	private:
		std::array<ItemStorage, PageSize> _items;
		IndexType _freeListHead = 0;
		IndexType _size = 0;
	};
	
	Page _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};

template <typename T, size_t PageSize>
using PoolAllocator = BasicPoolAllocator<sizeof(T), alignof(T), PageSize>;
