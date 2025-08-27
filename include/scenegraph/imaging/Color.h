#pragma once

#include <scenegraph/math/Vector4.h>
#include <scenegraph/utils/FloatUtils.h>

#include <cstdint>

union Color {
	uint32_t value;
	struct { uint8_t r, g, b, a; } rgba;
};

constexpr Color ColorMakeZero() { return { .value = 0 }; }
constexpr Color ColorMake(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return { .rgba = {r, g, b, a} }; }
constexpr Color ColorMakeBlack() { return { .rgba = {0x00, 0x00, 0x00, 0xff} }; }
constexpr Color ColorMakeWhite() { return { .rgba = {0xff, 0xff, 0xff, 0xff} }; }
constexpr Color ColorMakeWhiteTransparent() { return { .rgba = {0xff, 0xff, 0xff, 0x00} }; }
constexpr Color ColorMakeWithVector4(const Vector4& v) {
	constexpr float s = 255.0f;
	return {
		.rgba = {
			static_cast<uint8_t>(Clamp(v.x, 0, 1) * s + 0.5f),
			static_cast<uint8_t>(Clamp(v.y, 0, 1) * s + 0.5f),
			static_cast<uint8_t>(Clamp(v.z, 0, 1) * s + 0.5f),
			static_cast<uint8_t>(Clamp(v.w, 0, 1) * s + 0.5f)
		}
	};
}

constexpr Vector4 ColorToVector4(Color c) {
	constexpr float s = 1.0f / 255.0f;
	return { c.rgba.r * s, c.rgba.g * s, c.rgba.b * s, c.rgba.a * s };
}

constexpr Color ColorMultiply(Color a, Color b) {
	return {
		.rgba = {
			static_cast<uint8_t>((a.rgba.r * b.rgba.r) >> 8),
			static_cast<uint8_t>((a.rgba.g * b.rgba.g) >> 8),
			static_cast<uint8_t>((a.rgba.b * b.rgba.b) >> 8),
			static_cast<uint8_t>((a.rgba.a * b.rgba.a) >> 8)
		}
	};
}

constexpr Color ColorScale(Color a, uint8_t s) {
	return {
		.rgba = {
			static_cast<uint8_t>((a.rgba.r * s) >> 8),
			static_cast<uint8_t>((a.rgba.g * s) >> 8),
			static_cast<uint8_t>((a.rgba.b * s) >> 8),
			static_cast<uint8_t>((a.rgba.a * s) >> 8)
		}
	};
}

struct FloatColor {
	float r, g, b, a;
};

constexpr FloatColor FloatColorMakeZero() { return {}; }
constexpr FloatColor FloatColorMake(float r, float g, float b, float a) { return { r, g, b, a }; }
constexpr FloatColor FloatColorMakeBlack() { return { 0, 0, 0, 1 }; }
constexpr FloatColor FloatColorMakeWhite() { return { 1, 1, 1, 1 }; }
constexpr FloatColor FloatColorMakeWhiteTransparent() { return { 1, 1, 1, 0 }; }
constexpr FloatColor FloatColorMakeWithVector4(const Vector4& v) {
	return {
		Clamp(v.x, 0, 1),
		Clamp(v.y, 0, 1),
		Clamp(v.z, 0, 1),
		Clamp(v.w, 0, 1)
	};
}
constexpr FloatColor FloatColorMakeWithColor(Color c) {
	auto v = ColorToVector4(c);
	return { v.x, v.y, v.z, v.w };
}

constexpr Vector4 FloatColorToVector4(const FloatColor& c) { return { c.r, c.g, c.b, c.a }; }

constexpr FloatColor FloatColorMultiply(FloatColor a, FloatColor b) {
	return {
		a.r * b.r,
		a.g * b.g,
		a.b * b.b,
		a.a * b.a
	};
}

constexpr FloatColor FloatColorScale(const FloatColor& c, float s) {
	s = Clamp(s, 0, 1);
	return {c.r * s,c.g * s,c.b * s,c.a * s};
}
