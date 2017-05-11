#include "kh\kh_platform.h"
#include <windows.h>
#include "kh\kh_win32.h"

// TODO(flo): DXT, ETC2, PVRTC and ASTC real-time de/compression

#include "kh_asset_file.h"
#include "kh_asset_packaging.h"
#include "kh\kh_tokenizer.h"
// #include "..\..\..\kh\kh_texture_compression.h"
#include "kh_assimp_import.h"

#include <stdlib.h>

#define USE_STB_LIBRARY

#ifdef USE_STB_LIBRARY
	#define STBTT_STATIC
	#define STB_IMAGE_IMPLEMENTATION
	#include "ext\stb_image.h"
	#define load_texture_2d(filename, res) int _x,_y,n; stbi_uc *data = stbi_load(filename, &_x, &_y, &n, 0); \
	res = {}; res.memory = (void *)data; res.width = _x; res.height = _y; res.contents = (void *)data; res.bytes_per_pixel = n;
	#define free_texture_2d(contents) stbi_image_free(contents)
	#define STB_RECT_PACK_IMPLEMENTATION
	#include "ext\stb_rect_pack.h"
	#define STB_TRUETYPE_IMPLEMENTATION
	#include "ext\stb_truetype.h"
	// #define STB_DXT_IMPLEMENTATION
	// #include "..\..\..\libs\stb_dxt.h"
	// #define dxt_compression(dst, src, alpha, mode) stb_compress_dxt_block((u8 *)dst, (u8 *)src, alpha, mode)
#else
	#define free_texture_2d(contents) free(contents)
	KH_INTERN Texture2DContents
	load_texture_2d_file(char *filename, StackAllocator *stack)
	{
		Texture2DContents res = {};
		FileHandle f = win32_open_file(filename, stack, FileAccess_read, FileCreation_only_open);
		u32 size = win32_get_file_size(&f);
		void *contents = malloc(size);
		win32_read_bytes_of_file(&f, 0, size, contents);
		if(!f.error)
		{
			// if(test_png(f.contents)) res = load_png_file(contents);
			res = load_bmp_file(contents);
		}
		kh_assert(res.memory);
		return(res);
	}
	// TODO(flo): define for truetype fonts!
#endif

KH_INTERN TriangleMeshContents
load_trimesh_file(char *filename, MeshFileType mft, VertexFormat format, StackAllocator *stack) {
	TriangleMeshContents res = {};
	FileHandle f = win32_open_file(filename, stack, FileAccess_read, FileCreation_only_open);
	u32 size = win32_get_file_size(&f);
	StackAllocator contents_mem = {};
	void *contents = kh_push(&contents_mem, size);
	win32_read_bytes_of_file(&f, 0, size, contents);
	if(!f.error) {
		if(mft == MeshFile_obj) {
			res = load_trimesh(stack, contents, size, format);
		}
	}
	kh_clear(&contents_mem);

	// kh_assert(res.memory);
	return(res);
}

KH_INTERN AssetFileInfo *
add_asset_to_pack(asset_package *pack, AssetName name_key) {
	AssetTypeInfo *type_info = pack->types_info + name_key;
	if(type_info->asset_count == 0) {
		
		kh_assert(!type_info->arr_indices);
		pack->used_type_ids[pack->asset_type_count++] = name_key;
		type_info->arr_indices = (u32 *)kh_push_array(&pack->mem_infos, MAX_ASSETS_PER_TYPE, u32);
	}
	kh_assert(type_info->asset_count < MAX_ASSETS_PER_TYPE);
	type_info->arr_indices[type_info->asset_count++] = pack->asset_count;
	AssetFileInfo *res = pack->assets_info + pack->asset_count++;
	res->first_tag = 0;
	res->one_past_last_tag = 0;
	return(res);
}

KH_INTERN void
add_texture_2d_to_pack(asset_package *pack, AssetName name_key, char *filename) {
	AssetFileInfo *info = add_asset_to_pack(pack, name_key);
	info->filetype = AssetFileType_texture_2d;
	info->tex_2d_info.filename = filename;
}

KH_INTERN void
add_triangle_mesh_to_pack(asset_package *pack, AssetName name_key, char *filename, 
                          MeshFileType mft, VertexFormat format) {
	AssetFileInfo *info = add_asset_to_pack(pack, name_key);
	info->filetype = AssetFileType_triangle_mesh;
	info->tri_mesh_info.filename = filename;
	info->tri_mesh_info.mft = mft;
	info->tri_mesh_info.format = format;
}

#define ONE_PAST_MAX_FONT_CODEPOINT (0x10FFFF + 1)
#define MAX_GLYPHS_PER_FONT 4096

KH_INTERN AssetFileInfo *
add_font_to_pack(asset_package *pack, AssetName name_key, char *filename, char *fontname, f32 pixels_height) {
	AssetFileInfo *info = add_asset_to_pack(pack, name_key);
	info->filetype = AssetFileType_font;
	info->font_info.filename = filename;
	info->font_info.param = (FontParam *)kh_push_struct(&pack->mem_infos, FontParam);
	info->font_info.param->fontname = fontname;
	info->font_info.param->pixels_height = pixels_height;
	info->font_info.param->glyph_count = 0;
	info->font_info.param->max_glyph_count = MAX_GLYPHS_PER_FONT;
	info->font_info.param->cp_arr = (u32 *)kh_push_array(&pack->mem_infos, info->font_info.param->max_glyph_count, u32);
	return(info);
}

KH_INTERN void
add_glyph_to_font(AssetFileInfo *f_info, u32 code_point, AssetName name_key) {
	kh_assert(f_info->font_info.filename);
	kh_assert(f_info->font_info.param->pixels_height > 0);
	kh_assert(f_info->font_info.param->glyph_count < f_info->font_info.param->max_glyph_count);
	u32 glyph_ind = f_info->font_info.param->glyph_count++;
	f_info->font_info.param->cp_arr[glyph_ind] = code_point;
}

KH_INTERN void
add_asset_tag(asset_package *pack, AssetTagKeys key, f32 val) {
	AssetFileInfo *info = pack->assets_info + (pack->asset_count - 1);
	if(info->first_tag == 0) {	
		info->first_tag = pack->tag_count;
		info->one_past_last_tag = info->first_tag;
	}
	++info->one_past_last_tag;
	AssetTag *tag = pack->tag_arr + pack->tag_count++;
	tag->key = key;
	tag->val = val;
}

// #define BGR_TO_ARGB(src, dst) dst = ((0xFF << 24) | ((u8 *)src[2] << 16) | ((u8 *)src[1] << 8) | ((u8 *)src[0] << 0))
// #define ABGR_TO_ARGB(src, dst) dst = ((src[0] << 24) | ((u8 *)src[3] << 16) | ((u8 *)src[2] << 8) | ((u8 *)src[1] << 0))

// TODO(flo): handle premultiplied alpha in our bitmap if stb does not make it
// TODO(flo): multithread this
// TODO(flo): optmized this (SIMD/multithread see above), incremental writing
KH_INTERN void
send_pack_to_file(asset_package *pack, char *filename) {
	FILE *pack_file = fopen(filename, "wb");
	if(pack_file) {
		AssetFileHeader header = {};
		header.signature = KH_PACKAGE_SIGNATURE;
		header.version = KH_PACKAGE_VER;
		header.tag_count = pack->tag_count;
		header.asset_type_count = pack->asset_type_count;
		header.asset_count = pack->asset_count;

		u32 tag_arr_size = header.tag_count * sizeof(AssetTag);
		u32 type_arr_size = header.asset_type_count * sizeof(SourceAssetType);
		u32 asset_arr_size = header.asset_count * sizeof(SourceAsset);

		const u32 num_asset = pack->asset_count;

		header.tags_offset = sizeof(header);
		header.asset_types_offset = header.tags_offset + tag_arr_size;
		header.assets_offset = header.asset_types_offset + type_arr_size;
		u64 asset_data_offset = header.assets_offset + asset_arr_size;

		fwrite(&header, sizeof(header), 1, pack_file);
		fwrite(pack->tag_arr, tag_arr_size, 1, pack_file);
		fseek(pack_file, asset_data_offset, SEEK_SET);

		SourceAsset asset_arr[MAX_ASSET_COUNT];
		SourceAssetType type_arr[AssetName_count];

		u32 asset_count = 1;

		for(u32 type_ind = 0; type_ind < pack->asset_type_count; ++type_ind) {
			StackAllocator asset_mem = {};
			u32 type_id = pack->used_type_ids[type_ind];
			AssetTypeInfo *type_info = pack->types_info + type_id;
			kh_assert(type_info->asset_count > 0);
			SourceAssetType *type = type_arr + type_ind;
			type->type_id = type_id;
			type->one_past_last_asset = asset_count;
			type->first_asset = type->one_past_last_asset;
			for(u32 asset_ind = 0; asset_ind < type_info->asset_count; ++asset_ind) {
				u32 arr_ind = type_info->arr_indices[asset_ind];
				AssetFileInfo *asset_info = pack->assets_info + arr_ind;
				u32 ind = type->one_past_last_asset++;
				SourceAsset *asset = asset_arr + ind;
				asset->offset = ftell(pack_file);
				asset->first_tag = asset_info->first_tag;
				asset->one_past_last_tag = asset_info->one_past_last_tag;
				asset->format = asset_info->filetype;
				switch(asset_info->filetype) {
					case AssetFileType_texture_2d : {
						Texture2DContents tex_2d;
						#ifdef USE_STB_LIBRARY
						load_texture_2d(asset_info->tex_2d_info.filename, tex_2d);
						#else
						tex_2d = load_texture_2d_file(asset_info->tex_2d_info.filename, &asset_mem);
						#endif
						// TODO(flo): do the compression here
						asset->src_texture_2d.width = tex_2d.width;
						asset->src_texture_2d.height = tex_2d.height;
						asset->src_texture_2d.bytes_per_pixel = tex_2d.bytes_per_pixel;
						kh_assert(tex_2d.width > 0);
						kh_assert(tex_2d.height > 0);
						kh_assert(tex_2d.bytes_per_pixel > 0);
						kh_assert(tex_2d.memory);

						void *texture_memory = kh_push(&asset_mem, tex_2d.width * tex_2d.height * tex_2d.bytes_per_pixel);

						// TODO(flo): we don't need to do this if we are not using the STB library!
						// TODO(flo): handle gamma correction and premultiplied alpha?
						// TODO(flo): handle multiple bytes per pixel for textures!
						u32 src_pitch = tex_2d.bytes_per_pixel * tex_2d.width;
						u8 *dst_row = (u8 *)texture_memory;
						u8 *src_row = (u8 *)tex_2d.memory + ((tex_2d.height - 1) * src_pitch);
						for(u32 y = 0; y < tex_2d.height; ++y) {
							u8 *dst_pixel = (u8 *)dst_row;
							u8 *src_pixel = (u8 *)src_row;
							for(u32 x = 0; x < tex_2d.width; ++x) {					
								if(tex_2d.bytes_per_pixel == 4) {
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
								dst_pixel += tex_2d.bytes_per_pixel;
								src_pixel += tex_2d.bytes_per_pixel;
							}

							dst_row += src_pitch;
							src_row -= src_pitch;
						}

						// TODO(flo): need to do bottom up inversion for stb_image!
						fwrite(texture_memory, tex_2d.width*tex_2d.height*asset->src_texture_2d.bytes_per_pixel, 1, pack_file);

						// free(texture_memory);
						free_texture_2d(tex_2d.contents);
					} break;
					case AssetFileType_font : {
						// TODO(flo): remove all stb function so we can build our own font import system
						FileHandle f = win32_open_file(asset_info->font_info.filename, &asset_mem, 
							FileAccess_read, FileCreation_only_open);
						u32 size = win32_get_file_size(&f);
						void *contents = malloc(size);
						win32_read_bytes_of_file(&f, 0, size, contents);
						FontParam *f_param = asset_info->font_info.param;

						u32 pack_range_count = 1;
						stbtt_pack_range ranges[1] = {};

						u32 glyph_count = f_param->glyph_count;
						asset->src_font.glyph_count = glyph_count;
						asset->src_font.highest_codepoint = 0;

						stbtt_pack_context spc;
						ranges[0].font_size = f_param->pixels_height;
						ranges[0].array_of_unicode_codepoints = (int *)f_param->cp_arr;
						ranges[0].num_chars = f_param->glyph_count;
						ranges[0].chardata_for_range = kh_push_array(&asset_mem, f_param->glyph_count, stbtt_packedchar);

						u32 tex_w = 512;
						u32 tex_h = 512;
						u32 bpp = 1;
						asset->src_font.tex_w = tex_w;
						asset->src_font.tex_h = tex_h;
						asset->src_font.tex_bytes_per_pixel = bpp;
						u32 texture_size = tex_w * tex_h * bpp; 
						u8 *texture_memory = (u8 *)kh_push(&asset_mem, texture_size);

						int stb_begin = stbtt_PackBegin(&spc, texture_memory, (int)tex_w, (int)tex_h, 0, 4, NULL);
						kh_assert(stb_begin == 1);
						int stb_pack = stbtt_PackFontRanges(&spc, (u8 *)contents, 0, ranges, 1);
						kh_assert(stb_pack == 1);
						stbtt_PackEnd(&spc);

						stbtt_fontinfo stb_font;
						stbtt_InitFont(&stb_font, (u8 *)contents, stbtt_GetFontOffsetForIndex((u8 *)contents, 0));

						u32 kernel_advance_size = glyph_count*glyph_count*sizeof(f32);
						void *kernel_advance_memory = kh_push(&asset_mem, kernel_advance_size);

						f32 *kernel_advance = (f32 *)kernel_advance_memory;

						f32 scale = stbtt_ScaleForPixelHeight(&stb_font, f_param->pixels_height); 

						i32 ascent, descent, line_gap;
						stbtt_GetFontVMetrics(&stb_font, &ascent, &descent, &line_gap);

						// f32 scale = stb_scale;//(f_param->pixels_height / (ascent - descent)) + 1;
						// asset->src_font.ascent_height = (f32)ascent * scale;
						// asset->src_font.descent_height = (f32)descent * scale;
						// asset->src_font.line_gap = (f32)line_gap * scale;
						asset->src_font.advance_y =  -((f32)((ascent - descent) + line_gap) * scale);

						for(u32 glyph_ind = 0; glyph_ind < glyph_count; ++glyph_ind) {
							stbtt_packedchar *stb_char = ranges[0].chardata_for_range + glyph_ind;
							u32 glyph_offset = ftell(pack_file);
							u32 codepoint = f_param->cp_arr[glyph_ind];

							SourceFontGlyph glyph;
							glyph.code_point = codepoint;
							glyph.x0 = stb_char->x0;
							glyph.x1 = stb_char->x1;
							glyph.y0 = stb_char->y0;
							glyph.y1 = stb_char->y1;;
							glyph.xoff = stb_char->xoff;
							glyph.yoff = stb_char->yoff;
							glyph.xoff1 = stb_char->xoff2;
							glyph.yoff1 = stb_char->yoff2;
							glyph.advance_width = stb_char->xadvance;
							// glyph.left_side_bearing = (f32)left_side_bearing * scale;
							fwrite(&glyph, sizeof(SourceFontGlyph), 1, pack_file);



							// TODO(flo): can we just avoid this n^2 loop?
							for(u32 glyph_ind_2 = 0; glyph_ind_2 < glyph_count; ++glyph_ind_2) {
								u32 codepoint_2 = f_param->cp_arr[glyph_ind_2];
								i32 advance = stbtt_GetCodepointKernAdvance(&stb_font, codepoint, codepoint_2);
								*kernel_advance++ = (f32)advance * scale;
							}

							if(codepoint > asset->src_font.highest_codepoint) {
								asset->src_font.highest_codepoint = codepoint;
							}
						}
						fwrite(kernel_advance_memory, kernel_advance_size, 1, pack_file);
						fwrite(texture_memory, texture_size, 1, pack_file);

						free(contents);

					} break;
					case AssetFileType_triangle_mesh : {
						TriangleMeshContents tri_mesh = load_trimesh_file(asset_info->tri_mesh_info.filename,
							asset_info->tri_mesh_info.mft, asset_info->tri_mesh_info.format, &asset_mem);

						asset->src_tri_mesh.count = tri_mesh.vertices_count;
						asset->src_tri_mesh.tri_count = tri_mesh.tri_count;
						asset->src_tri_mesh.interleave = 1;
						VertexFormat format = tri_mesh.format;
						asset->src_tri_mesh.format = format;

						u32 vertex_size = get_size_from_vertex_format(format);
						fwrite(tri_mesh.vertices, tri_mesh.vertices_count * vertex_size, 1, pack_file);
						fwrite(tri_mesh.indices, tri_mesh.indices_count * sizeof(u32), 1, pack_file);

					} break;
					default : {} break;
				}
				kh_clear(&asset_mem);
			}

			asset_count = type->one_past_last_asset;
		}
		kh_assert(asset_count == pack->asset_count);

		fseek(pack_file, header.asset_types_offset, SEEK_SET);
		fwrite(type_arr, type_arr_size, 1, pack_file);
		fwrite(asset_arr, asset_arr_size, 1, pack_file);
		fclose(pack_file);
	}
}

KH_INTERN asset_package
init_package() {
	asset_package res;

	// arr_index mem_infos_size = megabytes(64);
	// arr_index mem_asset_size = MAX_TEMP_MEM_PER_ASSET;
	// TODO(flo): platform specific
	// void *mem = VirtualAlloc(0, mem_infos_size + mem_asset_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);//win32_virtual_alloc(mem_infos_size + mem_asset_size);

	res.mem_infos = {};

	res.tag_count = 1;
	res.asset_count = 1;
	res.asset_type_count = 0;
	return(res);
}

Platform g_platform;

KH_INTERN void
init_test_package() {

	win32_init_allocator_sentinel();
	g_platform.add_work_to_queue = win32_add_work_to_queue;
	g_platform.complete_all_queue_works = win32_complete_all_queue_works;
	g_platform.get_all_files_of_type = win32_get_all_files_of_type;
	g_platform.close_file_group = win32_close_file_group;
	g_platform.open_next_file_of_type = win32_open_next_file_of_type;
	g_platform.open_file = win32_open_file;
	g_platform.close_file = win32_close_file;
	g_platform.read_bytes_of_file = win32_read_bytes_of_file;
	g_platform.write_bytes_to_file = win32_write_bytes_to_file;
	g_platform.virtual_alloc = win32_virtual_alloc;
	g_platform.virtual_free = win32_virtual_free;
	// TODO(flo): allocation to virtual memory here!
	asset_package pack = init_package();
	#if 0
	add_texture_2d_to_pack(&pack, AssetType_randy_normal, "Randy_nmap.jpg");

	add_texture_2d_to_pack(&pack, AssetType_randy_occlusion, "Randy_ao.jpg");

	add_texture_2d_to_pack(&pack, AssetType_randy_emission, "Randy_gloss.jpg");

	add_texture_2d_to_pack(&pack, AssetType_randy_metal, "Randy_metalness2.jpg");

	// add_triangle_mesh_to_pack(&pack, AssetType_randy_mesh, "randy.dae");

	add_material_to_pack(&pack, AssetType_randy_mat_pbr, AssetType_randy_albedo, AssetType_randy_normal, AssetType_randy_occlusion,
		AssetType_randy_emission, AssetType_randy_metal);
	#endif
	/* TODO(flo) : animations!
		collada should gives us back a list of animations that can be treated separately

		add_animation_to_pack(&pack, AssetType_randy_idle, "randy.dae", "idle");
	*/

	// add_texture_2d_to_pack(&pack, AssetType_first_bmp, "textures/4squared.bmp");



	u32 count = (u32)('~' - '!') + 1;
	AssetFileInfo *arial_info = add_font_to_pack(&pack, AssetName_font, "c:/Windows/Fonts/arial.ttf", "Arial", 42);
	add_glyph_to_font(arial_info, ' ', AssetName_font);
	for(u32 c = '!'; c <= '~'; ++c)	{
		add_glyph_to_font(arial_info, c, AssetName_font);
	}
	// add_texture_2d_to_pack(&pack, AssetName_bricks, "textures/bricks2.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 0.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 0.0f);

	// add_texture_2d_to_pack(&pack, AssetName_bricks, "textures/bricks.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 1.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 1.0f);
	add_texture_2d_to_pack(&pack, AssetName_bricks, "textures/bricks4.jpg");
	add_texture_2d_to_pack(&pack, AssetName_bricks_normal, "textures/bricks4_nmap.jpg");

	add_texture_2d_to_pack(&pack, AssetName_wall, "textures/brickwall.jpg");
	add_texture_2d_to_pack(&pack, AssetName_wall_normal, "textures/brickwall_nmap.jpg");

	// add_texture_2d_to_pack(&pack, AssetName_first_png, "textures/SteamArcher01.png");

	// TODO(flo): real-time compression ?
	add_texture_2d_to_pack(&pack, AssetName_randy_albedo, "textures/Randy_albedo.jpg");
	add_texture_2d_to_pack(&pack, AssetName_randy_normal, "textures/Randy_nmap.jpg");
	// add_texture_2d_to_pack(&pack, AssetName_first_bmp, "..\\4squared.bmp");


	add_texture_2d_to_pack(&pack, AssetName_skybox_back, "textures/skybox/back.jpg");
	add_texture_2d_to_pack(&pack, AssetName_skybox_bottom, "textures/skybox/bottom.jpg");
	add_texture_2d_to_pack(&pack, AssetName_skybox_front, "textures/skybox/front.jpg");
	add_texture_2d_to_pack(&pack, AssetName_skybox_left, "textures/skybox/left.jpg");
	add_texture_2d_to_pack(&pack, AssetName_skybox_right, "textures/skybox/right.jpg");
	add_texture_2d_to_pack(&pack, AssetName_skybox_top, "textures/skybox/top.jpg");

	add_triangle_mesh_to_pack(&pack, AssetName_icosphere, "models/icosphere.obj", MeshFile_obj, VertexFormat_PosNormalUV);

	add_texture_2d_to_pack(&pack, AssetName_bricks, "textures/bricks3.jpg");
	add_asset_tag(&pack, AssetTag_vertical_wall, 0.5f);
	add_asset_tag(&pack, AssetTag_greyness, 0.0f);
	add_triangle_mesh_to_pack(&pack, AssetName_cube, "models/cube.obj", MeshFile_obj, VertexFormat_PosNormalUV);
	add_triangle_mesh_to_pack(&pack, AssetName_randy_mesh, "models/randy_static.obj", MeshFile_obj, VertexFormat_PosNormalUV);
	
	add_triangle_mesh_to_pack(&pack, AssetName_terrain_test, "models/terrain.obj", MeshFile_obj, VertexFormat_PosNormalTangentBitangentUV);

	add_triangle_mesh_to_pack(&pack, AssetName_randy_mesh_tangent, "models/randy_static.obj", MeshFile_obj, VertexFormat_PosNormalTangentBitangentUV);

	send_pack_to_file(&pack, "datas.khjr");
}

// TODO(flo): how do we handle meshes and materials?
int main(int arg_count, char **args) {
	init_test_package();
	return(0);
}
