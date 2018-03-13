#define V3_ZERO kh_vec3(0,0,0)
#define V3_ONE kh_vec3(1,1,1)
#define V3_RIGHT kh_vec3(1,0,0)
#define V3_UP kh_vec3(0,1,0)
#define V3_FORWARD kh_vec3(0,0,1)

inline v3
kh_vec3(f32 x, f32 y, f32 z) {
	v3 res = { x, y, z };
	return(res);
}

inline v3
kh_vec3(v2 xy, f32 z) {
	v3 res;
	res.x = xy.x;
	res.y = xy.y;
	res.z = z;
	return(res);
}

inline v3
kh_vec3(v4 a) {
	v3 res;
	res.x = a.x;
	res.y = a.y;
	res.z = a.z;
	return(res);
}

inline v3
kh_vec3(f32 a) {
	v3 res;
	res.x = a;
	res.y = a;
	res.z = a;
	return(res);
}

inline v3
operator+(v3 a, v3 b) {
	v3 res = { a.x + b.x, a.y + b.y, a.z + b.z };
	return(res);
}

inline v3 &
operator+=(v3 &a, v3 b) {
	a = a + b;
	return(a);
}

inline v3
operator-(v3 a) {
	v3 res = { -a.x, -a.y, -a.z };
	return(res);
}

inline v3
operator-(v3 a, v3 b) {
	v3 res = { a.x - b.x, a.y - b.y, a.z - b.z };
	return(res);
}

inline v3 &
operator-=(v3 &a, v3 b) {
	a = a - b;
	return(a);
}

inline v3
operator*(f32 a, v3 b) {
	v3 res = { a * b.x, a * b.y, a * b.z };
	return(res);
}

inline v3
operator*(v3 a, f32 b) {
	v3 res = b*a;
	return(res);
}

inline v3
operator/(v3 a, f32 b) {
	// TODO(flo): figure out if we loose too much precision here in some cases!
	f32 one_over_b = 1.0f / b;
	v3 res = a * one_over_b;
	return(res);
}

inline v3 &
operator*=(v3 &a, f32 b) {
	a = b * a;
	return(a);
}

inline b32
operator==(v3 a, v3 b) {
	b32 res = ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
	return(res);
}

inline b32
operator!=(v3 a, v3 b) {
	b32 res = ((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
	return(res);
}

inline v3
kh_hadamard_v3(v3 a, v3 b) {
	v3 res;
	res.x = a.x * b.x;
	res.y = a.y * b.y;
	res.z = a.z * b.z;
	return(res);
}

inline v3
kh_divide_v3(v3 a, v3 b) {
	v3 res;
	res.x = a.x / b.x;
	res.y = a.y / b.y;
	res.z = a.z / b.z;
	return(res);
}

inline f32
kh_dot_v3(v3 a, v3 b) {
	f32 res = a.x*b.x + a.y*b.y + a.z*b.z;
	return(res);
}

inline f32
kh_lensqr_v3(v3 a) {
	f32 res = kh_dot_v3(a, a);
	return(res);
}

inline f32
kh_length_v3(v3 a) {
	f32 res = kh_sqrt_f32(kh_lensqr_v3(a));
	return(res);
}

inline v3
kh_sqrt_v3(v3 a) {
	v3 res;
	res.x = kh_sqrt_f32(a.x);
	res.y = kh_sqrt_f32(a.y);
	res.z = kh_sqrt_f32(a.z);
	return(res);
}

inline v3
kh_normalize_v3(v3 a) {
	f32 invlen = kh_safe_ratio0_f32(1.0f, kh_length_v3(a));
	v3 res = invlen * a;
	return(res);
}

inline v3
kh_mix_imprecise_v3(v3 a, v3 b, f32 t) {
	v3 res = a + t*(b - a);
	return(res);
}

inline v3
kh_mix_v3(v3 a, v3 b, f32 t) {
	v3 res = (1.0f - t)*a + t*b;
	return(res);
}

inline v3
kh_cross_v3(v3 a, v3 b) {
	v3 res;
	res.x = a.y * b.z - a.z * b.y;
	res.y = a.z * b.x - a.x * b.z;
	res.z = a.x * b.y - a.y * b.x;
	return(res);
}

inline f32
kh_angle_v3(v3 a, v3 b) {
	f32 c = kh_dot_v3(a, b) / (kh_length_v3(a)*kh_length_v3(b));
	f32 res = kh_acos_f32(c);
	return(res);
}

inline f32
kh_norm_angle_v3(v3 a, v3 b) {
	f32 c = kh_dot_v3(a, b);
	f32 res = kh_acos_f32(c);
	return(res);
}

inline v3
kh_orth_proj_v3(v3 a, v3 b) {
	f32 b_length_sqrt = kh_lensqr_v3(b);
	v3 res = (kh_dot_v3(a, b) / b_length_sqrt) * b;
	return(res);
}

inline v3
kh_reflect_v3(v3 a, v3 b) {
	v3 proj = kh_orth_proj_v3(a, b);
	v3 res = a - 2*proj;
	return(res);
}

inline v3
kh_reject_v3(v3 a, v3 b) {
	v3 proj = kh_orth_proj_v3(a, b);
	v3 res = a - proj;
	return(res);
}

inline v3
kh_norm_orth_proj_v3(v3 a, v3 b) {
	v3 res = kh_dot_v3(a, b)*b;
	return(res);
}

inline v3
kh_norm_reflect_v3(v3 a, v3 b) {
	v3 proj = kh_norm_orth_proj_v3(a, b);
	v3 res = a - 2*proj;
	return(res);
}

inline v3
kh_norm_reject_v3(v3 a, v3 b) {
	v3 proj = kh_norm_orth_proj_v3(a, b);
	v3 res = a - proj;
	return(res);
}

inline v3
kh_quad_bezier_v3(v3 a, v3 b, v3 c, f32 t) {
	v3 d = kh_mix_v3(a, b, t);
	v3 e = kh_mix_v3(b, c, t);

	v3 res = kh_mix_v3(d, e, t);
	return(res);
}

inline v3
kh_cubic_bezier_v3(v3 a, v3 b, v3 c, v3 d, f32 t) {
	v3 e = kh_mix_v3(a, b, t);
	v3 f = kh_mix_v3(b, c, t);
	v3 g = kh_mix_v3(c, d, t);

	v3 res = kh_quad_bezier_v3(e, f, g, t);
	return(res);
}

inline v3
kh_quintic_bezier_v3(v3 a, v3 b, v3 c, v3 d, v3 e, f32 t) {
	v3 f = kh_mix_v3(a, b, t);
	v3 g = kh_mix_v3(b, c, t);
	v3 h = kh_mix_v3(c, d, t);
	v3 i = kh_mix_v3(d, e, t);
	v3 res = kh_cubic_bezier_v3(f, g, h, i, t);
	return(res);
}

inline v3
kh_cubic_bspline10_v3(v3 *cp, f32 t) {
	if(t <= 0.0f)
		return cp[0];

	if(t >= 1.0f)
		return cp[9];

	f32 f = t * 3.0f;
	int i = kh_floor_f32_to_i32(f);
	f32 s = kh_fract_f32(t);

	v3 a = cp[i * 3 + 0];
	v3 b = cp[i * 3 + 1];
	v3 c = cp[i * 3 + 2];
	v3 d = cp[i * 3 + 3];

	v3 res = kh_cubic_bezier_v3(a, b, c, d, s);
	return(res);
}

inline v3
kh_vert_tangent_from_uv(v3 pos_0, v3 pos_1, v3 pos_2, v2 uv_0, v2 uv_1, v2 uv_2) {
	v3 res;

	float du1 = uv_1.x - uv_0.x;
	float dv1 = uv_1.y - uv_1.y;
	float du2 = uv_2.x - uv_0.x;
	float dv2 = uv_2.y - uv_0.y;

	v3 e1 = pos_1 - pos_0;
	v3 e2 = pos_2 - pos_0;

	float r = 1.0f / ((du1 * dv2) - (du2 * dv1));

	res = ((e1 * dv2) - (e2 * dv2)) * r;

	// v3 bitangent = ((e2 * du1) - (e1 * du2)) * r;

	return(res);
}

inline v3
kh_clamp01_v3(v3 a) {
	v3 res;
	res.x = kh_clamp01_f32(a.x);
	res.y = kh_clamp01_f32(a.y);
	res.z = kh_clamp01_f32(a.z);
	return(res);
}