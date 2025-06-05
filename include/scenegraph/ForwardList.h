#pragma once

#include <type_traits>
#include <iterator>
#include <utility>

//---------------------------------------------------------------------------------------------------------------------
// ForwardListNode
//---------------------------------------------------------------------------------------------------------------------

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

//---------------------------------------------------------------------------------------------------------------------
// ForwardList
//---------------------------------------------------------------------------------------------------------------------

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
	{}
	
	constexpr ForwardList& operator=(ForwardList&& rhs) noexcept {
		if (&rhs != this) {
			_head = std::exchange(rhs._head, {});
		}
		return *this;
	}
	
	constexpr iterator begin() noexcept { return iterator{&_head}; }
	constexpr iterator end() noexcept { return iterator{}; }
	
	constexpr const_iterator begin() const noexcept { return const_iterator{&_head}; }
	constexpr const_iterator end() const noexcept { return const_iterator{}; }
	
	constexpr const_iterator cbegin() const noexcept { return const_iterator{&_head}; }
	constexpr const_iterator cend() const noexcept { return const_iterator{}; }
	
	constexpr bool Empty() const noexcept { return !_head; }
	constexpr void Clear() noexcept { _head = {}; }

	constexpr T& Front() noexcept { return static_cast<T&>(*_head); }
	constexpr const T& Front() const noexcept { return static_cast<const T&>(*_head); }

	constexpr T& Back() noexcept { return static_cast<T&>(*Tail()); }
	constexpr const T& Back() const noexcept { return static_cast<const T&>(*Tail()); }

	constexpr void PushFront(T& node) noexcept { node._next = _head; _head = &node; }
	constexpr void PushBack(T& node) noexcept { *Tail() = &node; node._next = {}; }

	constexpr void Prepend(ForwardList& list) noexcept { list.Append(*this); swap(*this, list); }
	constexpr void Append(ForwardList& list) noexcept { *Tail() = std::exchange(list._head, {}); }

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
			current->_next = std::exchange(prev, current);
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

//---------------------------------------------------------------------------------------------------------------------
// ForwardList::iterator
//---------------------------------------------------------------------------------------------------------------------

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

	constexpr reference operator*() const noexcept { return static_cast<reference>(**_current); }
	constexpr pointer operator->() const noexcept { return static_cast<pointer>(*_current); }

	constexpr iterator_impl& operator++() noexcept { _current = &(*_current)->_next; return *this; }
	constexpr iterator_impl operator++(int) noexcept { auto ret = *this; ++*this; return ret; }

	constexpr bool operator==(iterator_impl rhs) const noexcept { return _current == rhs._current || (Terminal() && rhs.Terminal()); }
	constexpr bool operator!=(iterator_impl rhs) const noexcept { return !operator==(std::move(rhs)); }

private:
	friend class ForwardList<T, Tag>;

	constexpr explicit iterator_impl(NodeType** current) noexcept : _current{current} {}

	constexpr iterator_impl& EraseAndMoveForward() noexcept {
		if (_current) {
			*_current = std::exchange((*_current)->_next, {});
		}
		return *this;
	}
	
	constexpr bool Terminal() const noexcept { return !_current || !*_current; }

private:
	NodeType** _current{};
};

//---------------------------------------------------------------------------------------------------------------------
// CircularForwardList
//---------------------------------------------------------------------------------------------------------------------

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
	{}
	
	constexpr CircularForwardList& operator=(CircularForwardList&& rhs) noexcept {
		if (&rhs != this) {
			_last = std::exchange(rhs._last, {});
		}
		return *this;
	}

	constexpr iterator before_begin() noexcept { return iterator{&_last}; }
	constexpr const_iterator before_begin() const noexcept { return const_iterator{&_last}; }
	constexpr const_iterator cbefore_begin() const noexcept { return const_iterator{&_last}; }

	constexpr iterator begin() noexcept { return iterator{HeadOrNull()}; }
	constexpr const_iterator begin() const noexcept { return const_iterator{HeadOrNull()}; }
	constexpr const_iterator cbegin() const noexcept { return const_iterator{HeadOrNull()}; }

	constexpr iterator end() noexcept { return iterator{}; }
	constexpr const_iterator end() const noexcept { return const_iterator{}; }
	constexpr const_iterator cend() const noexcept { return const_iterator{}; }

	constexpr bool Empty() const noexcept { return !_last; }
	constexpr void Clear() noexcept { _last = {}; }

	constexpr T& Front() noexcept { return static_cast<T&>(*_last->_next); }
	constexpr const T& Front() const noexcept { return static_cast<const T&>(*_last->_next); }

	constexpr T& Back() noexcept { return static_cast<T&>(*_last); }
	constexpr const T& Back() const noexcept { return static_cast<const T&>(*_last); }

	constexpr void PushFront(T& node) noexcept { InsertAfter(before_begin(), node); }
	constexpr void PushBack(T& node) noexcept { InsertAfter(before_begin(), node); _last = &node; }

	constexpr iterator InsertAfter(iterator pos, T& node) noexcept {
		if (auto after = pos.operator->()) {
			node._next = after->_next;
			after->_next = &node;
			return iterator{&after->_next};
		}
		else {
			_last = &node;
			node._next = &node;
			return before_begin();
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
				current->_next = std::exchange(prev, current);
				current = next;
			}
			while (current != head);

			_last = head;
		}
	}

	constexpr void Swap(CircularForwardList<T, Tag>& rhs) noexcept { std::swap(_last, rhs._last); }

	constexpr friend void swap(CircularForwardList<T, Tag>& lhs, CircularForwardList<T, Tag>& rhs) noexcept { lhs.Swap(rhs); }

private:
	constexpr NodeType** HeadOrNull() noexcept {
		return _last ? &_last->_next : nullptr;
	}

private:
	NodeType* _last{};
};

//---------------------------------------------------------------------------------------------------------------------
// CircularForwardList::iterator
//---------------------------------------------------------------------------------------------------------------------

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

	constexpr operator iterator_impl<const U>() const noexcept { return iterator_impl<const U>{_current, _head}; }

	constexpr reference operator*() const noexcept { return static_cast<reference>(**_current); }
	constexpr pointer operator->() const noexcept { return static_cast<pointer>(*_current); }

	constexpr iterator_impl& operator++() noexcept {
		if (_current) {
			_current = &(*_current)->_next;
			if (*_current == *_head) {
				_current = {};
			}
		}
		return *this;
	}

	constexpr iterator_impl operator++(int) noexcept { auto ret = *this; ++*this; return ret; }

	constexpr bool operator==(iterator_impl rhs) const noexcept { return _current == rhs._current || (Terminal() && rhs.Terminal()); }
	constexpr bool operator!=(iterator_impl rhs) const noexcept { return !operator==(std::move(rhs)); }

private:
	friend class CircularForwardList<T, Tag>;

	constexpr explicit iterator_impl(NodeType** current) noexcept : _current{current}, _head{current} {}
	constexpr explicit iterator_impl(NodeType** current, NodeType** head) noexcept : _current{current}, _head{head} {}

	constexpr iterator_impl& EraseAndMoveForward() noexcept {
		if (_current) {
			if (*_current == (*_current)->_next) {
				*_current = {};
				_current = {};
			}
			else {
				*_current = (*_current)->_next;
				if (_current != _head && *_current == *_head) {
					*_current = {};
					_current = {};
				}
			}
		}
		return *this;
	}
	
	constexpr bool Terminal() const noexcept { return !_current || !*_current; }

private:
	NodeType** _current{};
	NodeType** _head{};
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
