#define STBTT_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "ext\stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "ext\stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "ext\stb_truetype.h"


KH_INTERN void
load_tex2d(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size) {
	int w, h, bpp;
	stbi_uc *memory = stbi_load_from_memory((u8 *)file_contents, file_size, &w, &h, &bpp, 0);
	kh_assert(memory);
	contents->type.tex2d.width = w;
	contents->type.tex2d.height = h;
	contents->type.tex2d.bytes_per_pixel = bpp;
	contents->type.key = AssetType_tex2d;

	kh_assert(w > 0);
	kh_assert(h > 0);
	kh_assert(bpp > 0);
	kh_assert(memory);
	contents->size = w * h * bpp;
	contents->memory = kh_push(allocator, contents->size);
	u32 pitch = bpp * w;
					// NOTE(flo): we need to flip the Y from the stb loaded image
	u8 *dst_row = (u8 *)contents->memory;
	u8 *src_row = (u8 *)memory + ((h -1) * pitch);
	for(u32 y = 0; y < (u32)h; ++y) {
		u8 *dst_pixel = (u8 *)dst_row;
		u8 *src_pixel = (u8 *)src_row;
		for(u32 x = 0; x < (u32)w; ++x) {					
			if(bpp == 4) {
				u8 *src_r = src_pixel + 0;
				u8 *src_g = src_pixel + 1;
				u8 *src_b = src_pixel + 2;
				u8 *src_a = src_pixel + 3;

				u8 *dst_b = dst_pixel + 0;
				u8 *dst_g = dst_pixel + 1;
				u8 *dst_r = dst_pixel + 2;
				u8 *dst_a = dst_pixel + 3;

				*dst_a = *src_a;
				*dst_r = *src_r;
				*dst_g = *src_g;
				*dst_b = *src_b;
			}
			else {
				u8 *src_r = src_pixel + 0;
				u8 *src_g = src_pixel + 1;
				u8 *src_b = src_pixel + 2;

				u8 *dst_b = dst_pixel + 0;
				u8 *dst_g = dst_pixel + 1;
				u8 *dst_r = dst_pixel + 2;

				*dst_r = *src_r;
				*dst_g = *src_g;
				*dst_b = *src_b;

			}
			dst_pixel += bpp;
			src_pixel += bpp;
		}

		dst_row += pitch;
		src_row -= pitch;
	}
	stbi_image_free(memory);
}

KH_INTERN void
load_font(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size, FontParam *f_param) {
	stbtt_pack_range ranges[1] = {};

	u32 glyph_count = f_param->glyph_count;
	contents->type.font.glyph_count = glyph_count;
	contents->type.font.highest_codepoint = 0;
	contents->type.key = AssetType_font;


	void *chardata_memory = malloc(glyph_count * sizeof(stbtt_packedchar));
	stbtt_pack_context spc;
	ranges[0].font_size = f_param->pixels_height;
	ranges[0].array_of_unicode_codepoints = (int *)f_param->cp_arr;
	ranges[0].num_chars = glyph_count;
	ranges[0].chardata_for_range = (stbtt_packedchar *)chardata_memory;


	u32 tex_w = 512;
	u32 tex_h = 512;
	u32 bpp = 1;

	contents->type.font.tex_w = tex_w;
	contents->type.font.tex_h = tex_h;

	u32 font_glyphs_size = sizeof(FontGlyph) * glyph_count;
	u32 kernel_advance_size = glyph_count*glyph_count*sizeof(f32);
	u32 texture_size = tex_w * tex_h * bpp;

	contents->size = font_glyphs_size + kernel_advance_size + texture_size;
	contents->memory = kh_push(allocator, contents->size);

	f32 *kernel_advances = (f32 *)((u8 *)contents->memory + font_glyphs_size);
	u8 *texture_memory = (u8 *)contents->memory + kernel_advance_size;

	int stb_begin = stbtt_PackBegin(&spc, texture_memory, (int)tex_w, (int)tex_h, 0, 4, NULL);
	kh_assert(stb_begin == 1);
	int stb_pack = stbtt_PackFontRanges(&spc, (u8 *)file_contents, 0, ranges, 1);
	kh_assert(stb_pack == 1);
	stbtt_PackEnd(&spc);

	stbtt_fontinfo stb_font;
	stbtt_InitFont(&stb_font, (u8 *)file_contents, stbtt_GetFontOffsetForIndex((u8 *)file_contents, 0));
	f32 scale = stbtt_ScaleForPixelHeight(&stb_font, f_param->pixels_height);

	i32 ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&stb_font, &ascent, &descent, &line_gap);

	contents->type.font.advance_y = -((f32)((ascent - descent) + line_gap) * scale);

	for(u32 glyph_ind = 0; glyph_ind < glyph_count; ++glyph_ind) {
		stbtt_packedchar *stb_char = ranges[0].chardata_for_range + glyph_ind;
		u32 codepoint = f_param->cp_arr[glyph_ind];

		FontGlyph *glyph = (FontGlyph *)contents->memory + glyph_ind;
		glyph->code_point       = codepoint;
		glyph->x0               = stb_char->x0;
		glyph->x1               = stb_char->x1;
		glyph->y0               = stb_char->y0;
		glyph->y1               = stb_char->y1;;
		glyph->xoff             = stb_char->xoff;
		glyph->yoff             = stb_char->yoff;
		glyph->xoff1            = stb_char->xoff2;
		glyph->yoff1            = stb_char->yoff2;
		glyph->advance_width    = stb_char->xadvance;
						// glyph.left_side_bearing = (f32)left_side_bearing * scale;

						// TODO(flo): can we just avoid this n^2 loop?
		for(u32 glyph_ind_2 = 0; glyph_ind_2 < glyph_count; ++glyph_ind_2) {
			u32 codepoint_2 = f_param->cp_arr[glyph_ind_2];
			i32 advance = stbtt_GetCodepointKernAdvance(&stb_font, codepoint, codepoint_2);
			*kernel_advances++ = (f32)advance * scale;
		}

		if(codepoint > contents->type.font.highest_codepoint) {
			contents->type.font.highest_codepoint = codepoint;
		}

	}
	free(chardata_memory);
}