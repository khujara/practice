#define V2_ZERO kh_vec2(0,0)
#define V2_ONE kh_vec2(0,0)
#define V2_RIGHT kh_vec2(1,0)
#define V2_UP kh_vec2(0,1)

inline v2
kh_vec2(f32 x, f32 y)
{
	v2 res = {x, y};
	return(res);
}

inline v2
kh_vec2(i32 x, i32 y)
{
	v2 res = {(f32)x, (f32)y};
	return(res);
}

inline v2
operator+(v2 a, v2 b)
{
	v2 res = { a.x + b.x, a.y + b.y };
	return(res);
}

inline v2 &
operator+=(v2 &a, v2 b)
{
	a = a + b;
	return(a);
}

inline v2
operator-(v2 a)
{
	v2 res = { -a.x, -a.y};
	return(res);
}

inline v2
operator-(v2 a, v2 b)
{
	v2 res = { a.x - b.x, a.y - b.y };
	return(res);
}

inline v2 &
operator-=(v2 &a, v2 b)
{
	a = a - b;

	return(a);
}

inline v2
operator*(f32 a, v2 b)
{
	v2 res = { a * b.x, a * b.y };
	return(res);
}

inline v2
operator*(v2 a, f32 b)
{
	v2 res = b * a;
	return(res);
}

inline v2 &
operator*=(v2 &a, f32 b)
{
	a = b * a;
	return(a);
}

inline v2
operator/(v2 a, f32 b)
{
	// TODO(flo): figure out if we loose too much precision here in some cases!
	f32 one_over_b = 1.0f / b;
	v2 res = a * one_over_b;
	return(res);
}

inline b32
operator==(v2 a, v2 b)
{
	b32 res = ((a.x == b.x) && (a.y == b.y));
	return(res);
}

inline b32
operator!=(v2 a, v2 b)
{
	b32 res = ((a.x != b.x) || (a.y != b.y));
	return(res);
}

inline f32
kh_dot_v2(v2 a, v2 b)
{
	f32 res = a.x*b.x + a.y*b.y;
	return(res);
}

inline f32
kh_lensqr_v2(v2 a)
{
	f32 res = kh_dot_v2(a, a);
	return(res);
}

inline v2
kh_perp_v2(v2 a)
{
	v2 res;
	res.x = -a.y;
	res.y = a.x;
	return(res);
}

inline f32
kh_len_v2(v2 a)
{
	f32 res = kh_sqrt_f32(kh_lensqr_v2(a));
	return(res);
}
