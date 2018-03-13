inline u32
kh_safe_truncate_u64_to_u32(u64 val) {
	kh_assert(val < 0xFFFFFFFF);
	u32 res = (u32)val;
	return(res);
}

inline u32
kh_safe_ratioN_u32(u32 numerator, u32 divisor, u32 n) {
	u32 res = n;
	if(divisor != 0) 	{
		res = numerator / divisor;
	}
	return(res);
}

inline u32
kh_safe_ratio0_u32(u32 numerator, u32 divisor) {
	u32 res = kh_safe_ratioN_u32(numerator, divisor, 0);
	return(res);
}

inline u32
kh_safe_ratio1_u32(u32 numerator, u32 divisor) {
	u32 res = kh_safe_ratioN_u32(numerator, divisor, 1);
	return(res);
}

inline u32
kh_safe_ratioNum_u32(u32 numerator, u32 divisor) {
	u32 res = kh_safe_ratioN_u32(numerator, divisor, numerator);
	return(res);
}

inline u32
kh_min_u32(u32 a, u32 b) {
  u32 res = (a < b) ? a : b;
  return(res);
}

inline u32
kh_max_u32(u32 a, u32 b) {
  u32 res = (a > b) ? a : b;
  return(res);
}

inline u32
kh_clamp_u32(u32 min, u32 max, u32 val) {
	u32 res = val;
	if(res < min) {
		res = min;
	}
	if(res > max) {
		res = max;
	}
	return(res);
}

inline umm
kh_min_umm(umm a, umm b) {
  umm res = (a < b) ? a : b;
  return(res);
}

inline umm
kh_max_umm(umm a, umm b) {
  umm res = (a > b) ? a : b;
  return(res);
}
