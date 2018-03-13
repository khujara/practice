inline f32
kh_mod_f32(f32 a, f32 b) {
	f32 res = fmodf(a, b);
	return(res);
}

inline f32
kh_signof_f32(f32 val) {
		f32 res = (val >= 0.0f) ? 1.0f : -1.0f;
		return(res);
}

inline f32
kh_rsqrt_f32(f32 val) {
	f32 res = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(val)));
	return(res);
}

inline f32
kh_sqrt_f32(f32 val) {
	f32 res = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(val)));
	return(res);
}

inline f32
kh_sin_f32(f32 angle) {
	f32 res = sinf(angle);
	return(res);
}

inline f32
kh_cos_f32(f32 angle) {
	f32 res = cosf(angle);
	return(res);
}

inline f32
kh_tan_f32(f32 angle) {
  f32 res = tanf(angle);
  return(res);
}

inline f32
kh_atan2_f32(f32 y, f32 x) {
	f32 res = atan2f(y, x);
	return(res);
}

inline f32
kh_asin_f32(f32 val) {
	f32 res = asinf(val);
	return(res);
}

inline f32
kh_acos_f32(f32 val) {
	f32 res = acosf(val);
	return(res);
}

inline f32
kh_pow_f32(f32 a, f32 b) {
	f32 res = powf(a, b);
	return(res);
}

inline f32
kh_exp_f32(f32 a) {
	f32 res = expf(a);
	return(res);
}

inline f32
kh_sqr_f32(f32 a) {
	f32 res = a * a;
	return(res);
}

inline f32
kh_mix_f32(f32 a, f32 b, f32 t) {
	f32 res = (1.0f - t)*a + t*b;
	return(res);
}

inline f32
kh_mix_imprecise_f32(f32 a, f32 b, f32 t) {
	f32 res = a + t*(b - a);
	return(res);
}

inline f32
kh_clamp_f32(f32 min, f32 max, f32 val) {
	f32 res = val;
	if(res < min) {
		res = min;
	}
	if(res > max) {
		res = max;
	}
	return(res);
}

inline f32
kh_clamp01_f32(f32 val) {
	f32 res = kh_clamp_f32(0.0f, 1.0f, val);
	return(res);
}

inline f32
kh_remap_f32(f32 min, f32 max, f32 t, f32 remap_min, f32 remap_max) {
	f32 res = 0;
	f32 range = max - min;
	if(range > 0) {
		f32 normalized = (t - min) / range;
		res = remap_min + normalized*(remap_max - remap_min);
	}
	return(res);
}

inline f32
kh_remap_safe(f32 min, f32 max, f32 t, f32 remap_min, f32 remap_max) {
	f32 res = remap_min + (((t - min) / (max -min)) * (remap_max - remap_min));
	return(res);
}

inline f32
kh_remap_range(f32 range, f32 t, f32 remap_range) {
	f32 res = (t / range) * remap_range;
	return(res);
}

inline f32
kh_remap01_f32(f32 min, f32 max, f32 t) {
	f32 res = 0;
	f32 range = max - min;
	if(range > 0) {
		res = (t - min) / range;
	}
	return(res);
}

inline f32
kh_remap_clamp_f32(f32 min, f32 max, f32 t, f32 remap_min, f32 remap_max) {
	f32 res = 0;
	if(t < min) {
		res = remap_min;
	} else if(t > max) {
		res = remap_max;
	} else {
		res = kh_remap_f32(min, max, t, remap_min, remap_max);
	}
}

inline f32
kh_remap_clamp01_f32(f32 min, f32 max, f32 t) {
	f32 res = 0.0f;
	f32 Range = max - min;
	if(Range != 0.0f) {
		res = kh_clamp01_f32((t - min) / Range);
	}
	return(res);
}

inline f32
kh_abs_f32(f32 val) {
	f32 res = fabsf(val);
	return(res);
}

inline f32
kh_min_f32(f32 a, f32 b) {
  f32 res = (a < b) ? a : b;
  // f32 res = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
  return(res);
}

inline f32
kh_max_f32(f32 a, f32 b) {
  f32 res = (a > b) ? a : b;
  return(res);
}

inline f32
kh_min2_f32(f32 a, f32 b) {
	f32 res = 0.5f * (a + b - kh_abs_f32(a - b));
	return(res);
}

inline f32
kh_max2_f32(f32 a, f32 b) {
	f32 res = 0.5f * (a + b + kh_abs_f32(a - b));
	return(res);
}

inline f32
kh_safe_ratioN_f32(f32 numerator, f32 divisor, f32 n) {
	f32 res = n;
	if(divisor != 0.0f)	{
		res = numerator / divisor;
	}
	return(res);
}

inline f32
kh_safe_ratio0_f32(f32 numerator, f32 divisor) {
	f32 res = kh_safe_ratioN_f32(numerator, divisor, 0.0f);
	return(res);
}

inline f32
kh_safe_ratio1_f32(f32 numerator, f32 divisor) {
	f32 res = kh_safe_ratioN_f32(numerator, divisor, 1.0f);
	return(res);
}

inline f32
kh_safe_ratioNum_f32(f32 numerator, f32 divisor) {
	f32 res = kh_safe_ratioN_f32(numerator, divisor, numerator);
	return(res);
}

inline u32
kh_round_f32_to_u32(f32 val) {
  u32 res = (u32)roundf(val);
  return(res);
}

inline u32
kh_ceil_f32_to_u32(f32 val) {
	u32 res = (u32)ceilf(val);
	return(res);
}

inline i32
kh_truncate_f32_to_i32(f32 val) {
  i32 res = (i32)(val);
  return(res);
}


inline i32
kh_round_f32_to_i32(f32 val) {
  i32 res = (i32)roundf(val);
  return(res);
}

inline i32
kh_ceil_f32_to_i32(f32 val) {
  i32 res = (i32)ceilf(val);
  return(res);
}

inline i32
kh_floor_f32_to_i32(f32 val) {
  i32 res = (i32)floorf(val);
  return(res);
}

inline f32
kh_fract_f32(f32 a) {
	f32 res = a - kh_floor_f32_to_i32(a);
	return(res);
}

// NOTE(flo): FLOAT64

inline f64
kh_exp_f64(f64 a) {
	f64 res = exp(a);
	return(res);
}

inline f64
kh_abs_f64(f64 val) {
	f64 res = fabs(val);
	return(res);
}

inline f64
kh_clamp_f64(f64 min, f64 max, f64 val) {
	f64 res = val;
	if(res < min) res = min;
	if(res > max) res = max;
	return(res);
}

inline f64
kh_clamp01_f64(f64 val) {
	f64 res = kh_clamp_f64(0.0, 1.0, val);
	return(res);
}