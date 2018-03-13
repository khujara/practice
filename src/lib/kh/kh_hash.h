KH_INTERN u32
hash_key_from_djb2(char *str) {
	u32 res = 5381;
	char *at = str;
	while(*at++) {
		res = ((res << 5) + res) + *at; /* hash * 33 + *at */
	}
	return(res);
}

KH_INTERN u32
hash_key_from_djb2(char *str, u32 name_len) {
	u32 res = 5381;
	for(u32 i = 0; i < name_len; ++i) {
		res = ((res << 5) + res) + str[i]; /* hash * 33 + *at */
	}
	return(res);
}

KH_INTERN u32
hash_key_from_djb2a(char *str) {
	u32 res = 5381;
	char *at = str;
	while(*at++) {
		res = (((res << 5) + res) ^ (*at)); /* hash * 33 + *at */
	}
	return(res);
}

KH_INTERN u32
hash_key_from_dotnet(char *str, u32 length) {
	// u32 length = string_length(str);
	u32 hash1 = (5381<<16) + 5381;
	u32 hash2 = hash1;

	u32 *pint = (u32 *)str;
	i32 len;
	for(len = length; len > 1; len -= 8) {
		hash1 = ((hash1 << 5) + hash1 + (hash1 >> 27) ^ pint[0]);
		hash2 = ((hash2 << 5) + hash2 + (hash2 >> 27) ^ pint[1]);
		pint += 2;
	}

	if(len > 0) {
		hash1 = ((hash1 << 5) + hash1 + (hash1 >> 27)) ^ pint[0];
	}

	u32 res = hash1 + (hash2 * 1566083941);
	return(res);
}