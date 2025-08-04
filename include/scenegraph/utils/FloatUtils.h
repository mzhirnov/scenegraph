#pragma once

#include <scenegraph/utils/BitUtils.h>

constexpr bool AlmostEqualFloats(float a, float b, int ulps) {
	// Make sure ulps is small enough that the default NaN won't compare as equal to anything.
	assert(ulps > 0 && ulps < 4 * 1024 * 1024);
	
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
	
	return std::abs(aInt - bInt) <= ulps;
}

inline float DifferenceOfProducts(float a, float b, float c, float d) {
	float cd = c * d;
	float err = std::fma(-c, d, cd);
	float dop = std::fma(a, b, -cd);
	return dop + err;
}
