#pragma once

#include <scenegraph/math/Vector3.h>

struct Plane {
	Vector3 normal;
	float distance;
};

inline Plane PlaneMake(Vector3 normal, float distance) {
	Plane out;
	out.normal = normal;
	out.distance = distance;
	return out;
}

void PlaneNormalize(Plane& plane);

float PlaneDistanceToPoint(const Plane& plane, const Vector3& p);
