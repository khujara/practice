/*
---------------------------
	NOTE: INTRINSICS IMPLEMENTATION
---------------------------
*/

inline u32
kh_shift_left(u32 data, u32 shift) {
	u32 res = data << shift;
	return(res);
}

inline u32
kh_shift_right(u32 data, u32 shift) {
	u32 res = data >> shift;
	return(res);
}

inline u32
kh_rot_left(u32 val, i32 shift) {
#if COMPILER_MSVC
	u32 res = _rotl(val, shift);
#else
	u32 res = ((val >> (~shift)) >> 1) | (val << shift);
#endif
	return(res);
}

inline u32
kh_rot_right(u32 val, i32 shift) {
#if COMPILER_MSVC
  u32 res = _rotr(val, shift);
#else
  u32 res =  ((val << (~shift)) << 1) | (val >> shift);
#endif
  return(res);
}

inline BitScan
kh_find_least_significant_set_bit_U32(u32 val) {
	BitScan res = {};
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

inline BitScan
kh_find_most_significant_set_bit_U32(u32 val) {
	BitScan res = {};
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
kh_exchange_significant_bits(u32 val) {
	u8 *e = (u8 *)&val;
	u32 res = ((e[0] << 24) | (e[1] << 16) | (e[2] << 8) | (e[3] << 0));
	return(res);
}