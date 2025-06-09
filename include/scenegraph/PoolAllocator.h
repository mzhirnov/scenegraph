#pragma once

#include <array>
#include <cstddef>

///
/// Free list paged pool allocator
///
template <typename T, size_t PageSize>
class PoolAllocator {
public:
	using ValueType = T;
	
	constexpr PoolAllocator() = default;
	
	[[nodiscard]]
	constexpr T* Allocate() noexcept {
		// Try allocate from occupied pages, from newer to older
		for (auto page = _firstPage.get(); page; page = page->nextPage.get()) {
			if (auto p = page->TryAllocate()) {
				return p;
			}
		}
		
		// No free space. Take free page or create new one
		auto newPage = _firstFreePage ?
			std::exchange(_firstFreePage, std::move(_firstFreePage->nextPage)) :
			std::make_unique<Page>();
		
		// Insert new empty page in the top of the list
		newPage->nextPage = std::move(_firstPage);
		_firstPage = std::move(newPage);
		
		// Allocation from empty page must be successful
		auto p = _firstPage->TryAllocate();
		assert(p != nullptr);
		return p;
	}
	
	constexpr void Deallocate(T* p) noexcept {
		// Try deallocate from occupied pages, from newer to older
		for (auto page = &_firstPage; *page; page = &(*page)->nextPage) {
			if ((*page)->TryDeallocate(p)) {
				// If deallocated and the page got empty, move the page to the free list
				if ((*page)->Empty()) {
					auto emptyPage = std::exchange(*page, std::move((*page)->nextPage));
					emptyPage->Format();					
					emptyPage->nextPage = std::move(_firstFreePage);
					_firstFreePage = std::move(emptyPage);
				}
				return;
			}
		}
		
		assert(!"Invalid pointer");
	}
	
private:
	class Page {
	public:
		using IndexType = uint16_t;
		
		static_assert(std::numeric_limits<IndexType>::max() >= PageSize);
		static_assert(sizeof(IndexType) <= sizeof(T));
		
		struct ItemStorage {
			union {
				IndexType nextFreeIndex;
				alignas(T) std::array<std::byte, sizeof(T)> bytes;
			};
		};
		
		constexpr Page() noexcept {
			Format();
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
		constexpr T* TryAllocate() noexcept {
			// The page is full
			if (_freeListHead >= PageSize) {
				return nullptr;
			}
			
			// Remove item from free list
			auto& storage = _items[_freeListHead];
			_freeListHead = storage.nextFreeIndex;
			
			_size++;
			
			return static_cast<T*>(static_cast<void*>(storage.bytes.data()));
		}
		
		constexpr bool TryDeallocate(T* p) noexcept {
			// Deallocating nullptr must be ok
			if (!p) {
				return true;
			}
			
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
		
		constexpr void Format() noexcept {
			assert(_size == 0);
			
			_freeListHead = 0;
			
			for (size_t i = 0; i < PageSize; ++i) {
				_items[i].nextFreeIndex = static_cast<IndexType>(i + 1);
			}
		}
		
	public:
		std::unique_ptr<Page> nextPage;
		
	private:
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
	
	std::unique_ptr<Page> _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};
