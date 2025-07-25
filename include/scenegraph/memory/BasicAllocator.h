#pragma once

#include <memory>
#include <type_traits>
#include <cstddef>

///
/// Basic allocator
///
template <typename Page>
class BasicAllocator {
public:
	BasicAllocator() = default;
	
	static constexpr std::size_t kMaxSize = Page::kMaxSize;
	static constexpr std::size_t kMaxAlign = Page::kMaxAlign;
	
	[[nodiscard]]
	static BasicAllocator* GetAllocator(void* p) noexcept {
		auto page = Page::GetPage(p);
		return static_cast<BasicAllocator*>(page->Allocator());
	}
	
	template <typename T>
	[[nodiscard]] T* Allocate() noexcept {
		// Type must be complete
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		
		static_assert(sizeof(T) <= kMaxSize);
		static_assert(alignof(T) <= kMaxAlign);
		
		return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
	}
	
	[[nodiscard]]
	void* Allocate(std::size_t size, std::size_t align) noexcept {
		assert(size <= kMaxSize);
		assert(align <= kMaxAlign);
		
		if (size > kMaxSize || align > kMaxAlign) {
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
		
		auto page = Page::GetPage(p);
		
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
	std::unique_ptr<Page> _firstPage;
	std::unique_ptr<Page> _firstFreePage;
};

///
/// Basic static allocator
///
template <typename Page>
class BasicStaticAllocator {
public:
	BasicStaticAllocator() = default;
	
	static constexpr std::size_t kMaxSize = Page::kMaxSize;
	static constexpr std::size_t kMaxAlign = Page::kMaxAlign;
	
	[[nodiscard]]
	static BasicStaticAllocator* GetAllocator(void* p) noexcept {
		auto page = Page::GetPage(p);
		return static_cast<BasicStaticAllocator*>(page->Allocator());
	}
	
	template <typename T>
	[[nodiscard]] T* Allocate() noexcept {
		// Type must be complete
		static_assert(sizeof(T) > 0);
		static_assert(!std::is_void_v<T>);
		
		static_assert(sizeof(T) <= kMaxSize);
		static_assert(alignof(T) <= kMaxAlign);
		
		return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
	}
	
	[[nodiscard]]
	void* Allocate(std::size_t size, std::size_t align) noexcept {
		assert(size <= kMaxSize);
		assert(align <= kMaxAlign);
		
		if (size > kMaxSize || align > kMaxAlign) {
			return nullptr;
		}
		
		return _page.TryAllocate(size, align);
	}
	
	void Deallocate(void* p) noexcept {
		// Deallocating nullptr must be ok
		if (!p) {
			return;
		}
		
		assert(Page::GetPage(p) == std::addressof(_page));
		
		_page.Deallocate(p);
	}
	
	void DisposeFreePages() noexcept {}

private:
	Page _page{this};
};
