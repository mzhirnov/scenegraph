#include <scenegraph/geometry/Frustum.h>
#include <scenegraph/geometry/Plane.h>
#include <scenegraph/geometry/Matrix4.h>
#include <scenegraph/geometry/Sphere.h>

Frustum FrustumMakeWithMatrix(const Matrix4& m) {
	Frustum out;
	
	 // Left clipping plane
	 out.clippingPlanes[0].normal.x = m.m14 + m.m11;
	 out.clippingPlanes[0].normal.y = m.m24 + m.m21;
	 out.clippingPlanes[0].normal.z = m.m34 + m.m31;
	 out.clippingPlanes[0].distance = m.m44 + m.m41;
	 // Right clipping plane
	 out.clippingPlanes[1].normal.x = m.m14 - m.m11;
	 out.clippingPlanes[1].normal.y = m.m24 - m.m21;
	 out.clippingPlanes[1].normal.z = m.m34 - m.m31;
	 out.clippingPlanes[1].distance = m.m44 - m.m41;
	 // Top clipping plane
	 out.clippingPlanes[2].normal.x = m.m14 - m.m12;
	 out.clippingPlanes[2].normal.y = m.m24 - m.m22;
	 out.clippingPlanes[2].normal.z = m.m34 - m.m32;
	 out.clippingPlanes[2].distance = m.m44 - m.m42;
	 // Bottom clipping plane
	 out.clippingPlanes[3].normal.x = m.m14 + m.m12;
	 out.clippingPlanes[3].normal.y = m.m24 + m.m22;
	 out.clippingPlanes[3].normal.z = m.m34 + m.m32;
	 out.clippingPlanes[3].distance = m.m44 + m.m42;
	 // Near clipping plane
	 out.clippingPlanes[4].normal.x = m.m13;
	 out.clippingPlanes[4].normal.y = m.m23;
	 out.clippingPlanes[4].normal.z = m.m33;
	 out.clippingPlanes[4].distance = m.m43;
	 // Far clipping plane
	 out.clippingPlanes[5].normal.x = m.m14 - m.m13;
	 out.clippingPlanes[5].normal.y = m.m24 - m.m23;
	 out.clippingPlanes[5].normal.z = m.m34 - m.m33;
	 out.clippingPlanes[5].distance = m.m44 - m.m43;
	 
	 return out;
}

void FrustumNormalize(Frustum& frustum) {
	for (auto& plane : frustum.clippingPlanes) {
		PlaneNormalize(plane);
	}
}

bool FrustumCullPoint(const Frustum& frustum, const Vector3& p) {
	for (auto& plane : frustum.clippingPlanes) {
		if (PlaneDistanceToPoint(plane, p) < 0) {
			return true;
		}
	}
	
	return false;
}

bool FrustumCullSphere(const Frustum& frustum, const Sphere& s) {
	for (auto& plane : frustum.clippingPlanes) {
		if (PlaneDistanceToPoint(plane, s.origin) + s.radius < 0) {
			return true;
		}
	}
	
	return false;
}
