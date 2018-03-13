typedef struct SortEntry {
	f32 key;
	u32 ind;
} SortEntry;

inline u32
sort_key_to_u32(f32 key) {
	u32 res = *(u32 *)&key;
	if(res & 0x80000000) {
		res = ~res;
	} else {
		res |= 0x80000000;
	}
	return(res);
}

KH_INTERN void
radix_sort(u32 count, SortEntry *src, SortEntry *dst) {
	kh_assert(src);
	kh_assert(dst);

	const u32 size = 256;
	for(u32 b = 0; b < 32; b += 8) {
		u32 sort_keys_off[size] = {};
		for(u32 i = 0; i < count; ++i) {
			u32 radix_val = sort_key_to_u32(src[i].key);
			u32 radix_piece = (radix_val >> b) & 0xFF;
			++sort_keys_off[radix_piece];
		}

		u32 total = 0;
		for(u32 sk_i = 0; sk_i < array_count(sort_keys_off); ++sk_i) {
			u32 c = sort_keys_off[sk_i];
			sort_keys_off[sk_i] = total;
			total += c;
		}

		for(u32 j = 0; j < count; ++j) {
			u32 radix_val = sort_key_to_u32(src[j].key);
			u32 radix_piece = (radix_val >> b) & 0xFF;
			dst[sort_keys_off[radix_piece]++] = src[j];
		}

		kh_swap_ptr(SortEntry, src, dst, tmp);
	}
}