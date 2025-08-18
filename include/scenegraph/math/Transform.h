#pragma once

struct Transform {
	float sx, sy, sz;
	float radX, radY, radZ;
	float tx, ty, tz;
};

inline Transform TransformMakeZero() {
	return Transform{};
}

inline Transform TransformMakeIdentity() {
	return Transform { .sx = 1, .sy = 1, .sz = 1 };
}
