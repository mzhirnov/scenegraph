#pragma once

#include <scenegraph/math/Plane.h>

#include <array>

struct Matrix4;
struct Vector3;
struct Sphere;

enum class ClippingPlane { Left, Right, Top, Bottom, Near, Far };

struct Frustum {
	// Planes follow in order of ClippingPlane values
	std::array<Plane, 6> clippingPlanes;
};

Frustum FrustumMakeWithMatrix(const Matrix4& m);

void FrustumNormalize(Frustum& frustum);

// Checks if point is culled by (lays outside) the frustum
bool FrustumCullPoint(const Frustum& frustum, const Vector3& p);
// Checks if sphere is culled by (lays outside) the frustum
bool FrustumCullSphere(const Frustum& frustum, const Sphere& s);
