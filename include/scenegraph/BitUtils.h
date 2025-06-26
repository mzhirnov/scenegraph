#pragma once

#include <cstddef>

constexpr uint32_t PopCount(uint32_t x) {
	auto a = x - ((x >> 1) & 0x55555555);
	auto b = (((a >> 2) & 0x33333333) + (a & 0x33333333));
	auto c = (((b >> 4) + b) & 0x0f0f0f0f);
	auto d = c + (c >> 8);
	auto e = d + (d >> 16);
	return e & 0x0000003f;
};

constexpr uint32_t Log2(uint32_t x) {
	auto a = x | (x >> 1);
	auto b = a | (a >> 2);
	auto c = b | (b >> 4);
	auto d = c | (c >> 8);
	auto e = d | (d >> 16);
	return PopCount(e >> 1);
};
