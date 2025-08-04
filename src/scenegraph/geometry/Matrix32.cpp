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

Matrix32 Matrix32MakeWithComponents(const Matrix32Components& c) {
	if (c.shear > std::numeric_limits<float>::epsilon()) {
		auto m = Matrix32MultiplyRotation(
			Matrix32MakeScaleAndShear(c.sx, c.sy, c.shear),
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

Matrix32Components Matrix32DecomposeToComponents(const Matrix32& m, bool* invertible) {
	float det = DifferenceOfProducts(m.a, m.d, m.b, m.c);
	if (invertible && !(*invertible = det > std::numeric_limits<float>::epsilon())) {
		return Matrix32ComponentsMakeZero();
	}
	
	Matrix32Components out;
	
	out.sx = std::sqrtf(m.a * m.a + m.b * m.b);
	out.sy = det / out.sx;
	out.shear = (m.a * m.c + m.b * m.d) / det;
	out.rad = std::atan2(m.b, m.a);
	out.tx = m.tx;
	out.ty = m.ty;
	
	return out;
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
