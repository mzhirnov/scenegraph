#include <scenegraph/geometry/Quaternion.h>
#include <scenegraph/geometry/Matrix4.h>

Quaternion QuaternionMakeRotationAxisAngle(float x, float y, float z, float rad) {
	float s = x * x + y * y + z * z;
	if (!AlmostEqualFloats(s, 1.0f, 4)) {
		s = 1.0f / std::sqrtf(s);
	}
	
	// Embed scaling to sin value in order to normalize vector
	float sn2 = std::sinf(rad * 0.5f) * s;
	float cs2 = std::cosf(rad * 0.5f);
	
	return Quaternion{sn2 * x, sn2 * y, sn2 * z, cs2};
}

Quaternion QuaternionMultiply(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion {
		DifferenceOfProducts(q1.w, q2.x, -q1.x, q2.w) + DifferenceOfProducts(q1.y, q2.z,  q1.z, q2.y),
		DifferenceOfProducts(q1.w, q2.y,  q1.x, q2.z) + DifferenceOfProducts(q1.y, q2.w, -q1.z, q2.x),
		DifferenceOfProducts(q1.w, q2.z, -q1.z, q2.w) + DifferenceOfProducts(q1.x, q2.y,  q1.y, q2.x),
		DifferenceOfProducts(q1.w, q2.w,  q1.x, q2.x) - DifferenceOfProducts(q1.y, q2.y,  q1.z, q2.z)
	};
}
