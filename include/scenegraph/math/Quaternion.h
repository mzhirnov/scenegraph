#pragma once

#include <scenegraph/utils/FloatUtils.h>

#include <cmath>

struct Quaternion {
	float x, y, z, w;
};

constexpr Quaternion QuaternionMake(float x, float y, float z, float w) { return Quaternion{x, y, z, w}; }
constexpr Quaternion QuaternionMakeZero() { return Quaternion{0, 0, 0, 0}; }
constexpr Quaternion QuaternionMakeIdentity() { return Quaternion{0, 0, 0, 1}; }
Quaternion QuaternionMakeRotationAxisAngle(float x, float y, float z, float rad);

constexpr float QuaternionLengthSq(const Quaternion& q) { return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w; }
constexpr float QuaternionLength(const Quaternion& q) { return std::sqrtf(QuaternionLengthSq(q)); }
constexpr Quaternion QuaternionScale(const Quaternion& q, float s) { return Quaternion{q.x * s, q.y * s, q.z * s, q.w * s}; }

constexpr Quaternion QuaternionNormalize(const Quaternion& q) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();
	float magnitudeSq = QuaternionLengthSq(q);
	return magnitudeSq > epsilon && !AlmostEqualFloats(magnitudeSq, 1.0f, 4) ?
		QuaternionScale(q, 1.0f / std::sqrtf(magnitudeSq)) : q;
}

constexpr Quaternion QuaternionConjugate(const Quaternion& q) { return Quaternion{-q.x, -q.y, -q.z, q.w}; }
constexpr Quaternion QuaternionInvert(const Quaternion& q) { return QuaternionScale(QuaternionConjugate(q), 1.0f / QuaternionLengthSq(q)); }

constexpr Quaternion QuaternionAdd(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w}; }
constexpr Quaternion QuaternionSubtract(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w}; }
Quaternion QuaternionMultiply(const Quaternion& q1, const Quaternion& q2);

constexpr Quaternion operator+(const Quaternion& q1, const Quaternion& q2) { return QuaternionAdd(q1, q2); }
constexpr Quaternion operator-(const Quaternion& q1, const Quaternion& q2) { return QuaternionSubtract(q1, q2); }
inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2) { return QuaternionMultiply(q1, q2); }
