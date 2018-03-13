#ifndef KH_MATHDEF_H
// TODO(flo): remove this math library once everything is implemented
#include "math.h"
#if defined(_MSC_VER)
#include <intrin.h>
#endif
typedef union v2 {
	f32 e[2];
	struct {
		f32 x, y;
	};
} v2;

typedef union v3 {
	f32 e[3];
	struct {
		f32 x,y,z;
	};
	struct {
		f32 r,g,b;
	};
} v3;

typedef union v4 {
	f32 e[4];
	struct {
		f32 x,y,z,w;
	};
	struct {
		f32 r,g,b,a;
	};
} v4;

typedef union v2i {
	struct {
		i32 x, y;
	};
	i32 e[2];
} v2i;

typedef union v4i {
	struct {
		i32 a, b, c, d;
	};
	struct {
		i32 r, g, b, a;
	};
	struct {
		i32 x, y, z, w;
	};
	i32 e[4];
} v4i;

// #define mat3 float[9]
// #define mat4 float[16]

typedef union mat3 {
	struct {
		v3 c0;
		v3 c1;
		v3 c2;
	};
	struct {
		f32 m00, m01, m02, m10, m11, m12, m20, m21, m22;
	};
	// NOTE(flo): we are in column major, c multidimensional arrays are in row major
	//f32 E[3][3];
	f32 e[9];
} mat3;

typedef union mat4 {
	struct {
		v4 c0;
		v4 c1;
		v4 c2;
		v4 c3;
	};
	struct {
		f32 m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33;
	};
	// NOTE(flo): we are in column major, c multidimensional arrays are in row major
	// f32 E[4][4];
	f32 e[16];
} mat4;


typedef union quat {
	struct {
		f32 x, y, z , w;
	};
	struct {
		v3 n;
		f32 w;
	};
	f32 e[4];
} quat;

typedef struct EulerAngle
{
	// NOTE(flo): y_axis rotation
	f32 pitch;
	// NOTE(flo): z_axis rotation
	f32 yaw;
	// NOTE(flo): x_axis rotation
	f32 roll;
} EulerAngle;

typedef struct Rect {
	v2 min, max;
} Rect;

typedef struct Recti {
	i32 min_x, max_x;
	i32 min_y, max_y;
} Recti;

typedef struct AABB {
	v3 min, max;
} AABB;

typedef struct Ray {
	v3 origin;
	v3 dir;
} Ray;

typedef struct Ray2D {
	v2 orig, dir;
} Ray2D;

typedef struct Line {
	v3 start, end;
} Line;

typedef struct Line2D {
	v2 start, end;
} Line2D;

typedef struct Triangle {
	// v3 a, b, c;
	v3 v_0, v_1, v_2;
} Triangle;

typedef struct Circle {
	v2 center;
	f32 radius;
} Circle;

typedef struct Sphere {
	v3 center;
	f32 radius;
} Sphere;

typedef struct Polygon3D {
	i32 count;
	v3 *pos;
} Polygon3D;

typedef struct Polygon2D {
	i32 count;
	v2 *pos;
} Polygon2D;

typedef struct Plane {
	v3 normal;
	f32 d;
} Plane;

typedef struct BitScan {
	b32 found;
	u32 index;
}BitScan;

typedef struct Transform_SQT {
	v3 pos;
	quat rot;
	v3 scale;
}Transform_SQT;

#define PI32 3.14159265354897932384626433837959f
#define INV_PI32 0.31830988618f 
#define TAU32 6.283185307179586476925286766559f
#define TWO_PI32 TAU32
#define INV_TAU32 0.15915494309f
#define INV_TWO_PI32 INV_TAU32
#define E32 2.71828182845904523536028747135266249f
#define PIOVER360 0.00872664625997164788461845384244f
#define TO_DEGREES 57.2957795131f
#define TO_RADIANS 0.0174532925f
#define ONE_OVER_255 0.0039215686274509803921568627451f

#define COS_15 0.9659258262890682867497431997289f
#define SIN_15 0.2588190451025207623489f
#define COS_30 0.86602540378443864676f
#define SIN_30 0.5f
#define COS_45 0.7071067811865475244f
#define SIN_45 COS_45
#define COS_60 0.5f
#define SIN_60 COS_30
#define COS_75 SIN_15
#define SIN_75 COS_15
#define COS_90 0.0f;
#define SIN_90 1.0f;

typedef v4 ColorRGBA;
typedef v3 ColorRGB;

// TODO(flo): we need to define our math functions here
KH_INLINE mat4 from_quat_to_mat4(quat rot);

KH_INLINE v2i
kh_vec2i(u32 x, u32 y) {
	v2i res;
	res.x = x;
	res.y = y;
	return(res);
}

#define KH_MATHDEF_H
#endif

#ifdef KH_MATH_IMPLEMENTATION
#include "math/kh_u32_64.cpp"
#include "math/kh_i32_64.cpp"
#include "math/kh_f32_64.cpp"
#include "math/kh_intrin.cpp"
#include "math/kh_vec2.cpp"
#include "math/kh_vec3.cpp"
#include "math/kh_vec4.cpp"
#include "math/kh_quat.cpp"
#include "math/kh_mat33.cpp"
#include "math/kh_mat44.cpp"
#include "math/kh_transform.cpp"
#include "math/kh_euler_angle.cpp"
#include "math/kh_geometry.cpp"
#include "math/kh_intersection.cpp"

inline v3
kh_spherical_dir(f32 cos_theta, f32 sin_theta, f32 cos_phi, f32 sin_phi) {
	v3 res;
	res.x = sin_theta * cos_phi;
	res.y = sin_theta * sin_phi;
	res.z = cos_theta;
	return(res);
}

inline v3
kh_spherical_dir(f32 cos_theta, f32 sin_theta, f32 phi) {
	v3 res = kh_spherical_dir(cos_theta, sin_theta, kh_cos_f32(phi), kh_sin_f32(phi));
	return(res);
}

inline v3
kh_spherical_dir(f32 theta, f32 phi) {
	f32 cos_t = kh_cos_f32(theta);
	f32 sin_t = kh_sin_f32(theta);
	f32 cos_p = kh_cos_f32(phi);
	f32 sin_p = kh_sin_f32(phi);
	v3 res = kh_spherical_dir(cos_t, sin_t, cos_p, sin_p);
	return(res);
}

inline v3
kh_spherical_dir(f32 cos_t, f32 sin_t, f32 cos_p, f32 sin_p, v3 x_axis, v3 y_axis, v3 z_axis) {
	v3 res = sin_t * cos_p * x_axis + sin_t * sin_p * y_axis + cos_t * z_axis;
	return(res);
}

inline f32
kh_spherical_theta(v3 v) {
	f32 res = kh_acos_f32(kh_clamp_f32(-1.0f, 1.0f, v.z));
	return(res);
}

inline f32
kh_spherical_phi(v3 v) {
	f32 p = kh_atan2_f32(v.y, v.x);
	f32 res = (p < 0) ? (p + 2 * PI32) : p;
	return(res); 
}

#endif //KH_INCLUDE_MATH_IMPLEMENTATION

#if 0
inline quat
to_axis_angle(quat a)
{
	quat res = {};
	f32 s = square_root(1.0f - a.w*a.w);
	f32 s_inv = safe_ratio1(1.0f, s);
	res.xyz = a.xyz * s_inv;
	res.w = 2 * inverse_cosine(a.w);
	return(res);
}

// NOTE(flo): cf http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/
inline quat
operator^(quat a, f32 t)
{
	quat res = {};
	res = to_axis_angle(a);
	res.w *= t;
	return(res);
}


/*
https://www.youtube.com/watch?v=fRSaaLtYj68&index=34&list=PLqBpFJy2aB0BHuwa_FVGzrO5df-5EfkKH
http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/

https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
https://en.wikipedia.org/wiki/Slerp
http://www.gamedev.net/page/resources/_/technical/math-and-physics/do-we-really-need-quaternions-r1199
http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/

*/


// https://en.wikipedia.org/wiki/Slerp
inline v3
slerp(v3 start, v3 end, f32 t)
{
	f32 dot_prod = dot(start, end);
	dot_prod = kh_clamp(-1.0f, 1.0f, dot_prod);
	f32 omega = inverse_cosine(dot_prod) * t;
	v3 relative_v = normalize(end - start*dot_prod);
	v3 res = (start * kh_cos_f32(omega)) + (relative_v*kh_sin_f32(omega));

		// More precise technique ??
	// source : https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
	// f32 DotProduct = Dot(start, end);
	// DotProduct = Clamp(-1.0f, 1.0f, DotProduct);
	// f32 Omega = Acos(DotProduct);
	// NOTE(flo) : if(Angle == 0) Sin(Omega == 0)
	// f32 InverseSinOmega = SafeRation1(1.0f / Sin(Omega));
	// v3 res = Sin((1-t)*Omega) * InverseOmega * start + Sin(t*Omega) * InverseOmega * end;
	return(res);
}

inline v3
nlerp(v3 start, v3 end, f32 t)
{
	v3 res = normalize(linear_blend(start, end, t));
	return(res);
}
#endif