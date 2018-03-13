struct Color_RGB_U8 {
	u8 r, g, b;
};

struct Color_BGR_U8 {
	u8 b, g, r;
};

struct Color_RGBA_U8 {
	u8 r, g, b, a;
};

struct Color_BGRA_U8 {
	u8 a, b, g, r;
};

struct Color_Gray_U8 {
	u8 r;
};

struct Color_RGB_F32 {
	f32 r, g, b;
};

struct Color_RGBA_F32 {
	f32 r, g, b, a;
};

inline Color_RGBA_U8
to_rgba_u8(Color_RGB_F32 *color) {
	Color_RGBA_U8 res;
	res.r = color->r * 255;
	res.g = color->g * 255;
	res.b = color->b * 255;
	res.a = 0xFF;
	return(res);
}

inline void
set_color(Color_BGR_U8 *pixel, u32 ind, u8 val) {
	((u8 *)pixel)[ind] = val;
}

inline u8
get_color(Color_BGR_U8 *pixel, u32 ind) {
	u8 res = ((u8 *)pixel)[ind];
	return(res);
}
