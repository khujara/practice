#include "kh\kh_platform.h"
#include <windows.h>
#include "kh\kh_win32.h"

#define USE_STB_LIBRARY
// TODO(flo): DXT, ETC2, PVRTC and ASTC real-time de/compression

#include "kh_asset_packaging.h"


#include "kh\import\kh_assimp_import.cpp"
#ifdef USE_STB_LIBRARY
#include "kh\import\kh_stb_import.cpp"
#endif

#include <stdlib.h>

Assimp::Importer g_assimp_importer;

KH_INTERN void
add_asset_to_import_from_file(AssetPackage *pack, char *filename, u32 asset_info) {

	FileHash *files = &pack->file_hash;

	u32 hash_key = hash_key_from_djb2(filename);
	u32 hash_slot = hash_key & (MAX_FILES_IN_HASH - 1);


	FileImport **first = files->hash + hash_slot;

	FileImport *find = 0;
	for(FileImport *search = *first; search; search = search->next_in_hash) {
		if(strings_equals(filename, search->name)) {
			find = search;
			break;
		}
	}

	if(!find) {
		u32 length = string_length(filename);
		find = kh_push_struct(&files->mem, FileImport);
		find->name = (char *)kh_push(&files->mem, length + 1);
		strings_copy(length, filename, find->name);
		find->next_in_hash = *first;
		find->asset_count = 0;
		find->loaded_asset_count = 0;
		*first = find;
	}
	kh_assert(find->asset_count < MAX_ASSET_PER_FILE);

	FileAsset *asset = find->assets + find->asset_count++;
	asset->asset_info_index = asset_info;
	asset->contents.memory = 0;
	asset->contents.size = 0;
	asset->contents.type = {};
}

KH_INLINE b32
load_with_assimp(AssetTypeKey type) {
	b32 res = ((type == AssetType_trimesh) || 
	           (type == AssetType_meshskin) || 
	           (type == AssetType_skeleton) || 
	           (type == AssetType_animation));
	return(res);
}

KH_INTERN LoadAssetResult
asset_loaded_from_file(AssetPackage *pack, u32 asset_info_index) {
	LoadAssetResult res;
	res.contents = 0;
	res.file = 0;

	char *filename = pack->assets_info[asset_info_index].filename;

	FileHash *files = &pack->file_hash;
	
	u32 hash_key = hash_key_from_djb2(filename);
	u32 hash_slot = hash_key & (MAX_FILES_IN_HASH - 1);

	FileImport **first = files->hash + hash_slot;

	FileImport *find = 0;
	for(FileImport *search = *first; search; search = search->next_in_hash) {
		if(strings_equals(filename, search->name)) {
			find = search;
			break;
		}
	}

	kh_assert(find);

	StackAllocator file_allocator = {};

	kh_assert(find->loaded_asset_count < find->asset_count);
	if(find->loaded_asset_count == 0) {
		FileHandle f = win32_open_file(filename, &file_allocator, FileAccess_read, FileCreation_only_open);
		u32 file_size = win32_get_file_size(&f);
		void *file_contents = kh_push(&file_allocator, file_size);
		win32_read_bytes_of_file(&f, 0, file_size, file_contents);
		kh_assert(!f.error);

		AssetTypeKey type = pack->assets_info[asset_info_index].type;
		b32 is_assimp = load_with_assimp(type);

		AssimpFileInfo assimp_file;
		if(is_assimp) {
			load_assimp_file(&g_assimp_importer, file_contents, file_size);
			if(type != AssetType_trimesh) {
				assimp_file = get_assimp_file_info(g_assimp_importer.GetScene());
			}

		} else {
			// NOTE(flo)): for now this functionnality is only available for assimp file
			kh_assert(find->asset_count == 1);
		}

		for(u32 i = 0; i < find->asset_count; ++i) {
			FileAsset *asset = find->assets + i;

			AssetInfo *info = pack->assets_info + asset->asset_info_index; 

			kh_assert(strings_equals(info->filename, filename));
			switch(info->type) {
				case AssetType_tex2d : {
					load_tex2d(&asset->contents, &find->allocator, file_contents, file_size);
				} break;
				case AssetType_font : {
					FontParam *f_param = info->font_info.param;
					load_font(&asset->contents, &find->allocator, file_contents, file_size, f_param);
				} break;
				case AssetType_trimesh : {
					assimp_file = load_trimesh(&asset->contents, &find->allocator, g_assimp_importer.GetScene(), info->trimesh_info.format);
				} break;
				case AssetType_meshskin : {
					load_skin_for_trimesh(&asset->contents, &find->allocator, g_assimp_importer.GetScene(), &assimp_file);
				} break;
				case AssetType_skeleton : {
					load_skeleton_hierarchy(&asset->contents, &find->allocator, g_assimp_importer.GetScene(), &assimp_file);
				} break;
				case AssetType_animation : {
					load_animation_clip(&asset->contents, &find->allocator, g_assimp_importer.GetScene(), &assimp_file);
				} break;
				default : {
					kh_assert(!"this type of asset cannot be packaged");
				} break;
			}


			if(asset->asset_info_index == asset_info_index) {
				res.contents = &asset->contents;
			}
		}
		// TODO(flo): close file and free
	} else {
		for(u32 i = 0; i < find->asset_count; ++i) {
			FileAsset *asset = find->assets + i;
			if(asset->asset_info_index == asset_info_index) {
				res.contents = &asset->contents;
				break;
			}
		}
	}
	find->loaded_asset_count++;

	res.file = find;

	close_assimp_file(&g_assimp_importer);
	kh_clear(&file_allocator);

	kh_assert(res.file);
	kh_assert(res.contents);

	return(res);
}

KH_INTERN void
clear_contents_from_file(FileImport *file) {
	if(file->loaded_asset_count == file->asset_count) {
		kh_clear(&file->allocator);
	}
}

KH_INTERN AssetInfo *
add_asset_to_pack(AssetPackage *pack, char *filename, AssetName name_key) {
	AssetNameInfo *name_info = pack->names_info + name_key;
	if(name_info->asset_count == 0) {
		
		kh_assert(!name_info->arr_indices);
		pack->used_type_ids[pack->asset_type_count++] = name_key;
		name_info->arr_indices = (u32 *)kh_push_array(&pack->mem_infos, MAX_ASSETS_PER_NAME, u32);
	}
	kh_assert(name_info->asset_count < MAX_ASSETS_PER_NAME);
	u32 index = pack->asset_count++;
	add_asset_to_import_from_file(pack, filename, index);
	name_info->arr_indices[name_info->asset_count++] = index;
	AssetInfo *res = pack->assets_info + index;
	res->first_tag = 0;
	res->one_past_last_tag = 0;
	return(res);
}

KH_INTERN void
add_tex2d_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_tex2d;
	info->filename = filename;
}

KH_INTERN void
add_skeleton_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_skeleton;
	info->filename = filename;
}

KH_INTERN void
add_animation_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_animation;
	info->filename = filename;
}

KH_INTERN void
add_skin_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_meshskin;
	info->filename = filename;
}

KH_INTERN void
add_trimesh_to_pack(AssetPackage *pack, AssetName name_key, char *filename, VertexFormat format) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_trimesh;
	info->filename = filename;
	info->trimesh_info.format = format;
}

// #define ONE_PAST_MAX_FONT_CODEPOINT (0x10FFFF + 1)
#define MAX_GLYPHS_PER_FONT 4096

KH_INTERN AssetInfo *
add_font_to_pack(AssetPackage *pack, AssetName name_key, char *filename, char *fontname, f32 pixels_height) {
	AssetInfo *info = add_asset_to_pack(pack, filename, name_key);
	info->type = AssetType_font;
	info->filename = filename;
	info->font_info.param = (FontParam *)kh_push_struct(&pack->mem_infos, FontParam);
	info->font_info.param->fontname = fontname;
	info->font_info.param->pixels_height = pixels_height;
	info->font_info.param->glyph_count = 0;
	info->font_info.param->max_glyph_count = MAX_GLYPHS_PER_FONT;
	info->font_info.param->cp_arr = (u32 *)kh_push_array(&pack->mem_infos, info->font_info.param->max_glyph_count, u32);
	return(info);
}

KH_INTERN void
add_glyph_to_font(AssetInfo *f_info, u32 code_point, AssetName name_key) {
	kh_assert(f_info->filename);
	kh_assert(f_info->font_info.param->pixels_height > 0);
	kh_assert(f_info->font_info.param->glyph_count < f_info->font_info.param->max_glyph_count);
	u32 glyph_ind = f_info->font_info.param->glyph_count++;
	f_info->font_info.param->cp_arr[glyph_ind] = code_point;
}

KH_INTERN void
add_asset_tag(AssetPackage *pack, AssetTagKeys key, f32 val) {
	AssetInfo *info = pack->assets_info + (pack->asset_count - 1);
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
send_pack_to_file(AssetPackage *pack, char *filename) {
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
			u32 type_id = pack->used_type_ids[type_ind];
			AssetNameInfo *name_info = pack->names_info + type_id;
			kh_assert(name_info->asset_count > 0);
			SourceAssetType *type = type_arr + type_ind;
			type->type_id = type_id;
			type->one_past_last_asset = asset_count;
			type->first_asset = type->one_past_last_asset;
			for(u32 asset_ind = 0; asset_ind < name_info->asset_count; ++asset_ind) {
				u32 arr_ind = name_info->arr_indices[asset_ind];
				AssetInfo *asset_info = pack->assets_info + arr_ind;
				u32 ind = type->one_past_last_asset++;
				SourceAsset *asset = asset_arr + ind;
				asset->offset = ftell(pack_file);
				asset->first_tag = asset_info->first_tag;
				asset->one_past_last_tag = asset_info->one_past_last_tag;
				asset->type.key = asset_info->type;
				LoadAssetResult load_res = asset_loaded_from_file(pack, arr_ind);
				asset->type = load_res.contents->type;
				asset->size = load_res.contents->size;
				fwrite(load_res.contents->memory, load_res.contents->size, 1, pack_file);
				clear_contents_from_file(load_res.file);
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

KH_INTERN AssetPackage
init_package() {
	AssetPackage res;

	// arr_index mem_infos_size = megabytes(64);
	// arr_index mem_asset_size = MAX_TEMP_MEM_PER_ASSET;
	// TODO(flo): platform specific
	// void *mem = VirtualAlloc(0, mem_infos_size + mem_asset_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);//win32_virtual_alloc(mem_infos_size + mem_asset_size);

	res.mem_infos = {};

	res.tag_count = 1;
	res.asset_count = 1;
	res.asset_type_count = 0;
	res.file_hash.mem = {};
	return(res);
}

Platform g_platform;

/* TODO(flo): 
	foreach file keep in memory everything we need to load from it
	so we load all the contents when we load the first asset that needs it
	and we free all the contents when the last asset from this file is loaded

	for this purpose it seems we need :
		a hash table for the file name
		for each file a counter of how many asset we need
		for each file a counter of how many asset we have loaded
		what we need to load 
*/
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
	g_platform.get_file_size = win32_get_file_size;
	g_platform.read_bytes_of_file = win32_read_bytes_of_file;
	g_platform.write_bytes_to_file = win32_write_bytes_to_file;
	g_platform.virtual_alloc = win32_virtual_alloc;
	g_platform.virtual_free = win32_virtual_free;
	// TODO(flo): allocation to virtual memory here!
	AssetPackage pack = init_package();
	#if 0
	add_tex2d_to_pack(&pack, AssetType_randy_normal, "Randy_nmap.jpg");

	add_tex2d_to_pack(&pack, AssetType_randy_occlusion, "Randy_ao.jpg");

	add_tex2d_to_pack(&pack, AssetType_randy_emission, "Randy_gloss.jpg");

	add_tex2d_to_pack(&pack, AssetType_randy_metal, "Randy_metalness2.jpg");

	// add_trimesh_to_pack(&pack, AssetType_randy_mesh, "randy.dae");

	add_material_to_pack(&pack, AssetType_randy_mat_pbr, AssetType_randy_albedo, AssetType_randy_normal, AssetType_randy_occlusion,
		AssetType_randy_emission, AssetType_randy_metal);
	#endif
	/* TODO(flo) : animations!
		collada should gives us back a list of animations that can be treated separately

		add_animation_to_pack(&pack, AssetType_randy_idle, "randy.dae", "idle");
	*/

	// add_tex2d_to_pack(&pack, AssetType_first_bmp, "textures/4squared.bmp");



	u32 count = (u32)('~' - '!') + 1;
	AssetInfo *arial_info = add_font_to_pack(&pack, AssetName_font, "c:/Windows/Fonts/arial.ttf", "Arial", 42);
	add_glyph_to_font(arial_info, ' ', AssetName_font);
	for(u32 c = '!'; c <= '~'; ++c)	{
		add_glyph_to_font(arial_info, c, AssetName_font);
	}
	// add_tex2d_to_pack(&pack, AssetName_bricks, "textures/bricks2.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 0.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 0.0f);

	// add_tex2d_to_pack(&pack, AssetName_bricks, "textures/bricks.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 1.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 1.0f);
	add_tex2d_to_pack(&pack, AssetName_bricks, "textures/bricks4.jpg");

	add_tex2d_to_pack(&pack, AssetName_bricks_normal, "textures/bricks4_nmap.jpg");

	add_tex2d_to_pack(&pack, AssetName_wall, "textures/brickwall.jpg");
	add_tex2d_to_pack(&pack, AssetName_wall_normal, "textures/brickwall_nmap.jpg");

	// add_tex2d_to_pack(&pack, AssetName_first_png, "textures/SteamArcher01.png");

	// TODO(flo): real-time compression ?
	add_tex2d_to_pack(&pack, AssetName_randy_albedo, "textures/Randy_albedo.jpg");
	add_tex2d_to_pack(&pack, AssetName_randy_normal, "textures/Randy_nmap.jpg");
	// add_tex2d_to_pack(&pack, AssetName_first_bmp, "..\\4squared.bmp");


	add_tex2d_to_pack(&pack, AssetName_skybox_back, "textures/skybox/back.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_bottom, "textures/skybox/bottom.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_front, "textures/skybox/front.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_left, "textures/skybox/left.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_right, "textures/skybox/right.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_top, "textures/skybox/top.jpg");

	add_trimesh_to_pack(&pack, AssetName_icosphere, "models/icosphere.obj", VertexFormat_PosNormalUV);

	add_tex2d_to_pack(&pack, AssetName_bricks, "textures/bricks3.jpg");
	add_asset_tag(&pack, AssetTag_vertical_wall, 0.5f);
	add_asset_tag(&pack, AssetTag_greyness, 0.0f);

	add_trimesh_to_pack(&pack, AssetName_cube, "models/cube.obj", VertexFormat_PosNormalUV);
	add_trimesh_to_pack(&pack, AssetName_randy_mesh, "models/randy_static.obj", VertexFormat_PosNormalUV);
	
	add_trimesh_to_pack(&pack, AssetName_terrain_test, "models/terrain.obj", VertexFormat_PosNormalTangentBitangentUV);

	add_trimesh_to_pack(&pack, AssetName_randy_mesh_tangent, "models/randy_static.obj", VertexFormat_PosNormalTangentBitangentUV);

	add_trimesh_to_pack(&pack, AssetName_randy_mesh_skinned, "models/randy.dae", VertexFormat_PosNormalTangentBitangentUVSkinned);

	add_skin_to_pack(&pack, AssetName_randy_skin, "models/randy.dae");
	add_skeleton_to_pack(&pack, AssetName_randy_skeleton, "models/randy.dae");
	add_animation_to_pack(&pack, AssetName_randy_idle, "models/randy.dae");

	send_pack_to_file(&pack, "datas.khjr");
}

// TODO(flo): how do we handle meshes and materials?
int main(int arg_count, char **args) {
	init_test_package();
	return(0);
}
