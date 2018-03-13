inline quat
kh_quat(f32 x, f32 y, f32 z, f32 w) {
	quat res;
	res.x = x;
	res.y = y;
	res.z = z;
	res.w = w;
	return(res);
}

inline quat
quat_identity() {
	quat res;
	res.n = V3_ZERO;
	res.w = 1.0f;
	return(res);
}

inline quat
construct_quat(v3 n, f32 a) {
	quat res;
	a = a * 0.5f;
	f32 w = kh_cos_f32(a);
	f32 s = kh_sin_f32(a);
	res.n = n * s;
	res.w = w;
	return(res);
}

inline quat
operator*(quat a, quat b) {
	quat res;

	res.x = a.w*b.x + b.w*a.x + a.y*b.z - a.z*b.y;
	res.y = a.w*b.y + b.w*a.y + a.z*b.x - a.x*b.z;
	res.z = a.w*b.z + b.w*a.z + a.x*b.y - a.y*b.x;
	res.w = a.w*b.w - (a.x*b.x + a.y*b.y + a.z*b.z);

	// res.n = a.w*b.n + b.w*a.n + cross(a.n, b.n);
	// res.w = a.w * b.w - dot(a.n, b.n);
	return(res);
}

inline quat
operator*(quat a, f32 b) {
	quat res;
	res.n = a.n*b;
	res.w = a.w*b;
	return(res);
}

inline quat
operator*(f32 a, quat b) {
	quat res = b * a;
	return(res);
}

inline quat
operator+(quat a, quat b) {
	quat res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	res.w = a.w + b.w;
	return(res);
}

inline quat
operator-(quat a, quat b) {
	quat res;
	res.x = a.x - b.x;
	res.y = a.y - b.y;
	res.z = a.z - b.z;
	res.w = a.w - b.w;
	return(res);
}

inline quat
negate(quat a) {
	quat res;
	res.n = -a.n;
	res.w = -a.w;
	return(res);
}

inline quat
conjugate(quat a) {
	quat res;
	res.n = a.n;
	res.w = -a.w;
	return(res);
}

inline f32
dot(quat a, quat b) {
	f32 res = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	return(res);
}

inline f32
length(quat a) {
	f32 res = kh_sqrt_f32(dot(a, a));
	return(res);
}

inline quat
difference(quat a, quat b) {
	quat res = b * conjugate(a);
	return(res);
}

inline quat
normalize(quat a) {
	f32 il = 1.0f / length(a);
	quat res = a * il;
	return(res);
}

inline f32
normalize_or_identity(quat *a) {
	f32 sq = kh_sqrt_f32(a->x*a->x + a->y*a->y + a->z*a->z + a->w*a->w);
	if(sq == 0.0f) {
		a->w = 1;
	} else {
		f32 factor = 1.0f / sq;
		a->x *= factor;
		a->y *= factor;
		a->z *= factor;
		a->w *= factor;
	}

	return(sq);
}

inline quat 
kh_mix_quat(quat a, quat b, f32 f, f32 t) {
	quat res;
	res.x = f * a.x + t * b.x;
	res.y = f * a.y + t * b.y;
	res.z = f * a.z + t * b.z;
	res.w = f * a.w + t * b.w;
	return(res);
}

inline quat
kh_mix_quat(quat a, quat b, f32 t) {
	quat res = kh_mix_quat(a, b, 1.0f - t, t);
	return(res);
}

inline v3
rotate(v3 v, quat q) {
	// quat p;
	// p.n = v;
	// p.w = 0;
	// v3 res = (q * (p * conjugate(q)).n;

	v3 q_cross_v = kh_cross_v3(q.n, v);
	v3 res = v + q_cross_v*(2.0f * q.w) + kh_cross_v3(q.n, q_cross_v)*2.0f;

	return(res);
}

#if 0
inline quat
slerp(quat a, quat b, f32 t)
{
	f32 d = dot(a, b);

	d = kh_clamp_f32(-1.0f, 1.0f, d);
	f32 o = kh_acos_f32(d) * t;

	quat proj = a * d;
	quat perp = b - proj;
	perp = normalize(perp);

	quat res = kh_cos_f32(o)*a + kh_sin_f32(o)*perp;


	// f32 is = 1.0f / kh_sin_f32(o);
	// f32 st = kh_sin_f32(t*o);
	// f32 sit = kh_sin_f32((1-t)*o);

	// quat res = (sit*is)*a + (st*is)*b;
	return(res);

}
#endif 

inline quat
kh_slerp_quat(quat a, quat b, f32 t) {
	quat res;

	f64 d = dot(a, b);
	if(kh_abs_f64(d) >= 1.0f) {
		res = a;
		return(res);
	}

	f64 o = kh_acos_f32(d);
	f64 s = kh_sqrt_f32(1.0f - d*d);

	if(s < 0.001f) {
		res.x = 0.5f * a.x + 0.5f * b.x;
		res.y = 0.5f * a.y + 0.5f * b.y;
		res.z = 0.5f * a.z + 0.5f * b.z;
		res.w = 0.5f * a.w + 0.5f * b.w;
		return(res);
	} 

	f64 inv = 1.0f / s;
	f64 ra = kh_sin_f32((1.0f - t) * o) * inv;
	f64 rb = kh_sin_f32(t * o) * inv;

	res.x = ra * a.x + rb * b.x; 
	res.y = ra * a.y + rb * b.y; 
	res.z = ra * a.z + rb * b.z; 
	res.w = ra * a.w + rb * b.w; 
	return(res);
}

inline v3
slerp(v3 a, v3 b, f32 t) {
	f32 d = kh_dot_v3(a, b);

	d = kh_clamp_f32(-1.0f, 1.0f, d);
	f32 o = kh_acos_f32(d) * t;

	v3 proj = a * d;
	v3 perp = b - perp;
	perp = kh_normalize_v3(perp);

	v3 res = kh_cos_f32(o)*a + kh_sin_f32(o)*perp;

	// f32 is = 1.0f / kh_sin_f32(o);
	// f32 st = kh_sin_f32(t*o);
	// f32 sit = kh_sin_f32((1-t)*o);

	// quat res = (sit*is)*a + (st*is)*b;
	return(res);

}

inline quat
kh_nlerp_quat(quat a, quat b, f32 t) {
	quat res = kh_mix_quat(a, b, t);
	normalize_or_identity(&res);

	return(res); 
}

// http://zeuxcg.org/2015/07/23/approximating-slerp/
// http://zeuxcg.org/2016/05/05/optimizing-slerp/
// NOTE(flo): this is for constant velocity in our interpolation
inline quat
correct_nlerp(quat a, quat b, f32 t) {
	f32 dt = dot(a, b);
	f32 d = kh_abs_f32(dt);
	f32 k = d * (d * 0.167127f - 0.631176f) + 0.461218f;
    f32 ot = t * (t * (2 * t - 3) * k + k + 1);

	f32 lt = 1.0f - ot;
	f32 rt = (dt > 0) ? ot : -ot;

	quat res = normalize(kh_mix_quat(a, b, lt, rt));
	return(res);

}