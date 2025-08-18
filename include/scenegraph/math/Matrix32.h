#pragma once

#include <scenegraph/math/Transform2D.h>

struct Matrix32 {
	float a, b;
	float c, d;
	float tx, ty;
};

inline Matrix32 Matrix32MakeZero() {
	return Matrix32 {
		0, 0,
		0, 0,
		0, 0
	};
}

inline Matrix32 Matrix32MakeIdentity() {
	return Matrix32 {
		1, 0,
		0, 1,
		0, 0
	};
}

inline Matrix32 Matrix32Make(float a, float b, float c, float d, float tx, float ty) {
	return Matrix32 {
		a, b,
		c, d,
		tx, ty
	};
}

inline Matrix32 Matrix32MakeScale(float sx, float sy) {
	return Matrix32 {
		sx, 0,
		0, sy,
		0, 0
	};
}

inline Matrix32 Matrix32MakeXShear(float shear) {
	return Matrix32 {
		1, -shear,
		0, 1,
		0, 0
	};
}

inline Matrix32 Matrix32MakeYShear(float shear) {
	return Matrix32 {
		1, 0,
		-shear, 1,
		0, 0
	};
}

inline Matrix32 Matrix32MakeTranslation(float tx, float ty) {
	return Matrix32 {
		1, 0,
		0, 1,
		tx, ty
	};
}

Matrix32 Matrix32MakeRotation(float rad);

Matrix32 Matrix32MakeWithTransform2D(const Transform2D& c);
bool Matrix32DecomposeToTransform2D(const Matrix32& m, Transform2D* transform);

Matrix32 Matrix32Scale(const Matrix32& m, float s);
Matrix32 Matrix32Add(const Matrix32& m1, const Matrix32& m2);
Matrix32 Matrix32Subtract(const Matrix32& m1, const Matrix32& m2);
Matrix32 Matrix32Multiply(const Matrix32& m1, const Matrix32& m2);
Matrix32 Matrix32MultiplyRotation(const Matrix32& m1, const Matrix32& m2);
Matrix32 Matrix32Invert(const Matrix32& m, bool* invertible);

inline Matrix32 operator+(const Matrix32& m1, const Matrix32& m2) { return Matrix32Add(m1, m2); }
inline Matrix32 operator-(const Matrix32& m1, const Matrix32& m2) { return Matrix32Subtract(m1, m2); }
inline Matrix32 operator*(const Matrix32& m1, const Matrix32& m2) { return Matrix32Multiply(m1, m2); }
inline Matrix32 operator*(const Matrix32& m, float s) { return Matrix32Scale(m, s); }
