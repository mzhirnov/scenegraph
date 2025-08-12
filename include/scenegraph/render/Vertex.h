#pragma once

#include <functional>

#include <cstdint>

template <typename... Attr>
struct Vertex : Attr... {
	static constexpr uint32_t Size() noexcept { return sizeof(Vertex); }
	
	template <typename Callback>
	constexpr void ForEachAttribute(Callback&& callback) noexcept {
		(std::invoke(std::forward<Callback>(callback), this, static_cast<Attr*>(this)), ...);
	}
};

template <std::size_t Stage = 0>
struct Position3 {
	float x, y, z;
};

template <std::size_t Stage = 0>
struct Color {
	uint8_t r, g, b, a;
};
