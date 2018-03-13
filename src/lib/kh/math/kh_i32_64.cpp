inline i32
kh_signof_i32(i32 val) {
	i32 res = (val >= 0) ? 1 : -1;
	return(res);
}

inline i32
kh_abs_i32(i32 val) {
	i32 res = abs(val);
	return(res);
}

inline i32
kh_min_i32(i32 a, i32 b) {
  i32 res = (a < b) ? a : b;
  return(res);
}

inline i32
kh_max_i32(i32 a, i32 b) {
  i32 res = (a > b) ? a : b;
  return(res);
}

inline i32
kh_clamp_i32(i32 min, i32 max, i32 val) {
	i32 res = val;

	if(res < min) {
		res = min;
	}
	if(res > max) {
		res = max;
	}
	return(res);
}