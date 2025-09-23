#pragma once

#include <type_traits>
#include <iterator>
#include <functional>
#include <utility>
#include <cassert>

///
/// ForwardListNode
///
template <typename T, typename Tag = void>
class ForwardListNode {
public:
	using value_type = T;
	
	constexpr ForwardListNode() = default;

	constexpr ForwardListNode(const ForwardListNode&) noexcept {}
	constexpr ForwardListNode& operator=(const ForwardListNode&) noexcept { return *this; }
	
	constexpr ForwardListNode(ForwardListNode&&) noexcept {}
	constexpr ForwardListNode& operator=(ForwardListNode&&) noexcept { return *this; }

	constexpr void Swap(ForwardListNode<Tag>& rhs) noexcept { std::swap(_next, rhs._next); }

	constexpr friend void swap(ForwardListNode<Tag>& lhs, ForwardListNode<Tag>& rhs) noexcept { lhs.Swap(rhs); }

private:
	template <typename T1, typename T2> friend class ForwardList;
	template <typename T1, typename T2> friend class CircularForwardList;

	ForwardListNode* _next{};
	
private:
	static constexpr ForwardListNode* Split(ForwardListNode* head) noexcept {
		auto slow = head;
		auto fast = head;
		
		while (fast && fast->_next) {
			if ((fast = fast->_next->_next)) {
				slow = slow->_next;
			}
		}
		
		auto mid = slow->_next;
		slow->_next = nullptr;
		return mid;
	}
	
	template <typename Compare>
	static constexpr ForwardListNode* Merge(ForwardListNode* first, ForwardListNode* second, Compare&& comp) noexcept {
		if (!first) return second;
		if (!second) return first;
		
		ForwardListNode dummy;
		auto tail = &dummy;

		while (first && second) {
			if (std::invoke(std::forward<Compare>(comp), static_cast<const T&>(*first), static_cast<const T&>(*second))) {
				tail->_next = first;
				first = first->_next;
			}
			else {
				tail->_next = second;
				second = second->_next;
			}
			tail = tail->_next;
		}

		if (first) {
			tail->_next = first;
		}
		else if (second) {
			tail->_next = second;
		}

		return dummy._next;
	}
	
	template <typename Compare>
	static constexpr ForwardListNode* MergeSort(ForwardListNode* head, Compare&& comp) noexcept {
		if (!head || !head->_next) {
			return head;
		}
		
		auto mid = Split(head);
		
		head = MergeSort(head, std::forward<Compare>(comp));
		mid = MergeSort(mid, std::forward<Compare>(comp));
		
		return Merge(head, mid, std::forward<Compare>(comp));
	}
};

///
/// ForwardList
///
template <typename T, typename Tag = void>
class ForwardList {
private:
	template <typename U> class iterator_impl;

public:
	using value_type = T;
	using size_type = std::size_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using iterator = iterator_impl<T>;
	using const_iterator = iterator_impl<const T>;
	
	using NodeType = ForwardListNode<T, Tag>;

	static_assert(std::is_base_of_v<NodeType, T>);

	constexpr ForwardList() = default;

	constexpr ForwardList(const ForwardList&) noexcept = delete;
	constexpr ForwardList& operator=(const ForwardList&) noexcept = delete;
	
	constexpr ForwardList(ForwardList&& rhs) noexcept
		: _head{std::exchange(rhs._head, {})}
	{
	}
	
	constexpr ForwardList& operator=(ForwardList&& rhs) noexcept {
		if (&rhs != this) {
			_head = std::exchange(rhs._head, {});
		}
		return *this;
	}
	
	constexpr iterator begin() noexcept { return iterator{_head ? &_head : nullptr}; }
	constexpr const_iterator begin() const noexcept { return const_iterator{_head ? &_head : nullptr}; }
	constexpr const_iterator cbegin() const noexcept { return const_iterator{_head ? &_head : nullptr}; }
	
	constexpr iterator end() noexcept { return iterator{}; }
	constexpr const_iterator end() const noexcept { return const_iterator{}; }
	constexpr const_iterator cend() const noexcept { return const_iterator{}; }
	
	constexpr bool Empty() const noexcept { return !_head; }
	constexpr void Clear() noexcept { _head = {}; }

	constexpr T& Front() noexcept { assert(_head); return static_cast<T&>(*_head); }
	constexpr const T& Front() const noexcept { assert(_head); return static_cast<const T&>(*_head); }

	constexpr T& Back() noexcept { assert(_head); return static_cast<T&>(**LastOrTail()); }
	constexpr const T& Back() const noexcept { assert(_head); return static_cast<const T&>(**LastOrTail()); }

	constexpr void PushFront(T& node) noexcept { node._next = _head; _head = &node; }
	constexpr void PushBack(T& node) noexcept { *Tail() = &node; node._next = {}; }

	constexpr void PrependList(ForwardList& other) noexcept { other.Append(*this); *this = std::move(other); }
	constexpr void AppendList(ForwardList& other) noexcept { *Tail() = std::exchange(other._head, {}); }

//	constexpr void Splice(const_iterator pos, ForwardList& other) noexcept {
//	}
//
//	constexpr void Splice(const_iterator pos, ForwardList& other, const_iterator it) noexcept {
//	}
//
//	constexpr void Splice(const_iterator pos, ForwardList& other, const_iterator first, const_iterator last) noexcept {
//	}

	constexpr void Merge(ForwardList& other) noexcept { Merge(other, std::less<value_type>{}); }
	
	template <typename Compare>
	constexpr void Merge(ForwardList& other, Compare&& comp) noexcept {
		_head = NodeType::Merge(_head, other._head, std::forward<Compare>(comp));
		other._head = {};
	}
	
	constexpr void Sort() noexcept { Sort(std::less<T>{}); }
	
	template <typename Compare>
	constexpr void Sort(Compare&& comp) noexcept {
		_head = NodeType::MergeSort(_head, std::forward<Compare>(comp));
	}

	constexpr iterator Erase(iterator it) noexcept { return it.EraseAndMoveForward(); }

	constexpr void Rotate() noexcept {
		if (_head && _head->_next) {
			auto tmp = _head;
			_head = _head->_next;
			*Tail() = tmp;
			tmp->_next = {};
		}
	}

	constexpr void Rotate(std::size_t n) noexcept {
		if (_head && _head->_next) {
			auto tail_ = Tail();
			while (n--) {
				auto tmp = _head;
				_head = _head->_next;
				*tail_ = tmp;
				tmp->_next = {};
				tail_ = &tmp->_next;
			}
		}
	}

	constexpr void Reverse() noexcept {
		NodeType* prev = nullptr;
		NodeType* current = _head;
		while (current) {
			auto next = current->_next;
			current->_next = prev;
			prev = current;
			current = next;
		}
		_head = prev;
	}

	constexpr void Swap(ForwardList<T, Tag>& rhs) noexcept { std::swap(_head, rhs._head); }

	constexpr friend void swap(ForwardList<T, Tag>& lhs, ForwardList<T, Tag>& rhs) noexcept { lhs.Swap(rhs); }

private:
	constexpr NodeType** LastOrTail() noexcept {
		auto pp = &_head;
		while (*pp && (*pp)->_next) {
			pp = &(*pp)->_next;
		}
		return pp;
	}
	
	constexpr NodeType** Tail() noexcept {
		auto pp = &_head;
		while (*pp) {
			pp = &(*pp)->_next;
		}
		return pp;
	}

private:
	NodeType* _head{};
};

///
/// ForwardList::iterator
///
template <typename T, typename Tag>
template <typename U>
class ForwardList<T, Tag>::iterator_impl {
public:
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::size_t;
	using value_type = U;
	using pointer = value_type*;
	using reference = value_type&;

	constexpr explicit iterator_impl() = default;

	constexpr operator iterator_impl<const U>() const noexcept { return iterator_impl<const U>{_current}; }

	constexpr reference operator*() const noexcept { assert(_current); return static_cast<reference>(**_current); }
	constexpr pointer operator->() const noexcept { assert(_current); return static_cast<pointer>(*_current); }

	constexpr iterator_impl& operator++() noexcept {
		assert(_current);
		_current = &(*_current)->_next;
		if (!*_current) {
			_current = {};
		}
		return *this;
	}
	constexpr iterator_impl operator++(int) noexcept { auto ret = *this; ++*this; return ret; }

	constexpr bool operator==(iterator_impl rhs) const noexcept { return _current == rhs._current; }
	constexpr bool operator!=(iterator_impl rhs) const noexcept { return _current != rhs._current; }
	
	constexpr bool Last() const noexcept { return _current && !_current->_next; }

private:
	friend class ForwardList<T, Tag>;

	constexpr explicit iterator_impl(NodeType** current) noexcept : _current{current} {}

	constexpr iterator_impl& EraseAndMoveForward() noexcept {
		assert(_current);
		if (!(*_current = (*_current)->_next)) {
			_current = {};
		}
		return *this;
	}

private:
	NodeType** _current{};
};

///
/// CircularForwardList
///
template <typename T, typename Tag = void>
class CircularForwardList {
private:
	template <typename U> class iterator_impl;

public:
	using value_type = T;
	using size_type = std::size_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using iterator = iterator_impl<T>;
	using const_iterator = iterator_impl<const T>;
	
	using NodeType = ForwardListNode<T, Tag>;

	static_assert(std::is_base_of_v<NodeType, T>);

	constexpr CircularForwardList() = default;

	constexpr CircularForwardList(const CircularForwardList&) noexcept = delete;
	constexpr CircularForwardList& operator=(const CircularForwardList&) noexcept = delete;
	
	constexpr CircularForwardList(CircularForwardList&& rhs) noexcept
		: _last{std::exchange(rhs._last, {})}
	{
	}
	
	constexpr CircularForwardList& operator=(CircularForwardList&& rhs) noexcept {
		if (&rhs != this) {
			_last = std::exchange(rhs._last, {});
		}
		return *this;
	}
	
	constexpr iterator begin() noexcept { return iterator{_last ? &_last : nullptr}; }
	constexpr const_iterator begin() const noexcept { return const_iterator{_last ? &_last : nullptr}; }
	constexpr const_iterator cbegin() const noexcept { return const_iterator{_last ? &_last : nullptr}; }

	constexpr iterator end() noexcept { return iterator{}; }
	constexpr const_iterator end() const noexcept { return const_iterator{}; }
	constexpr const_iterator cend() const noexcept { return const_iterator{}; }

	constexpr bool Empty() const noexcept { return !_last; }
	constexpr void Clear() noexcept { _last = {}; }

	constexpr T& Front() noexcept { assert(_last); return static_cast<T&>(*_last->_next); }
	constexpr const T& Front() const noexcept { assert(_last); return static_cast<const T&>(*_last->_next); }

	constexpr T& Back() noexcept { assert(_last); return static_cast<T&>(*_last); }
	constexpr const T& Back() const noexcept { assert(_last); return static_cast<const T&>(*_last); }

	constexpr void PushFront(T& node) noexcept { InsertBefore(begin(), node); }
	constexpr void PushBack(T& node) noexcept { InsertBefore(begin(), node); _last = &node; }
	
	constexpr iterator InsertBefore(iterator pos, T& node) noexcept {
		if (auto after = pos._currentLast ? *pos._currentLast : nullptr) {
			node._next = after->_next;
			after->_next = &node;
			return iterator{&after->_next};
		}
		else {
			node._next = &node;
			_last = &node;
			return begin();
		}
	}
	
	constexpr void PrependList(CircularForwardList& other) noexcept { other.Append(*this); *this = std::move(other); }
	constexpr void AppendList(CircularForwardList& other) noexcept {
		if (other.Empty()) {
			return;
		}
		
		if (_last) {
			auto tmp = _last->_next;
			_last->_next = other._last->_next;
			other._last->_next = tmp;
		}
		
		_last = other._last;
		other._last = {};
	}
	
//	constexpr void Splice(const_iterator pos, CircularForwardList& other) noexcept {
//		if (auto after = pos._currentLast; after && *after && other._last) {
//			auto tmp = (*after)->_next;
//			(*after)->_next = other._last->_next;
//			other._last->_next = tmp;
//			*after = other._last;
//		}
//		other._last = {};
//	}
//
//	constexpr void Splice(const_iterator pos, CircularForwardList& other, const_iterator it) noexcept {
//		if (it != other.end()) {
//			auto& node = *it;
//			it.EraseAndMoveForward();
//			InsertBefore(pos, node);
//		}
//		if (auto after = pos._currentLast; after && *after && it != other.end()) {
//			auto tmp = (*after)->_next;
//			(*after)->_next = (*it._currentLast)->_next;
//			if ((*it._currentLast)->_next == *it._currentLast) {
//				*it._currentLast = {};
//			}
//			else {
//				(*it._currentLast)->_next = (*it._currentLast)->_next->_next;
//			}
//			//it.EraseAndMoveForward();
//			(*after)->_next = tmp;
//		}
//	}

//	constexpr void Splice(const_iterator pos, CircularForwardList& other, const_iterator first, const_iterator last) noexcept {
//	}

	constexpr void Merge(CircularForwardList& other) noexcept { Merge(other, std::less<value_type>{}); }
	
	template <typename Compare>
	constexpr void Merge(CircularForwardList& other, Compare&& comp) noexcept {
		NodeType dummy;
		auto tail = &dummy;
		
		auto head1 = _last ? _last->_next : nullptr;
		auto head2 = other._last ? other._last->_next : nullptr;
		
		// Break cycle
		if (_last) {
			_last->_next = {};
		}
		if (other._last) {
			other._last->_next = {};
		}
		
		while (head1 && head2) {
			if (std::invoke(std::forward<Compare>(comp), static_cast<const_reference>(*head1), static_cast<const_reference>(*head2))) {
				tail->_next = head1;
				head1 = head1->_next;
			}
			else {
				tail->_next = head2;
				head2 = head2->_next;
			}
			tail = tail->_next;
			
			if (!head1) {
				_last = other._last;
			}
		}
		
		if (head1) {
			tail->_next = head1;
		}
		else if (head2) {
			tail->_next = head2;
		}
		
		// Cycle back
		_last->_next = dummy._next;
		
		other._last = {};
	}
	
	constexpr void Sort() noexcept { Sort(std::less<T>{}); }
	
	template <typename Compare>
	constexpr void Sort(Compare&& comp) noexcept {
		if (!_last || _last->_next == _last) {
			return;
		}
		
		auto head = _last->_next;
		auto last = _last;
		
		// Break cycle
		_last->_next = {};
		
		head = NodeType::MergeSort(head, std::forward<Compare>(comp));
		
		// Find sorted last node.
		// Starting from unsorted last (<= N) better then starting from sorted head (N).
		_last = last;
		while (_last->_next) {
			_last = _last->_next;
		}
		
		// Cycle back
		_last->_next = head;
	}
	
	constexpr iterator Erase(iterator it) noexcept { return it.EraseAndMoveForward(); }

	constexpr void Rotate() noexcept {
		if (_last) {
			_last = _last->_next;
		}
	}

	constexpr void Rotate(size_type n) noexcept {
		if (_last && _last != _last->_next) {
			while (n--) {
				_last = _last->_next;
			}
		}
	}

	constexpr void Reverse() noexcept {
		if (_last && _last != _last->_next) {
			auto head = _last->_next;
			auto prev = _last;
			auto current = head;

			do {
				auto next = current->_next;
				current->_next = prev;
				prev = current;
				current = next;
			}
			while (current != head);

			_last = head;
		}
	}

	constexpr void Swap(CircularForwardList<T, Tag>& rhs) noexcept { std::swap(_last, rhs._last); }

	constexpr friend void swap(CircularForwardList<T, Tag>& lhs, CircularForwardList<T, Tag>& rhs) noexcept { lhs.Swap(rhs); }

private:
	NodeType* _last{};
};

///
/// CircularForwardList::iterator
///
template <typename T, typename Tag>
template <typename U>
class CircularForwardList<T, Tag>::iterator_impl {
public:
	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::size_t;
	using value_type = U;
	using pointer = value_type*;
	using reference = value_type&;

	constexpr explicit iterator_impl() = default;

	constexpr operator iterator_impl<const U>() const noexcept { return iterator_impl<const U>{_currentLast, _initialLast}; }

	constexpr reference operator*() const noexcept { assert(_currentLast); return static_cast<reference>(*(*_currentLast)->_next); }
	constexpr pointer operator->() const noexcept { assert(_currentLast); return static_cast<pointer>((*_currentLast)->_next); }

	constexpr iterator_impl& operator++() noexcept {
		if (_currentLast) {
			_currentLast = &(*_currentLast)->_next;
			if (*_currentLast == *_initialLast) {
				// This is the end
				_currentLast = {};
			}
		}
		return *this;
	}

	constexpr iterator_impl operator++(int) noexcept { auto ret = *this; ++*this; return ret; }

	constexpr bool operator==(iterator_impl rhs) const noexcept { return _currentLast == rhs._currentLast; }
	constexpr bool operator!=(iterator_impl rhs) const noexcept { return _currentLast != rhs._currentLast; }
	
	constexpr bool Last() const noexcept { return _currentLast && (*_currentLast)->_next == *_initialLast; }

private:
	friend class CircularForwardList<T, Tag>;

	constexpr explicit iterator_impl(NodeType** last) noexcept : _currentLast{last}, _initialLast{last} {}
	constexpr explicit iterator_impl(NodeType** last, NodeType** initialLast) noexcept : _currentLast{last}, _initialLast{initialLast} {}

	constexpr iterator_impl& EraseAndMoveForward() noexcept {
		if (_currentLast) {
			if (*_currentLast == (*_currentLast)->_next) {
				// If single element, it's the _last one, so just clear it
				*_currentLast = {};
				_currentLast = {};
			}
			else {
				// Overwrite current with next one
				(*_currentLast)->_next = (*_currentLast)->_next->_next;
				if (_currentLast != _initialLast && (*_currentLast)->_next == (*_initialLast)->_next) {
					// If removed the tail, modify _last
					*_initialLast = *_currentLast;
					_currentLast = {};
				}
			}
		}
		return *this;
	}

private:
	NodeType** _currentLast{};
	NodeType** _initialLast{};
};

//---------------------------------------------------------------------------------------------------------------------
// std
//---------------------------------------------------------------------------------------------------------------------

namespace std {
	template <typename Tag>
	constexpr void swap(ForwardListNode<Tag>& lhs, ForwardListNode<Tag>& rhs) noexcept {
		lhs.Swap(rhs);
	}

	template <typename T, typename Tag>
	constexpr void swap(ForwardList<T, Tag>& lhs, ForwardList<T, Tag>& rhs) noexcept {
		lhs.Swap(rhs);
	}

	template <typename T, typename Tag>
	constexpr void swap(CircularForwardList<T, Tag>& lhs, CircularForwardList<T, Tag>& rhs) noexcept {
		lhs.Swap(rhs);
	}
}
