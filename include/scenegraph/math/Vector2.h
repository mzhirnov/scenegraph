#pragma once

#include <scenegraph/utils/FloatUtils.h>

struct Matrix32;

struct Vector2 {
	float x, y;
};

inline Vector2 Vector2Make(float x, float y) { return Vector2 {x, y}; }
inline Vector2 Vector2MakeZero() { return Vector2 {0, 0}; }
inline Vector2 Vector2MakeOne() { return Vector2 {1, 1}; }
inline Vector2 Vector2MakeUnitX() { return Vector2 {1, 0}; }
inline Vector2 Vector2MakeUnitY() { return Vector2 {0, 1}; }

inline float Vector2LengthSq(Vector2 v) { return v.x * v.x + v.y * v.y; }
inline float Vector2Length(Vector2 v) { return std::sqrtf(Vector2LengthSq(v)); }
inline Vector2 Vector2Scale(Vector2 v, float s) { return Vector2 {v.x * s, v.y * s}; }
inline Vector2 Vector3Negate(Vector2 v) { return Vector2 {-v.x, -v.y}; }

inline Vector2 Vector2Normalize(Vector2 v) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();
	float magnitudeSq = Vector2LengthSq(v);
	return magnitudeSq > epsilon && !AlmostEqualFloats(magnitudeSq, 1.0f, 4) ?
		Vector2Scale(v, 1.0f / std::sqrtf(magnitudeSq)) : v;
}

inline Vector2 Vector2Max(Vector2 v1, Vector2 v2) { return Vector2 { std::max(v1.x, v2.x), std::max(v1.y, v2.y) }; }
inline Vector2 Vector2Min(Vector2 v1, Vector2 v2) { return Vector2 { std::min(v1.x, v2.x), std::min(v1.y, v2.y) }; }
inline Vector2 Vector2Add(Vector2 v1, Vector2 v2) { return Vector2 {v1.x + v2.x, v1.y + v2.y}; }
inline Vector2 Vector2Subtract(Vector2 v1, Vector2 v2) { return Vector2 {v1.x - v2.x, v1.y - v2.y}; }

inline Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float s) {
	return Vector2 {
		DifferenceOfProducts(v2.x, s, v1.x, s - 1.0f),
		DifferenceOfProducts(v2.y, s, v1.y, s - 1.0f)
	};
}

inline float Vector2Cross(Vector2 v1, Vector2 v2) { return DifferenceOfProducts(v1.x, v2.y, v1.y, v2.x); }
inline float Vector2Dot(Vector2 v1, Vector2 v2) { return v1.x * v2.x + v1.y * v2.y; }

Vector2 Vector2TransformCoord(Vector2 v, const Matrix32& m);
Vector2 Vector2TransformNormal(Vector2 v, const Matrix32& m);

inline Vector2 operator+(Vector2 v1, Vector2 v2) { return Vector2Add(v1, v2); }
inline Vector2 operator-(Vector2 v1, Vector2 v2) { return Vector2Subtract(v1, v2); }
inline Vector2 operator*(Vector2 v, float s) { return Vector2Scale(v, s); }
