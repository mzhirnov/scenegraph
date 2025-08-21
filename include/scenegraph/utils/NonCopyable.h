#pragma once

///
/// Non Copyable
///
class NonCopyable {
public:
	NonCopyable() = default;
	
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	
	NonCopyable(NonCopyable&&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;
	
protected:
	~NonCopyable() = default;
};

///
/// Non Movable
///
class NonMovable {
public:
	NonMovable() = default;
	
	NonMovable(const NonMovable&) = default;
	NonMovable& operator=(const NonMovable&) = default;
	
	NonMovable(NonMovable&&) = delete;
	NonMovable& operator=(NonMovable&&) = delete;
	
protected:
	~NonMovable() = default;
};

///
/// Non Copyable Non Movable
///
class NonCopyableNonMovable {
public:
	NonCopyableNonMovable() = default;
	
	NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
	NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;
	
	NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
	NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
	
protected:
	~NonCopyableNonMovable() = default;
};
