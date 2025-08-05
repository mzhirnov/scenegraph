#pragma once

#include <scenegraph/utils/FloatUtils.h>

#include <cmath>

struct Quaternion {
	float x, y, z, w;
};

inline Quaternion QuaternionMake(float x, float y, float z, float w) { return Quaternion{x, y, z, w}; }
inline Quaternion QuaternionMakeZero() { return Quaternion{0, 0, 0, 0}; }
inline Quaternion QuaternionMakeIdentity() { return Quaternion{0, 0, 0, 1}; }
inline Quaternion QuaternionMakeRotationAxisAngle(float x, float y, float z, float rad);

inline float QuaternionLengthSq(const Quaternion& q) { return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w; }
inline float QuaternionLength(const Quaternion& q) { return std::sqrtf(QuaternionLengthSq(q)); }
inline Quaternion QuaternionScale(const Quaternion& q, float s) { return Quaternion{q.x * s, q.y * s, q.z * s, q.w * s}; }

inline Quaternion QuaternionNormalize(const Quaternion& q) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();
	float magnitudeSq = QuaternionLengthSq(q);
	return magnitudeSq > epsilon && !AlmostEqualFloats(magnitudeSq, 1.0f, 4) ?
		QuaternionScale(q, 1.0f / std::sqrtf(magnitudeSq)) : q;
}

inline Quaternion QuaternionConjugate(const Quaternion& q) { return Quaternion{-q.x, -q.y, -q.z, q.w}; }
inline Quaternion QuaternionInvert(const Quaternion& q) { return QuaternionScale(QuaternionConjugate(q), 1.0f / QuaternionLengthSq(q)); }

inline Quaternion QuaternionAdd(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w}; }
inline Quaternion QuaternionSubtract(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w}; }
Quaternion QuaternionMultiply(const Quaternion& q1, const Quaternion& q2);

inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2) { return QuaternionAdd(q1, q2); }
inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2) { return QuaternionSubtract(q1, q2); }
inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2) { return QuaternionMultiply(q1, q2); }
