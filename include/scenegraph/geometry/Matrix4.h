#pragma once

struct Matrix4 {
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
};

inline Matrix4 Matrix4MakeZero() {
	return Matrix4 {
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
}

inline Matrix4 Matrix4MakeIdentity() {
	return Matrix4 {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

inline Matrix4 Matrix4Make(
	float m11, float m12, float m13, float m14,
	float m21, float m22, float m23, float m24,
	float m31, float m32, float m33, float m34,
	float m41, float m42, float m43, float m44)
{
	return Matrix4 {
		m11, m12, m13, m14,
		m21, m22, m23, m24,
		m31, m32, m33, m34,
		m41, m42, m43, m44
	};
}

inline Matrix4 Matrix4MakeAndTranspose(
	float m11, float m12, float m13, float m14,
	float m21, float m22, float m23, float m24,
	float m31, float m32, float m33, float m34,
	float m41, float m42, float m43, float m44)
{
	return Matrix4 {
		m11, m21, m31, m41,
		m12, m22, m32, m42,
		m13, m23, m33, m43,
		m14, m24, m34, m44
	};
}

inline Matrix4 Matrix4MakeScale(float sx, float sy, float sz) {
	return Matrix4 {
		sx, 0, 0, 0,
		0, sy, 0, 0,
		0, 0, sz, 0,
		0, 0, 0, 1
	};
}

inline Matrix4 Matrix4MakeTranslation(float tx, float ty, float tz) {
	return Matrix4 {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		tx, ty, tz, 1
	};
}

Matrix4 Matrix4MakeOrthographic(float width, float height, float nearZ, float farZ);
Matrix4 Matrix4MakeOrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ);
Matrix4 Matrix4MakePerspective(float width, float height, float nearZ, float farZ);
Matrix4 Matrix4MakePerspectiveOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ);
Matrix4 Matrix4MakePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearZ, float farZ);

Matrix4 Matrix4Scale(const Matrix4& m, float s);
Matrix4 Matrix4Add(const Matrix4& m1, const Matrix4& m2);
Matrix4 Matrix4Subtract(const Matrix4& m1, const Matrix4& m2);
Matrix4 Matrix4Multiply(const Matrix4& m1, const Matrix4& m2);
Matrix4 Matrix4Invert(const Matrix4& m, bool* invertible);
Matrix4 Matrix4InvertAndTranspose(const Matrix4& m, bool* invertible);

inline Matrix4 operator+(const Matrix4& m1, const Matrix4& m2) { return Matrix4Add(m1, m2); }
inline Matrix4 operator-(const Matrix4& m1, const Matrix4& m2) { return Matrix4Subtract(m1, m2); }
inline Matrix4 operator*(const Matrix4& m1, const Matrix4& m2) { return Matrix4Multiply(m1, m2); }
inline Matrix4 operator*(const Matrix4& m, float s) { return Matrix4Scale(m, s); }
