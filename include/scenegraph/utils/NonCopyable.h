#pragma once

class NonCopyable {
public:
	NonCopyable() = default;
	
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

class NonCopyableNonMovable : public NonCopyable {
public:
	NonCopyableNonMovable() = default;
	
	NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
	NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
};
