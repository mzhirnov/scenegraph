#pragma once

#include <scenegraph/math/Vector3.h>

struct Sphere {
	Vector3 origin;
	float radius;
};

constexpr Sphere SphereMake(Vector3 origin, float radius) {
	Sphere out;
	out.origin = origin;
	out.radius = radius;
	return out;
}

constexpr float SphereDistance(const Sphere& s1, const Sphere& s2) { return Vector3Distance(s1.origin, s2.origin) - (s1.radius + s2.radius); }
constexpr bool SphereIntersect(const Sphere& s1, const Sphere& s2) { return SphereDistance(s1, s2) < 0; }
