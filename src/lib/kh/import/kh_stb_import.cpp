#define STBTT_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "ext\stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "ext\stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "ext\stb_truetype.h"


KH_INTERN void
load_tex2d(AssetContents *contents, LinearArena *arena, void *file_contents, u32 file_size) {
	int w, h, src_bpp;
	stbi_uc *memory = stbi_load_from_memory((u8 *)file_contents, file_size, &w, &h, &src_bpp, 0);
	u32 dst_bpp = (src_bpp == 1) ? 3 : src_bpp;
	kh_assert(memory);
	contents->type.tex2d.width = w;
	contents->type.tex2d.height = h;
	contents->type.tex2d.bytes_per_pixel = dst_bpp;
	contents->type.key = AssetType_tex2d;

	kh_assert(w > 0);
	kh_assert(h > 0);
	kh_assert(src_bpp > 0);
	kh_assert(memory);

	contents->size = w * h * dst_bpp;
	contents->memory = kh_push(arena, contents->size);

	u32 src_pitch = src_bpp * w;
	u32 dst_pitch = dst_bpp * w;

	// NOTE(flo): we need to flip the Y from the stb loaded image
	u8 *dst_row = (u8 *)contents->memory;
	u8 *src_row = (u8 *)memory + ((h -1) * src_pitch);
	for(u32 y = 0; y < (u32)h; ++y) {
		u8 *dst_pixel = (u8 *)dst_row;
		u8 *src_pixel = (u8 *)src_row;
		for(u32 x = 0; x < (u32)w; ++x) {					
			if(src_bpp == 4) {
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
			else if(src_bpp == 3) {
				u8 *src_r = src_pixel + 0;
				u8 *src_g = src_pixel + 1;
				u8 *src_b = src_pixel + 2;

				u8 *dst_b = dst_pixel + 0;
				u8 *dst_g = dst_pixel + 1;
				u8 *dst_r = dst_pixel + 2;

				*dst_r = *src_r;
				*dst_g = *src_g;
				*dst_b = *src_b;
			} else {
				// TODO(flo): stop doing that !!!!!!!!! and support different bytes per pixel for texturing
				kh_assert(src_bpp == 1);
				u8 *src_r = src_pixel + 0;

				u8 *dst_b = dst_pixel + 0;
				u8 *dst_g = dst_pixel + 1;
				u8 *dst_r = dst_pixel + 2;

				*dst_r = *src_r;
				*dst_g = *src_r;
				*dst_b = *src_r;
			}
			dst_pixel += dst_bpp;
			src_pixel += src_bpp;
		}

		dst_row += dst_pitch;
		src_row -= src_pitch;
	}
	stbi_image_free(memory);
}

KH_INTERN void
load_font(AssetContents *contents, LinearArena *arena, void *file_contents, u32 file_size,
          u32 first_glyph_in_range, u32 last_glyph_in_range, f32 pixels_height, u32 tex_size) {

	u32 glyph_count = last_glyph_in_range - first_glyph_in_range;

	contents->type.font.first_glyph = first_glyph_in_range;
	contents->type.font.last_glyph = last_glyph_in_range;
	contents->type.key = AssetType_font;

	void *chardata_memory = malloc(glyph_count * sizeof(stbtt_packedchar));
	int *codepoints = (int *)malloc(glyph_count * sizeof(int));
	u32 first = first_glyph_in_range;
	for(u32 i = 0; i < glyph_count; ++i) {
		codepoints[i] = first++;
	}

	stbtt_pack_range ranges[1] = {};
	stbtt_pack_context spc;
	ranges[0].font_size = pixels_height;
	ranges[0].array_of_unicode_codepoints = codepoints;
	ranges[0].num_chars = glyph_count;
	ranges[0].chardata_for_range = (stbtt_packedchar *)chardata_memory;

	u32 tex_w = tex_size;
	u32 tex_h = tex_size;
	u32 bpp = 1;

	contents->type.font.tex_w = tex_w;
	contents->type.font.tex_h = tex_h;

	u32 font_glyphs_size = sizeof(FontGlyph) * glyph_count;
	u32 kernel_advance_size = glyph_count*glyph_count*sizeof(f32);
	u32 texture_size = tex_w * tex_h * bpp;

	contents->size = font_glyphs_size + kernel_advance_size + texture_size;
	contents->memory = kh_push(arena, contents->size);

	u8 *texture_memory = (u8 *)contents->memory;
	f32 *kernel_advances = (f32 *)((u8 *)contents->memory + texture_size + font_glyphs_size);


	u8 *tmp_texture = (u8 *)malloc(texture_size);
	int stb_begin = stbtt_PackBegin(&spc, tmp_texture, (int)tex_w, (int)tex_h, 0, 4, NULL);
	kh_assert(stb_begin == 1);
	int stb_pack = stbtt_PackFontRanges(&spc, (u8 *)file_contents, 0, ranges, 1);
	kh_assert(stb_pack == 1);
	stbtt_PackEnd(&spc);

	kh_assert(bpp == 1);
	u32 pitch = bpp * tex_w;
	u8 *dst_row = texture_memory;
	u8 *src_row = (u8 *)tmp_texture + ((tex_h - 1) * pitch);
	for(u32 y = 0; y < tex_h; ++y) {
		u8 *dst_pixel = dst_row;
		u8 *src_pixel = src_row;
		for(u32 x = 0; x < tex_w; ++x) {
			*dst_pixel++ = *src_pixel++;
		}
		dst_row += pitch;
		src_row -= pitch;
	}


	stbtt_fontinfo stb_font;
	stbtt_InitFont(&stb_font, (u8 *)file_contents, stbtt_GetFontOffsetForIndex((u8 *)file_contents, 0));
	f32 scale = stbtt_ScaleForPixelHeight(&stb_font, pixels_height);

	i32 ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&stb_font, &ascent, &descent, &line_gap);

	contents->type.font.advance_y = -((f32)((ascent - descent) + line_gap) * scale);

	FontGlyph *font_glyphs = (FontGlyph *)((u8 *)contents->memory + texture_size);
	for(u32 glyph_ind = 0; glyph_ind < glyph_count; ++glyph_ind) {
		stbtt_packedchar *stb_char = ranges[0].chardata_for_range + glyph_ind;
		u32 codepoint = codepoints[glyph_ind];

		FontGlyph *glyph = font_glyphs + glyph_ind;
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

		for(u32 glyph_ind_2 = 0; glyph_ind_2 < glyph_count; ++glyph_ind_2) {
			u32 codepoint_2 = codepoints[glyph_ind_2];
			i32 advance = stbtt_GetCodepointKernAdvance(&stb_font, codepoint, codepoint_2);
			*kernel_advances++ = (f32)advance * scale;
		}
	}
	free(tmp_texture);
	free(chardata_memory);
	free(codepoints);
}