#pragma once

#include <memory>
#include <array>
#include <cstddef>

///
/// Free list paged pool allocator
///
template <size_t Size, size_t Align, size_t PageItems>
class BasicPoolAllocator {
public:
	BasicPoolAllocator() = default;
	
	[[nodiscard]]
	static BasicPoolAllocator* GetAllocator(void* p) noexcept {
		auto page = GetPage(p);
		return page->Allocator();
	}
	
	template <typename T>
	[[nodiscard]] T* Allocate() noexcept {
		// Type must be complete
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		
		// Type must fit pool item
		static_assert(sizeof(T) <= Size);
		static_assert(alignof(T) <= Align);
		
		return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
	}
	
	[[nodiscard]]
	void* Allocate() noexcept {
		return Allocate(Size, Align);
	}
	
	[[nodiscard]]
	void* Allocate(size_t size, size_t align) noexcept {
		assert(size <= Size);
		assert(align <= Align);
		
		if (size > Size || align > Align) {
			return nullptr;
		}
		
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
	class Page {
	public:
		using IndexType = uint16_t;
		
		static_assert(std::numeric_limits<IndexType>::max() >= PageItems);
		static_assert(sizeof(IndexType) <= Size);
		
		// Public fields
		std::unique_ptr<Page> nextPage;
		Page* prevPage = this;
		
		struct ItemStorage {
			union {
				uint32_t offset;
				IndexType nextFreeIndex;
			};
			alignas(Align) std::array<std::byte, Size> bytes;
		};
		
		explicit Page(BasicPoolAllocator* allocator) noexcept
			: _allocator(allocator)
		{
			// Format page by setting up free list indices
			for (size_t i = 0; i < PageItems; ++i) {
				_items[i].nextFreeIndex = static_cast<IndexType>(i + 1);
			}
		}
		
		Page(const Page&) = delete;
		Page& operator=(const Page&) = delete;
		
		Page(Page&&) = delete;
		Page& operator=(Page&&) = delete;
		
		~Page() {
			assert(_allocatedCount == 0 && "Destroying items storage with external pointers to it");
			
			// Destroy list inplace in the loop instead of auto recursion
			while (nextPage) {
				nextPage = std::move(nextPage->nextPage);
			}
		}
		
		[[nodiscard]]
		void* TryAllocate(size_t, size_t) noexcept {
			// The page is full
			if (_freeListHead >= PageItems) {
				return nullptr;
			}
			
			// Remove item from free list
			auto& storage = _items[_freeListHead];
			_freeListHead = storage.nextFreeIndex;
			
			_allocatedCount++;
			
			auto p = storage.bytes.data();
			storage.offset = static_cast<uint32_t>(p - reinterpret_cast<std::byte*>(this));
			return p;
		}
		
		void Deallocate(void* p) noexcept {
			assert(IsOwnPointer(p));
			
			// Add item to free list
			auto storage = StorageFromPointer(p);
			storage->nextFreeIndex = _freeListHead;
			_freeListHead = GetStorageIndex(storage);
			
			assert(_freeListHead <= PageItems);
			assert(_allocatedCount > 0);
			
			_allocatedCount--;
		}
		
		[[nodiscard]]
		bool Empty() const noexcept { return _allocatedCount == 0; }
		
		[[nodiscard]]
		bool Single() const noexcept { return prevPage == this; }
		
		[[nodiscard]]
		BasicPoolAllocator* Allocator() const noexcept { return _allocator; }
		
		[[nodiscard]]
		static ItemStorage* StorageFromPointer(void* p) {
			return static_cast<ItemStorage*>(
				static_cast<void*>(
					static_cast<std::byte*>(p) -
					offsetof(ItemStorage, bytes)));
		}
		
	private:
		[[nodiscard]]
		bool IsOwnPointer(const void* p) const noexcept {
			return p >= _items.front().bytes.data() && p <= _items.back().bytes.data();
		}
		
		[[nodiscard]]
		IndexType GetStorageIndex(const ItemStorage* p) const noexcept {
			return static_cast<IndexType>(p - _items.data());
		}
		
	private:
		BasicPoolAllocator* _allocator = nullptr;
		IndexType _freeListHead = 0;
		IndexType _allocatedCount = 0;
		std::array<ItemStorage, PageItems> _items;
	};
	
	[[nodiscard]]
	static Page* GetPage(void* p) noexcept {
		auto storage = Page::StorageFromPointer(p);
		auto page = reinterpret_cast<Page*>(static_cast<std::byte*>(p) - storage->offset);
		return page;
	}
	
private:
	std::unique_ptr<Page> _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};

template <typename T, size_t PageSize>
using PoolAllocator = BasicPoolAllocator<sizeof(T), alignof(T), PageSize>;
