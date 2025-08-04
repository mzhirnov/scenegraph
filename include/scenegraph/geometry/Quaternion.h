#pragma once

struct Quaternion {
	float x, y, z, w;
};

inline Quaternion QuaternionMake(float x, float y, float z, float w) { return Quaternion{x, y, z, w}; }
inline Quaternion QuaternionMakeZero() { return Quaternion{0, 0, 0, 0}; }
inline Quaternion QuaternionMakeIdentity() { return Quaternion{0, 0, 0, 1}; }

inline Quaternion QuaternionConjugate(const Quaternion& q) { return Quaternion{-q.x, -q.y, -q.z, -q.w}; }
inline Quaternion QuaternionAdd(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w}; }
inline Quaternion QuaternionSubtract(const Quaternion& q1, const Quaternion& q2) { return Quaternion{q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w}; }

inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2) { return QuaternionAdd(q1, q2); }
inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2) { return QuaternionSubtract(q1, q2); }
