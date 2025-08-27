#pragma once

#include <functional>

#include <cstdint>

template <typename... Attribute>
struct Vertex : Attribute... {
	static constexpr uint32_t Size() noexcept { return sizeof(Vertex); }
	
	template <typename Callback>
	constexpr void ForEachAttribute(Callback&& callback) noexcept {
		(std::invoke(std::forward<Callback>(callback), this, static_cast<Attribute*>(this)), ...);
	}
};

template <std::size_t Stage = 0>
struct AttributePosition3 {
	float x, y, z;
};

template <std::size_t Stage = 0>
struct AttributeColor {
	uint8_t r, g, b, a;
};
