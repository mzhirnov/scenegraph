#include <scenegraph/math/Vector3.h>
#include <scenegraph/math/Quaternion.h>
#include <scenegraph/math/Matrix4.h>

Vector3 Vector3Rotate(const Vector3& v, const Quaternion& q) {
	Quaternion p{v.x, v.y, v.z, 0};
	auto out = q * p * QuaternionConjugate(q);
	return Vector3{out.x, out.y, out.z};
}

Vector3 Vector3Rotate(const Vector3& v, const Matrix4& m) {
	Vector3 out;
	out.x = v.x * m.m11 + v.y * m.m21 + v.z * m.m31;
	out.y = v.x * m.m12 + v.y * m.m22 + v.z * m.m32;
	out.z = v.x * m.m13 + v.y * m.m23 + v.z * m.m33;
	return out;
}

Vector3 Vector3Transform(const Vector3& v, const Matrix4& m) {
	Vector3 out;
	out.x = v.x * m.m11 + v.y * m.m21 + v.z * m.m31 + m.m41;
	out.y = v.x * m.m12 + v.y * m.m22 + v.z * m.m32 + m.m42;
	out.z = v.x * m.m13 + v.y * m.m23 + v.z * m.m33 + m.m43;
	return out;
}

Vector3 Vector3TransformAndProject(const Vector3& v, const Matrix4& m) {
	Vector3 out;
	float w = 1.0f / (v.x * m.m14 + v.y * m.m24 + v.z * m.m34 + m.m44);
	out.x = (v.x * m.m11 + v.y * m.m21 + v.z * m.m31 + m.m41) * w;
	out.y = (v.x * m.m12 + v.y * m.m22 + v.z * m.m32 + m.m42) * w;
	out.z = (v.x * m.m13 + v.y * m.m23 + v.z * m.m33 + m.m43) * w;
	return out;
}
