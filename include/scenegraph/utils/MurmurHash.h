#pragma once

#include <string_view>
#include <utility>
#include <cstddef>

constexpr uint32_t Murmur3Scramble32(uint32_t k) {
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

constexpr uint32_t Murmur3Finalize32(uint32_t k) {
	k ^= k >> 16;
	k *= 0x85ebca6b;
	k ^= k >> 13;
	k *= 0xc2b2ae35;
	k ^= k >> 16;
	return k;
}

constexpr uint64_t Murmur3Finalize64(uint64_t k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccdULL;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53ULL;
	k ^= k >> 33;
	return k;
}

constexpr uint32_t Murmur3Hash32(const char* key, size_t len, uint32_t seed = 0) {
	uint32_t h = seed;
	uint32_t k = 0;
	// Read in groups of 4
	for (size_t i = len / 4; i; --i) {
		for (size_t n = 4; n; --n) {
			k = (k << 8) | static_cast<uint32_t>(key[n - 1]);
		}
		key += 4;
		h ^= Murmur3Scramble32(k);
		h = (h << 13) | (h >> 19);
		h = h * 5 + 0xe6546b64;
	}
	// Read the rest
	k = 0;
	for (size_t i = len & 3; i; --i) {
		k = (k << 8) | static_cast<uint32_t>(key[i - 1]);
	}
	h ^= Murmur3Scramble32(k);
	// Finalize
	h ^= len;
	h = Murmur3Finalize32(h);
	return h;
}

constexpr std::pair<uint64_t, uint64_t> Murmur3Hash128(const char* key, size_t len, uint64_t seed1, uint64_t seed2) {
	uint64_t h1 = seed1;
	uint64_t h2 = seed2;

	constexpr uint64_t c1 = 0x87c37b91114253d5ULL;
	constexpr uint64_t c2 = 0x4cf5ad432745937fULL;
	
	const auto nblocks = len / 16;

	// Read in groups of 16
	for (size_t i = 0; i < nblocks; ++i) {
		uint64_t k1 = 0;
		uint64_t k2 = 0;
		
		for (size_t n = 8; n; --n) {
			k1 = (k1 << 8) | static_cast<uint64_t>(key[n * 2 + 0 - 1]);
			k2 = (k2 << 8) | static_cast<uint64_t>(key[n * 2 + 1 - 1]);
		}
		
		key += 16;

		k1 *= c1;
		k1 = (k1 << 31) | (k1 >> 33);
		k1 *= c2;
		h1 ^= k1;

		h1 = (h1 << 27) | (h1 >> 37);
		h1 += h2;
		h1 = h1 * 5 + 0x52dce729;

		k2 *= c2;
		k2 = (k2 << 33) | (k2 >> 31);
		k2 *= c1;
		h2 ^= k2;

		h2 = (h2 << 31) | (h2 >> 33);
		h2 += h1;
		h2 = h2 * 5 + 0x38495ab5;
	}

	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch (len & 15) {
	case 15: k2 ^= static_cast<uint64_t>(key[14]) << 48;
	case 14: k2 ^= static_cast<uint64_t>(key[13]) << 40;
	case 13: k2 ^= static_cast<uint64_t>(key[12]) << 32;
	case 12: k2 ^= static_cast<uint64_t>(key[11]) << 24;
	case 11: k2 ^= static_cast<uint64_t>(key[10]) << 16;
	case 10: k2 ^= static_cast<uint64_t>(key[ 9]) << 8;
	case  9: k2 ^= static_cast<uint64_t>(key[ 8]) << 0;
	         k2 *= c2;
	         k2 = (k2 << 33) | (k2 >> 31);
	         k2 *= c1;
	         h2 ^= k2;
	case  8: k1 ^= static_cast<uint64_t>(key[7]) << 56;
	case  7: k1 ^= static_cast<uint64_t>(key[6]) << 48;
	case  6: k1 ^= static_cast<uint64_t>(key[5]) << 40;
	case  5: k1 ^= static_cast<uint64_t>(key[4]) << 32;
	case  4: k1 ^= static_cast<uint64_t>(key[3]) << 24;
	case  3: k1 ^= static_cast<uint64_t>(key[2]) << 16;
	case  2: k1 ^= static_cast<uint64_t>(key[1]) << 8;
	case  1: k1 ^= static_cast<uint64_t>(key[0]) << 0;
	         k1 *= c1;
	         k1 = (k1 << 31) | (k1 >> 33);
	         k1 *= c2;
	         h1 ^= k1;
	};

	h1 ^= len;
	h2 ^= len;

	h1 += h2;
	h2 += h1;

	h1 = Murmur3Finalize64(h1);
	h2 = Murmur3Finalize64(h2);

	h1 += h2;
	h2 += h1;
	
	return {h1, h2};
}

template <size_t N>
constexpr uint32_t Murmur3Hash32(const char (&str)[N], uint32_t seed = 0) {
	return Murmur3Hash32(str, N - 1, seed);
}

constexpr uint32_t Murmur3Hash32(std::string_view str, uint32_t seed = 0) {
	return Murmur3Hash32(str.data(), str.size(), seed);
}

template <size_t N>
constexpr std::pair<uint64_t, uint64_t> Murmur3Hash128(const char (&str)[N], uint64_t seed1 = 0, uint64_t seed2 = 0) {
	return Murmur3Hash128(str, N - 1, seed1, seed2);
}

constexpr std::pair<uint64_t, uint64_t> Murmur3Hash128(std::string_view str, uint64_t seed1 = 0, uint64_t seed2 = 0) {
	return Murmur3Hash128(str.data(), str.size(), seed1, seed2);
}
