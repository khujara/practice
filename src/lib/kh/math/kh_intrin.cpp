inline u32
kh_shift_left(u32 data, u32 shift)
{
	u32 res = data << shift;
	return(res);
}

inline u32
kh_shift_right(u32 data, u32 shift)
{
	u32 res = data >> shift;
	return(res);
}

inline u32
kh_rot_left(u32 val, i32 shift)
{
#if COMPILER_MSVC
	u32 res = _rotl(val, shift);
#else
	u32 res = ((val >> (~shift)) >> 1) | (val << shift);
#endif
	return(res);
}

inline u32
kh_rot_right(u32 val, i32 shift)
{
#if COMPILER_MSVC
  u32 res = _rotr(val, shift);
#else
  u32 res =  ((val << (~shift)) << 1) | (val >> shift);
#endif
  return(res);
}

inline bit_scan
kh_find_least_significant_set_bit_U32(u32 val)
{
	bit_scan res = {};

#if COMPILER_MSVC
	res.found = _BitScanForward((unsigned long *)&res.index, val);
#else
	for(u32 test = 0; test < 32; ++test)
	{
		if(val & (1 << test))
		{
			res.found = true;
			res.index = test;
			break;
		}
	}
#endif
	return(res);
}

inline bit_scan
kh_find_most_significant_set_bit_U32(u32 val)
{
	bit_scan res = {};
#if COMPILER_MSVC
	res.found = _BitScanReverse((i32 *)&res.index, val);
#else
	for(u32 test = 32; test > 0; --test)
	{
		if(val & (1 >> test))
		{
			res.found = true;
			res.index = test;
			break;
		}
	}
#endif
return(res);
}

inline u32
kh_exchange_significant_bits(u32 val)
{
	u8 *e = (u8 *)&val;
	u32 res = ((e[0] << 24) | (e[1] << 16) | (e[2] << 8) | (e[3] << 0));
	return(res);
}

/*
---------------------------
	NOTE: INTRINSICS IMPLEMENTATION
---------------------------
*/

inline f32
kh_mod_f32(f32 a, f32 b) {
	f32 res = fmodf(a, b);
	return(res);
}

inline i32
kh_signof_i32(i32 val)
{
	i32 res = (val >= 0) ? 1 : -1;
	return(res);
}

inline f32
kh_signof_f32(f32 val)
{
		f32 res = (val >= 0.0f) ? 1.0f : -1.0f;
		return(res);
}

inline f32
kh_sqrt_f32(f32 val)
{
	f32 res = sqrtf(val);
	return(res);
}

inline f32
kh_abs_f32(f32 val)
{
	f32 res = fabsf(val);
	return(res);
}

inline f64
kh_abs_f64(f64 val) {
	f64 res = fabs(val);
	return(res);
}

inline i32
kh_abs_i32(i32 val)
{
	i32 res = abs(val);
	return(res);
}

inline i32
kh_round_f32_to_i32(f32 val)
{
  i32 res = (i32)roundf(val);
  return(res);
}

inline u32
kh_round_f32_to_u32(f32 val)
{
  u32 res = (u32)roundf(val);
  return(res);
}

inline i32
kh_truncate_f32_to_i32(f32 val)
{
  i32 res = (i32)(val);
  return(res);
}

inline u32
kh_safe_truncate_u64_to_u32(u64 val)
{
	kh_assert(val < 0xFFFFFFFF);
	u32 res = (u32)val;
	return(res);
}

inline i32
kh_ceil_f32_to_i32(f32 val)
{

  i32 res = (i32)ceilf(val);
  return(res);
}

inline u32
kh_ceil_f32_to_u32(f32 val)
{
	u32 res = (u32)ceilf(val);
	return(res);
}

inline i32
kh_floor_f32_to_i32(f32 val)
{

  i32 res = (i32)floorf(val);
  return(res);
}

inline f32
kh_sin_f32(f32 angle)
{
	f32 res = sinf(angle);
	return(res);
}

inline f32
kh_cos_f32(f32 angle)
{
	f32 res = cosf(angle);
	return(res);
}

inline f32
kh_tan_f32(f32 angle)
{
  f32 res = tanf(angle);
  return(res);
}

inline f32
kh_atan_f32(f32 y, f32 x)
{
	f32 res = atan2f(y, x);
	return(res);
}

inline f32
kh_asin_f32(f32 val)
{
	f32 res = asinf(val);
	return(res);
}

inline f32
kh_acos_f32(f32 val)
{
	f32 res = acosf(val);
	return(res);
}

inline u32
kh_safe_ratioN_u32(u32 numerator, u32 divisor, u32 n)
{
	u32 res = n;

	if(divisor != 0)
	{
		res = numerator / divisor;
	}

	return(res);
}

inline u32
kh_safe_ratio0_u32(u32 numerator, u32 divisor)
{
	u32 res = kh_safe_ratioN_u32(numerator, divisor, 0);

	return(res);
}

inline u32
kh_safe_ratio1_u32(u32 numerator, u32 divisor)
{
	u32 res = kh_safe_ratioN_u32(numerator, divisor, 1);

	return(res);
}

inline u32
kh_safe_ratioNum_u32(u32 numerator, u32 divisor)
{
	u32 res = kh_safe_ratioN_u32(numerator, divisor, numerator);
	return(res);
}

inline f32
kh_safe_ratioN_f32(f32 numerator, f32 divisor, f32 n)
{
	f32 res = n;

	if(divisor != 0.0f)
	{
		res = numerator / divisor;
	}

	return(res);
}

inline f32
kh_safe_ratio0_f32(f32 numerator, f32 divisor)
{
	f32 res = kh_safe_ratioN_f32(numerator, divisor, 0.0f);

	return(res);
}

inline f32
kh_safe_ratio1_f32(f32 numerator, f32 divisor)
{
	f32 res = kh_safe_ratioN_f32(numerator, divisor, 1.0f);

	return(res);
}

inline f32
kh_safe_ratioNum_f32(f32 numerator, f32 divisor)
{
	f32 res = kh_safe_ratioN_f32(numerator, divisor, numerator);
	return(res);
}

inline f32
kh_sqr_f32(f32 a)
{
	f32 res = a * a;

	return(res);
}

inline f32
kh_mix_f32(f32 a, f32 b, f32 t)
{
	f32 res = (1.0f - t)*a + t*b;

	return(res);
}

inline f32
kh_mix_imprecise_f32(f32 a, f32 b, f32 t)
{
	f32 res = a + t*(b - a);
	return(res);
}

inline i32
kh_min_i32(i32 a, i32 b)
{
  i32 res = (a < b) ? a : b;
  return(res);
}

inline i32
kh_max_i32(i32 a, i32 b)
{
  i32 res = (a > b) ? a : b;
  return(res);
}

inline u32
kh_min_u32(u32 a, u32 b)
{
  u32 res = (a < b) ? a : b;
  return(res);
}

inline u32
kh_max_u32(u32 a, u32 b)
{
  u32 res = (a > b) ? a : b;
  return(res);
}

inline umm
kh_min_umm(umm a, umm b)
{
  umm res = (a < b) ? a : b;
  return(res);
}

inline umm
kh_max_umm(umm a, umm b)
{
  umm res = (a > b) ? a : b;
  return(res);
}

inline f32
kh_min_f32(f32 a, f32 b)
{
  f32 res = (a < b) ? a : b;
  return(res);
}

inline f32
kh_max_f32(f32 a, f32 b)
{
  f32 res = (a > b) ? a : b;
  return(res);
}

inline f32
kh_min2_f32(f32 a, f32 b)
{
	f32 res = 0.5f * (a + b - kh_abs_f32(a - b));
	return(res);
}

inline f32
kh_max2_f32(f32 a, f32 b)
{
	f32 res = 0.5f * (a + b + kh_abs_f32(a - b));
	return(res);
}

inline i32
kh_clamp_i32(i32 min, i32 max, i32 val)
{
	i32 res = val;

	if(res < min)
	{
		res = min;
	}
	if(res > max)
	{
		res = max;
	}

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

inline f32
kh_clamp_f32(f32 min, f32 max, f32 val)
{
	f32 res = val;

	if(res < min)
	{
		res = min;
	}
	if(res > max)
	{
		res = max;
	}

	return(res);
}

inline f32
kh_clamp01_f32(f32 val)
{
	f32 res = kh_clamp_f32(0.0f, 1.0f, val);

	return(res);
}

inline f32
kh_remap_f32(f32 min, f32 max, f32 t, f32 remap_min, f32 remap_max)
{
	f32 res = 0;
	f32 range = max - min;
	if(range > 0)
	{
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
kh_remap01_f32(f32 min, f32 max, f32 t)
{
	f32 res = 0;
	f32 range = max - min;
	if(range > 0)
	{
		res = (t - min) / range;
	}
	return(res);
}

inline f32
kh_remap_clamp_f32(f32 min, f32 max, f32 t, f32 remap_min, f32 remap_max)
{
	f32 res = 0;
	if(t < min)
	{
		res = remap_min;
	}
	else if(t > max)
	{
		res = remap_max;
	}
	else
	{
		res = kh_remap_f32(min, max, t, remap_min, remap_max);
	}

}

inline f32
kh_remap_clamp01_f32(f32 min, f32 max, f32 t)
{
	f32 res = 0.0f;

	f32 Range = max - min;
	if(Range != 0.0f)
	{
		res = kh_clamp01_f32((t - min) / Range);
	}

	return(res);
}

inline f32
kh_fract_f32(f32 a)
{
	f32 res = a - kh_floor_f32_to_i32(a);
	return(res);
}


/*
---------------------------
	NOTE: BITWISE IMPLEMENTATION
---------------------------
*/
	/* NOTE(flo): bitwise review
		& bitwise AND &=
			0 0 0
			0 1 0
			1 0 0
			1 1 1
		| bitwise OR |=
			0 0 0
			0 1 1
			1 0 1
			1 1 1
		^ bitwise XOR ^=
			0 0 0
			0 1 1
			1 0 1
			1 1 0
		~ bitwise NOT
		  0 1
			1 0
		<< left shift
		>> right shift

	flags set :
		0x1
		0x2
		0x4
		0x8
		0x10
		0x20
		0x40
		0x80
		0x100
		0x200
		0x400
		0x800
		0x1000
		.........

	*/