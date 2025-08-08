#include <scenegraph/geometry/Plane.h>
#include <scenegraph/geometry/Vector3.h>

void PlaneNormalize(Plane& plane) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();
	float magnitudeSq = Vector3LengthSq(plane.normal);
	if (magnitudeSq > epsilon && !AlmostEqualFloats(magnitudeSq, 1.0f, 4)) {
		float scale = 1.0f / std::sqrt(magnitudeSq);
		plane.normal = Vector3Scale(plane.normal, scale);
		plane.distance *= scale;
	}
}

float PlaneDistanceToPoint(const Plane& plane, const Vector3& p) {
	return Vector3Dot(plane.normal, p) + plane.distance;
}
