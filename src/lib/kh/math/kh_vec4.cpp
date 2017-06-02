#define V4_ZERO kh_vec4(0,0,0,0)
#define V4_ONE kh_vec4(1,1,1,1)
#define COLOR_RED kh_vec4(1,0,0,1)
#define COLOR_GREEN kh_vec4(0,1,0,1)
#define COLOR_BLUE kh_vec4(0,0,1,1)
#define COLOR_BLACK kh_vec4(0,0,0,1)
#define COLOR_WHITE V4_ONE

inline v4
kh_vec4(f32 x, f32 y, f32 z, f32 w)
{
	v4 res = { x, y, z, w };
	return(res);
}

inline v4
kh_vec4(v3 xyz, f32 w)
{
	v4 res;
	res.x = xyz.x;
	res.y = xyz.y;
	res.z = xyz.z;
	res.w = w;
	return(res);
}


inline v4
operator*(f32 a, v4 b)
{
	v4 res = { a * b.x, a * b.y, a * b.z, a * b.w };
	return(res);
}

inline v4
operator+(v4 a, v4 b)
{
	v4 res = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
	return(res);
}

inline v4 &
operator+=(v4 &a, v4 b)
{
	a = a + b;
	return(a);
}

inline v4
operator-(v4 a)
{
	v4 res = { -a.x, -a.y, -a.z, -a.w };
	return(res);
}

inline v4
operator-(v4 a, v4 b)
{
	v4 res = { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
	return(res);
}

inline v4 &
operator-=(v4 &a, v4 b)
{
	a = a - b;
	return(a);
}

inline v4
operator*(v4 a, f32 b)
{
	v4 res = b*a;
	return(res);
}

inline v4 &
operator*=(v4 &a, f32 b)
{
	a = b * a;
	return(a);
}

inline v4
operator/(v4 a, f32 b)
{
	// TODO(flo): figure out if we loose too much precision here in some cases!
	f32 one_over_b = 1.0f / b;
	v4 res = a * one_over_b;
	return(res);
}

inline b32
operator==(v4 a, v4 b)
{
	b32 res = ((a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w));
	return(res);
}

inline b32
operator!=(v4 a, v4 b)
{
	b32 res = ((a.x != b.x) ||
		(a.y != b.y) ||
		(a.z != b.z) ||
		(a.w != b.w));
	return(res);
}

inline f32
kh_dot_v4(v4 a, v4 b)
{
	f32 res = a.x*b.x + a.y*b.y + a.z*b.z;
	return(res);
}
