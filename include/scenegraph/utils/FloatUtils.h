#pragma once

#include <scenegraph/utils/BitUtils.h>

constexpr int FloatsDifferenceULPs(float a, float b) {
	// Make aInt lexicographically ordered as a two's-complement int
	int aInt = BitCast<int>(a);
	if (aInt < 0) {
		aInt = int(0x80000000) - aInt;
	}
	
	// Make bInt lexicographically ordered as a two's-complement int
	int bInt = BitCast<int>(b);
	if (bInt < 0) {
		bInt = int(0x80000000) - bInt;
	}
	
	return std::abs(aInt - bInt);
}

constexpr bool AlmostEqualFloats(float a, float b, int ulps) {
	// Make sure ulps is small enough that the default NaN won't compare as equal to anything.
	assert(ulps > 0 && ulps < 4 * 1024 * 1024);
	return FloatsDifferenceULPs(a, b) <= ulps;
}

inline float DifferenceOfProducts(float a, float b, float c, float d) {
	float cd = c * d;
	float err = std::fma(-c, d, cd);
	float dop = std::fma(a, b, -cd);
	return dop + err;
}

inline float Clamp(float v, float min, float max) {
	assert(min < max);
	return v < min ? min : v > max ? max : v;
}

inline float Lerp(float a, float b, float s) {
	assert(s >= 0 && s <= 1);
	return DifferenceOfProducts(b, s, a, s - 1.0f);
}
