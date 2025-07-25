#pragma once

#include <iterator>

template <typename T>
struct ReversionWrapper { T& iterable; };

template <typename T>
auto begin(ReversionWrapper<T> w) noexcept { return std::rbegin(w.iterable); }

template <typename T>
auto end(ReversionWrapper<T> w) noexcept { return std::rend(w.iterable); }

template <typename T>
ReversionWrapper<T> ReverseRange(T&& iterable) noexcept { return { iterable }; }
