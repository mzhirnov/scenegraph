#pragma once

struct Transform {
	float sx, sy, sz;
	float radX, radY, radZ;
	float tx, ty, tz;
};

constexpr Transform TransformMakeZero() {return {}; }
constexpr Transform TransformMakeIdentity() { return { .sx = 1, .sy = 1, .sz = 1 }; }
