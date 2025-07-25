#pragma once

#include <scenegraph/memory/BasicAllocator.h>

#include <array>

///
/// Free list pool page
///
template <std::size_t Size, std::size_t Align, std::size_t PageItems>
class PoolPage {
public:
	static_assert(Align && !(Align & (Align - 1)), "Align must be non zero power of two");
	
	using IndexType = uint16_t;
	
	static_assert(std::numeric_limits<IndexType>::max() >= PageItems);
	static_assert(sizeof(IndexType) <= Size);
	
	static constexpr std::size_t kMaxSize = Size;
	static constexpr std::size_t kMaxAlign = Align;
	
	// Public fields
	std::unique_ptr<PoolPage> nextPage;
	PoolPage* prevPage = this;
	
	explicit PoolPage(void* allocator) noexcept
		: _allocator(allocator)
	{
		// Format page by setting up free list indices
		for (std::size_t i = 0; i < PageItems; ++i) {
			_items[i].nextFreeIndex = static_cast<IndexType>(i + 1);
		}
	}
	
	PoolPage(const PoolPage&) = delete;
	PoolPage& operator=(const PoolPage&) = delete;
	
	PoolPage(PoolPage&&) = delete;
	PoolPage& operator=(PoolPage&&) = delete;
	
	~PoolPage() {
		assert(_allocatedCount == 0 && "Destroying items storage with external pointers to it");
		
		// Destroy list inplace in the loop instead of auto recursion
		while (nextPage) {
			nextPage = std::move(nextPage->nextPage);
		}
	}
	
	[[nodiscard]]
	void* TryAllocate(std::size_t, std::size_t) noexcept {
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
	void* Allocator() const noexcept { return _allocator; }
	
	[[nodiscard]]
	static PoolPage* GetPage(void* p) noexcept {
		auto storage = StorageFromPointer(p);
		auto page = reinterpret_cast<PoolPage*>(static_cast<std::byte*>(p) - storage->offset);
		return page;
	}
	
private:
	struct ItemStorage {
		union {
			uint32_t offset;
			IndexType nextFreeIndex;
		};
		alignas(Align) std::array<std::byte, Size> bytes;
	};
	
	[[nodiscard]]
	bool IsOwnPointer(const void* p) const noexcept {
		return p >= _items.front().bytes.data() && p <= _items.back().bytes.data();
	}
	
	[[nodiscard]]
	IndexType GetStorageIndex(const ItemStorage* p) const noexcept {
		return static_cast<IndexType>(p - _items.data());
	}
	
	[[nodiscard]]
	static ItemStorage* StorageFromPointer(void* p) {
		return static_cast<ItemStorage*>(
			static_cast<void*>(
				static_cast<std::byte*>(p) -
				offsetof(ItemStorage, bytes)));
	}
	
private:
	std::array<ItemStorage, PageItems> _items;
	void* _allocator = nullptr;
	IndexType _freeListHead = 0;
	IndexType _allocatedCount = 0;
};

// PoolAllocator

template <std::size_t Size, std::size_t Align, std::size_t PageItems>
using BasicPoolAllocator = BasicAllocator<PoolPage<Size, Align, PageItems>>;

template <typename T, std::size_t PageItems>
using PoolAllocator = BasicPoolAllocator<sizeof(T), alignof(T), PageItems>;

// StaticPoolAllocator

template <std::size_t Size, std::size_t Align, std::size_t PageItems>
using BasicStaticPoolAllocator = BasicStaticAllocator<PoolPage<Size, Align, PageItems>>;

template <typename T, std::size_t PageItems>
using StaticPoolAllocator = BasicStaticPoolAllocator<sizeof(T), alignof(T), PageItems>;
