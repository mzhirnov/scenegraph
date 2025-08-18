#include <scenegraph/math/Vector2.h>
#include <scenegraph/math/Matrix32.h>

Vector2 Vector2TransformCoord(Vector2 v, const Matrix32& m) {
	Vector2 out;
	out.x = v.x * m.a + v.y * m.c + m.tx;
	out.x = v.x * m.b + v.y * m.b + m.ty;
	return out;
}

Vector2 Vector2TransformNormal(Vector2 v, const Matrix32& m) {
	Vector2 out;
	out.x = v.x * m.a + v.y * m.c;
	out.x = v.x * m.b + v.y * m.b;
	return out;
}
