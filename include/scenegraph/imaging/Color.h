#pragma once

#include <cstdint>

union Color {
	uint32_t value;
	struct { uint8_t r, g, b, a; } rgba;
};

inline Color ColorMakeZero() { return { .value = 0 }; }

inline Color ColorMake(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return { .rgba = {r, g, b, a} }; }

inline Color ColorMakeBlack() { return { .rgba = {0x00, 0x00, 0x00, 0xff} }; }

inline Color ColorMakeWhite() { return { .rgba = {0xff, 0xff, 0xff, 0xff} }; }

inline Color ColorMakeWhiteTransparent() { return { .rgba = {0xff, 0xff, 0xff, 0x00} }; }

struct FloatColor {
	float r, g, b, a;
};

inline FloatColor FloatColorMakeZero() { return {}; }

inline FloatColor FloatColorMake(float r, float g, float b, float a) { return { r, g, b, a }; }

inline FloatColor FloatColorMakeBlack() { return { 0, 0, 0, 1 }; }

inline FloatColor FloatColorMakeWhite() { return { 1, 1, 1, 1 }; }

inline FloatColor FloatColorMakeWhiteTransparent() { return { 1, 1, 1, 0 }; }
