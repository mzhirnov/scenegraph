#pragma once

struct Transform2D {
	float sx, sy;
	float shearX, shearY;
	float rad;
	float tx, ty;
};

inline Transform2D Transform2DMakeZero() {
	return Transform2D{};
}

inline Transform2D Transform2DMakeIdentity() {
	return Transform2D { .sx = 1, .sy = 1 };
}
