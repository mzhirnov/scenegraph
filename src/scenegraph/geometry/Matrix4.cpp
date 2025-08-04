#include <scenegraph/geometry/Matrix4.h>
#include <scenegraph/utils/FloatUtils.h>

Matrix4 Matrix4MakeOrthographic(float width, float height, float nearZ, float farZ) {
	return Matrix4 {
		2.0f / width, 0, 0, 0,
		0, 2.0f / height, 0, 0,
		0, 0, 1.0f / (nearZ - farZ), 0,
		0, 0, nearZ / (nearZ - farZ), 1
	};
}

Matrix4 Matrix4MakeOrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ) {
	return Matrix4 {
		2.0f / (right - left), 0, 0, 0,
		0, 2.0f / (top - bottom), 0, 0,
		0, 0, 1.0f / (nearZ - farZ), 0,
		(left + right) / (left - right), (top + bottom) / (bottom - top), nearZ / (nearZ - farZ), 1
	};
}

Matrix4 Matrix4MakePerspective(float width, float height, float nearZ, float farZ) {
	float scaleX = 2 * nearZ / width;
	float scaleY = 2 * nearZ / height;
	
	return Matrix4 {
		scaleX, 0, 0, 0,
		0, scaleY, 0, 0,
		0, 0, farZ / (nearZ - farZ), -1,
		0, 0, (nearZ * farZ) / (nearZ - farZ), 0
	};
}

Matrix4 Matrix4MakePerspectiveOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ) {
	float scaleX = 2 * nearZ / (right - left);
	float scaleY = 2 * nearZ / (top - bottom);
	
	return Matrix4 {
		scaleX, 0, 0, 0,
		0, scaleY, 0, 0,
		(left + right) / (right - left), (top + bottom) / (top - bottom), farZ / (nearZ - farZ), -1,
		0, 0, (nearZ * farZ) / (nearZ - farZ), 0
	};
}

Matrix4 Matrix4MakePerspectiveFieldOfView(float fieldOfView, float aspectRatio, float nearZ, float farZ) {
	float scaleY = 1.0f / std::tanf(fieldOfView * 0.5f);
	float scaleX = scaleY / aspectRatio;
	
	return Matrix4 {
		scaleX, 0, 0, 0,
		0, scaleY, 0, 0,
		0, 0, farZ / (nearZ - farZ), -1,
		0, 0, (nearZ * farZ) / (nearZ - farZ), 0
	};
}

Matrix4 Matrix4Scale(const Matrix4& m, float s) {
	return Matrix4 {
		m.m11 * s, m.m12 * s, m.m13 * s, m.m14 * s,
		m.m21 * s, m.m22 * s, m.m23 * s, m.m24 * s,
		m.m31 * s, m.m32 * s, m.m33 * s, m.m34 * s,
		m.m41 * s, m.m42 * s, m.m43 * s, m.m44 * s
	};
}

Matrix4 Matrix4Add(const Matrix4& m1, const Matrix4& m2) {
	return Matrix4 {
		m1.m11 + m2.m11, m1.m12 + m2.m12, m1.m13 + m2.m13, m1.m14 + m2.m14,
		m1.m21 + m2.m21, m1.m22 + m2.m22, m1.m23 + m2.m23, m1.m24 + m2.m24,
		m1.m31 + m2.m31, m1.m32 + m2.m32, m1.m33 + m2.m33, m1.m34 + m2.m34,
		m1.m41 + m2.m41, m1.m42 + m2.m42, m1.m43 + m2.m43, m1.m44 + m2.m44
	};
}

Matrix4 Matrix4Subtract(const Matrix4& m1, const Matrix4& m2) {
	return Matrix4 {
		m1.m11 - m2.m11, m1.m12 - m2.m12, m1.m13 - m2.m13, m1.m14 - m2.m14,
		m1.m21 - m2.m21, m1.m22 - m2.m22, m1.m23 - m2.m23, m1.m24 - m2.m24,
		m1.m31 - m2.m31, m1.m32 - m2.m32, m1.m33 - m2.m33, m1.m34 - m2.m34,
		m1.m41 - m2.m41, m1.m42 - m2.m42, m1.m43 - m2.m43, m1.m44 - m2.m44
	};
}

Matrix4 Matrix4Multiply(const Matrix4& m1, const Matrix4& m2) {
	Matrix4 out;
	
	out.m11 = m1.m11 * m2.m11 + m1.m12 * m2.m21 + m1.m13 * m2.m31 + m1.m14 * m2.m41;
	out.m12 = m1.m11 * m2.m12 + m1.m12 * m2.m22 + m1.m13 * m2.m32 + m1.m14 * m2.m42;
	out.m13 = m1.m11 * m2.m13 + m1.m12 * m2.m23 + m1.m13 * m2.m33 + m1.m14 * m2.m43;
	out.m14 = m1.m11 * m2.m14 + m1.m12 * m2.m24 + m1.m13 * m2.m34 + m1.m14 * m2.m44;
	
	out.m21 = m1.m21 * m2.m11 + m1.m22 * m2.m21 + m1.m23 * m2.m31 + m1.m24 * m2.m41;
	out.m22 = m1.m21 * m2.m12 + m1.m22 * m2.m22 + m1.m23 * m2.m32 + m1.m24 * m2.m42;
	out.m23 = m1.m21 * m2.m13 + m1.m22 * m2.m23 + m1.m23 * m2.m33 + m1.m24 * m2.m43;
	out.m24 = m1.m21 * m2.m14 + m1.m22 * m2.m24 + m1.m23 * m2.m34 + m1.m24 * m2.m44;
	
	out.m31 = m1.m31 * m2.m11 + m1.m32 * m2.m21 + m1.m33 * m2.m31 + m1.m34 * m2.m41;
	out.m32 = m1.m31 * m2.m12 + m1.m32 * m2.m22 + m1.m33 * m2.m32 + m1.m34 * m2.m42;
	out.m33 = m1.m31 * m2.m13 + m1.m32 * m2.m23 + m1.m33 * m2.m33 + m1.m34 * m2.m43;
	out.m34 = m1.m31 * m2.m14 + m1.m32 * m2.m24 + m1.m33 * m2.m34 + m1.m34 * m2.m44;
	
	out.m41 = m1.m41 * m2.m11 + m1.m42 * m2.m21 + m1.m43 * m2.m31 + m1.m44 * m2.m41;
	out.m42 = m1.m41 * m2.m12 + m1.m42 * m2.m22 + m1.m43 * m2.m32 + m1.m44 * m2.m42;
	out.m43 = m1.m41 * m2.m13 + m1.m42 * m2.m23 + m1.m43 * m2.m33 + m1.m44 * m2.m43;
	out.m44 = m1.m41 * m2.m14 + m1.m42 * m2.m24 + m1.m43 * m2.m34 + m1.m44 * m2.m44;
	
	return out;
}

Matrix4 Matrix4Invert(const Matrix4& m, bool* invertible) {
	float A2323 = DifferenceOfProducts(m.m33, m.m44, m.m34, m.m43);
	float A1323 = DifferenceOfProducts(m.m32, m.m44, m.m34, m.m42);
	float A1223 = DifferenceOfProducts(m.m32, m.m43, m.m33, m.m42);
	float A0323 = DifferenceOfProducts(m.m31, m.m44, m.m34, m.m41);
	float A0223 = DifferenceOfProducts(m.m31, m.m43, m.m33, m.m41);
	float A0123 = DifferenceOfProducts(m.m31, m.m42, m.m32, m.m41);
	float A2313 = DifferenceOfProducts(m.m23, m.m44, m.m24, m.m43);
	float A1313 = DifferenceOfProducts(m.m22, m.m44, m.m24, m.m42);
	float A1213 = DifferenceOfProducts(m.m22, m.m43, m.m23, m.m42);
	float A2312 = DifferenceOfProducts(m.m23, m.m34, m.m24, m.m33);
	float A1312 = DifferenceOfProducts(m.m22, m.m34, m.m24, m.m32);
	float A1212 = DifferenceOfProducts(m.m22, m.m33, m.m23, m.m32);
	float A0313 = DifferenceOfProducts(m.m21, m.m44, m.m24, m.m41);
	float A0213 = DifferenceOfProducts(m.m21, m.m43, m.m23, m.m41);
	float A0312 = DifferenceOfProducts(m.m21, m.m34, m.m24, m.m31);
	float A0212 = DifferenceOfProducts(m.m21, m.m33, m.m23, m.m31);
	float A0113 = DifferenceOfProducts(m.m21, m.m42, m.m22, m.m41);
	float A0112 = DifferenceOfProducts(m.m21, m.m32, m.m22, m.m31);

	float det =
		  m.m11 * (DifferenceOfProducts(m.m22, A2323, m.m23, A1323) + m.m24 * A1223)
		- m.m12 * (DifferenceOfProducts(m.m21, A2323, m.m23, A0323) + m.m24 * A0223)
		+ m.m13 * (DifferenceOfProducts(m.m21, A1323, m.m22, A0323) + m.m24 * A0123)
		- m.m14 * (DifferenceOfProducts(m.m21, A1223, m.m22, A0223) + m.m23 * A0123);
	
	if (invertible && !(*invertible = det > std::numeric_limits<float>::epsilon())) {
		return Matrix4MakeZero();
	}
	
	det = 1.0f / det;

	return Matrix4 {
		/* m11 */ det *  (DifferenceOfProducts(m.m22, A2323, m.m23, A1323) + m.m24 * A1223),
		/* m12 */ det * -(DifferenceOfProducts(m.m12, A2323, m.m13, A1323) + m.m14 * A1223),
		/* m13 */ det *  (DifferenceOfProducts(m.m12, A2313, m.m13, A1313) + m.m14 * A1213),
		/* m14 */ det * -(DifferenceOfProducts(m.m12, A2312, m.m13, A1312) + m.m14 * A1212),
		/* m21 */ det * -(DifferenceOfProducts(m.m21, A2323, m.m23, A0323) + m.m24 * A0223),
		/* m22 */ det *  (DifferenceOfProducts(m.m11, A2323, m.m13, A0323) + m.m14 * A0223),
		/* m23 */ det * -(DifferenceOfProducts(m.m11, A2313, m.m13, A0313) + m.m14 * A0213),
		/* m24 */ det *  (DifferenceOfProducts(m.m11, A2312, m.m13, A0312) + m.m14 * A0212),
		/* m31 */ det *  (DifferenceOfProducts(m.m21, A1323, m.m22, A0323) + m.m24 * A0123),
		/* m32 */ det * -(DifferenceOfProducts(m.m11, A1323, m.m12, A0323) + m.m14 * A0123),
		/* m33 */ det *  (DifferenceOfProducts(m.m11, A1313, m.m12, A0313) + m.m14 * A0113),
		/* m34 */ det * -(DifferenceOfProducts(m.m11, A1312, m.m12, A0312) + m.m14 * A0112),
		/* m41 */ det * -(DifferenceOfProducts(m.m21, A1223, m.m22, A0223) + m.m23 * A0123),
		/* m42 */ det *  (DifferenceOfProducts(m.m11, A1223, m.m12, A0223) + m.m13 * A0123),
		/* m43 */ det * -(DifferenceOfProducts(m.m11, A1213, m.m12, A0213) + m.m13 * A0113),
		/* m44 */ det *  (DifferenceOfProducts(m.m11, A1212, m.m12, A0212) + m.m13 * A0112)
	};
}
