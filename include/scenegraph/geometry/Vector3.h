#pragma once

#include <cmath>

struct Matrix4;

struct Vector3 {
	float x, y, z;
};

inline Vector3 Vector3Make(float x, float y, float z) { return Vector3 {x, y, z}; }
inline Vector3 Vector3MakeZero() { return Vector3 {0, 0, 0}; }
inline Vector3 Vector3MakeOne() { return Vector3 {1, 1, 1}; }
inline Vector3 Vector3MakeUnitX() { return Vector3 {1, 0, 0}; }
inline Vector3 Vector3MakeUnitY() { return Vector3 {0, 1, 0}; }
inline Vector3 Vector3MakeUnitZ() { return Vector3 {0, 0, 1}; }

inline float Vector3LengthSq(Vector3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
inline float Vector3Length(Vector3 v) { return std::sqrtf(Vector3LengthSq(v)); }
inline Vector3 Vector3Scale(Vector3 v1, float s) { return Vector3 {v1.x * s, v1.y * s, v1.z * s}; }

inline Vector3 Vector3Normalize(Vector3 v) {
	float magnitudeSq = Vector3LengthSq(v);
	return magnitudeSq > std::numeric_limits<float>::epsilon() ?
		Vector3Scale(v, 1.0f / std::sqrtf(magnitudeSq)) : v;
}

inline Vector3 Vector3Max(Vector3 v1, Vector3 v2) { return Vector3 { std::fmax(v1.x, v2.x), std::fmax(v1.y, v2.y), std::fmax(v1.z, v2.z) }; }
inline Vector3 Vector3Min(Vector3 v1, Vector3 v2) { return Vector3 { std::fmin(v1.x, v2.x), std::fmin(v1.y, v2.y), std::fmin(v1.z, v2.z) }; }
inline Vector3 Vector3Add(Vector3 v1, Vector3 v2) { return Vector3 {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
inline Vector3 Vector3Subtract(Vector3 v1, Vector3 v2) { return Vector3 {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

inline Vector3 Vector3Lerp(Vector3 v1, Vector3 v2, float s) {
	return Vector3 {
		v1.x + s * (v2.x - v1.x),
		v1.y + s * (v2.y - v1.y),
		v1.z + s * (v2.z - v1.z)
	};
}

inline Vector3 Vector3Cross(Vector3 v1, Vector3 v2) {
	return Vector3 {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	};
}

inline float Vector3Dot(Vector3 v1, Vector3 v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

Vector3 Vector3TransformCoord(Vector3 v, const Matrix4& m);
Vector3 Vector3TransformAndProjectCoord(Vector3 v, const Matrix4& m);
Vector3 Vector3TransformNormal(Vector3 v, const Matrix4& m);
