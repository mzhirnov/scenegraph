#pragma once

#include <type_traits>
#include <iterator>
#include <utility>
#include <cassert>

///
/// ForwardListNode
///
template <typename Tag = void>
class ForwardListNode {
public:
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
	
	using NodeType = ForwardListNode<Tag>;

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

	constexpr T& Back() noexcept { assert(_head); return static_cast<T&>(*Tail()); }
	constexpr const T& Back() const noexcept { assert(_head); return static_cast<const T&>(*Tail()); }

	constexpr void PushFront(T& node) noexcept { node._next = _head; _head = &node; }
	constexpr void PushBack(T& node) noexcept { *Tail() = &node; node._next = {}; }

	constexpr void PrependList(ForwardList&& list) noexcept { list.Append(*this); *this = std::move(list); }
	constexpr void AppendList(ForwardList&& list) noexcept { *Tail() = std::exchange(list._head, {}); }

//	constexpr void Splice(const_iterator pos, ForwardList& other) noexcept {
//
//	}

	constexpr iterator Erase(iterator it) noexcept { return it.EraseAndMoveForward(); }

	constexpr void Rotate() noexcept {
		if (_head) {
			auto tmp = _head;
			_head = _head->_next;
			*Tail() = tmp;
			tmp->_next = {};
		}
	}

	constexpr void Rotate(std::size_t n) noexcept {
		if (_head) {
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
	
	using NodeType = ForwardListNode<Tag>;

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

	constexpr iterator Erase(iterator it) noexcept { return it.EraseAndMoveForward(); }

	constexpr void Rotate() noexcept {
		if (_last) {
			_last = _last->_next;
		}
	}

	constexpr void Rotate(size_type n) noexcept {
		if (_last) {
			while (n--) {
				_last = _last->_next;
			}
		}
	}

//	constexpr void Splice(const_iterator pos, ForwardList& other) noexcept {
//
//	}

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
		assert(_currentLast);
		
		_currentLast = &(*_currentLast)->_next;
		if (*_currentLast == *_initialLast) {
			// This is the end
			_currentLast = {};
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
		assert(_currentLast);
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
