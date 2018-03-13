struct RandomXOR {
	u32 state;
};

inline u32 
xorshift_32(u32 state) {
	u32 x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return(x);
}

// 0.0, 0.5, 0.25, 0.75, 0.125, 0.625, 0.375, 0.825 etc...
KH_INTERN f32
radical_inverse_van_der_corpus(u32 i) {
	u32 bits = i;
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    f32 res = (f32)bits * 2.3283064365386963e-10; // / 0x100000000 
    return(res);
}

KH_INTERN v2 
hammersley(u32 i, u32 count) {
	v2 res;
	res.x = (f32)i / (f32)count;
	res.y = radical_inverse_van_der_corpus(i);
	return(res);
}


KH_INLINE RandomXOR 
rand_seed(u32 seed) {
	RandomXOR res;
	res.state = seed;
	return(res);
}


KH_INLINE u32
rand_next(RandomXOR *series) {
	u32 res = xorshift_32(series->state); 
	series->state = res;
	return(res);
}

inline f32
rand_01_f32(RandomXOR *series) {
	f32 denom = 1.0f / (f32)U32_MAX;
	f32 res = (f32)rand_next(series) * denom;
	return(res);
}

inline f32
rand_m11_f32(RandomXOR *series) {
	f32 res = -1.0f + 2.0f * rand_01_f32(series);
	return(res);
}