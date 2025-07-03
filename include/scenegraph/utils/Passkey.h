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

//template <typename T>
//class Passkey : private NonCopyableNonMovable {
//public:
//	template <typename U>
//	constexpr Passkey(Passkey<U>&&) noexcept {}
//	
//private:
//	friend T;
//	
//	constexpr explicit Passkey() = default;
//	constexpr explicit Passkey(const T*) noexcept {}
//};

template <typename T, typename... Ts>
class Passkey : private Passkey<Ts>... {
public:
	template <typename U>
	constexpr Passkey(Passkey<U>&&) noexcept {}
	
private:
	friend T;
	
	constexpr explicit Passkey() = default;
	constexpr explicit Passkey(const T*) noexcept {}
};

template <typename T> Passkey(const T*) -> Passkey<T>;
