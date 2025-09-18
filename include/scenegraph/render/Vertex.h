#pragma once

#include <functional>

#include <cstdint>

template <typename... Attribute>
struct Vertex : Attribute... {
	static constexpr uint32_t Stride = sizeof(Vertex);
	static constexpr uint32_t AttributesCount = sizeof...(Attribute);
	
	template <typename Callback>
	constexpr void ForEachAttribute(Callback&& callback) noexcept {
		(std::invoke(std::forward<Callback>(callback), this, static_cast<Attribute*>(this)), ...);
	}
};

template <std::size_t Slot = 0>
struct Position3Attribute {
	float x, y, z;
};

template <std::size_t Slot = 0>
struct ColorAttribute {
	uint8_t r, g, b, a;
};

template <typename T>
constexpr std::size_t AttributeSlot = ~0u;

template <template <std::size_t> typename Attribute, std::size_t Slot>
constexpr std::size_t AttributeSlot<Attribute<Slot>> = Slot;
