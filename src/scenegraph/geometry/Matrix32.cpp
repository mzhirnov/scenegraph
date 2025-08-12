#include <scenegraph/geometry/Matrix32.h>
#include <scenegraph/utils/FloatUtils.h>

Matrix32 Matrix32MakeRotation(float rad) {
	float s = std::sinf(rad);
	float c = std::cosf(rad);
	
	return Matrix32 {
		 c, s,
		-s, c,
		 0, 0
	};
}

Matrix32 Matrix32MakeWithTransform2D(const Transform2D& c) {
	constexpr auto epsilon = std::numeric_limits<float>::epsilon();
	if (c.shearX > epsilon || c.shearY > epsilon) {
		auto m =
			Matrix32MultiplyRotation(
				Matrix32MultiplyRotation(
					Matrix32MakeScale(c.sx, c.sy),
					Matrix32MultiplyRotation(
						Matrix32MakeXShear(c.shearX),
						Matrix32MakeYShear(c.shearY))),
				Matrix32MakeRotation(c.rad));
		m.tx = c.tx;
		m.ty = c.ty;
		return m;
	}
	else {
		float sn = std::sinf(c.rad);
		float cs = std::cosf(c.rad);
		
		return Matrix32 {
			 cs * c.sx, sn * c.sx,
			-sn * c.sy, cs * c.sy,
			 c.tx, c.ty
		};
	}
}

bool Matrix32DecomposeToTransform2D(const Matrix32& m, Transform2D* transform) {
	float det = DifferenceOfProducts(m.a, m.d, m.b, m.c);
	if (det <= std::numeric_limits<float>::epsilon()) {
		*transform = Transform2DMakeZero();
		return false;
	}
	
	transform->sx = std::sqrtf(m.a * m.a + m.b * m.b);
	transform->sy = det / transform->sx;
	transform->shearX = (m.a * m.c + m.b * m.d) / det;
	transform->shearY = 0;
	transform->rad = std::atan2(m.b, m.a);
	transform->tx = m.tx;
	transform->ty = m.ty;
	
	return true;
}

Matrix32 Matrix32Scale(const Matrix32& m, float s) {
	return Matrix32 {
		m.a * s, m.b * s,
		m.c * s, m.d * s,
		m.tx * s, m.ty * s
	};
}

Matrix32 Matrix32Add(const Matrix32& m1, const Matrix32& m2) {
	return Matrix32 {
		m1.a + m2.a, m1.b + m2.b,
		m1.c + m2.c, m1.d + m2.d,
		m1.tx + m2.tx, m1.ty + m2.ty
	};
}

Matrix32 Matrix32Subtract(const Matrix32& m1, const Matrix32& m2) {
	return Matrix32 {
		m1.a - m2.a, m1.b - m2.b,
		m1.c - m2.c, m1.d - m2.d,
		m1.tx - m2.tx, m1.ty - m2.ty
	};
}

Matrix32 Matrix32Multiply(const Matrix32& m1, const Matrix32& m2) {
	return Matrix32 {
		m1.a * m2.a + m1.b * m2.c,
		m1.a * m2.b + m1.b * m2.d,
		
		m1.c * m2.a + m1.d * m2.c,
		m1.c * m2.b + m1.d * m2.d,
		
		m1.tx * m2.a + m1.ty * m2.c + m2.tx,
		m1.tx * m2.b + m1.ty * m2.d + m2.ty
	};
}

Matrix32 Matrix32MultiplyRotation(const Matrix32& m1, const Matrix32& m2) {
	return Matrix32 {
		m1.a * m2.a + m1.b * m2.c,
		m1.a * m2.b + m1.b * m2.d,
		
		m1.c * m2.a + m1.d * m2.c,
		m1.c * m2.b + m1.d * m2.d,
		
		0,
		0
	};
}

Matrix32 Matrix32Invert(const Matrix32& m, bool* invertible) {
	float det = DifferenceOfProducts(m.a, m.d, m.b, m.c);
	
	if (invertible && !(*invertible = det > std::numeric_limits<float>::epsilon())) {
		return Matrix32MakeZero();
	}
	
	det = 1.0f / det;
	
	return Matrix32 {
		 det * m.d,
		-det * m.b,
		
		-det * m.c,
		 det * m.a,
		 
		 det * DifferenceOfProducts(m.c, m.ty, m.d, m.tx),
		 det * DifferenceOfProducts(m.b, m.tx, m.a, m.ty)
	};
}
