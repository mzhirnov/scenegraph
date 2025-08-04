#pragma once

struct Matrix32Components {
	float sx, sy;
	float shear;
	float rad;
	float tx, ty;
};

inline Matrix32Components Matrix32ComponentsMakeZero() {
	return Matrix32Components{0, 0, 0, 0, 0, 0};
}

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

inline Matrix32 Matrix32MakeShear(float m) {
	return Matrix32 {
		1, 0,
		m, 1,
		0, 0
	};
}

inline Matrix32 Matrix32MakeScaleAndShear(float sx, float sy, float m) {
	return Matrix32 {
		sx, 0,
		sy * m, sy,
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

Matrix32 Matrix32MakeWithComponents(const Matrix32Components& c);
Matrix32Components Matrix32DecomposeToComponents(const Matrix32& m, bool* invertible);

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
