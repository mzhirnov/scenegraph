#pragma once

struct Transform2D {
	float sx, sy;
	float shearX, shearY;
	float rad;
	float tx, ty;
};

constexpr Transform2D Transform2DMakeZero() { return {}; }
constexpr Transform2D Transform2DMakeIdentity() { return { .sx = 1, .sy = 1 }; }
