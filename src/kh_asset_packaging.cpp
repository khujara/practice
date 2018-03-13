#include "kh\kh_platform.h"
#include <windows.h>
#include "kh\kh_win32.h"


#define USE_STB_LIBRARY
// TODO(flo): DXT, ETC2, PVRTC and ASTC real-time de/compression

#include "kh_asset_names.h"
#include "kh_asset_packaging.h"

#include "kh\kh_mikktspace.h"
#include "kh\import\kh_assimp_import.cpp"
#ifdef USE_STB_LIBRARY
#include "kh\import\kh_stb_import.cpp"
#endif

#include <stdlib.h>

Assimp::Importer g_assimp_importer;

/*
	TODO(flo): 
		- ASSIMP seems not to work properly for generating tangent and bitangent
			of our models, recompute them once we need it (for now we compute
			them in real-time in the fragment shader) - use Mikktspace for better
			results!
*/
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
		find = kh_push_struct(&pack->arena, FileImport);
		find->name = (char *)kh_push(&pack->arena, length + 1);
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

KH_INTERN LoadAssetResult
asset_loaded_from_file(AssetPackage *pack, u32 asset_info_index) {
	LoadAssetResult res;
	res.contents = 0;
	res.file = 0;
	AssetInfo *first_info = pack->assets_info + asset_info_index;

	char *filename = first_info->filename;
	AssetTypeKey first_type = first_info->type;

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

	TransientLinear filecontents_alloc = kh_begin_transient(&pack->arena);
	kh_assert(find->loaded_asset_count < find->asset_count);
	if(find->loaded_asset_count == 0) {

		FileHandle f = win32_open_file(filename, FileAccess_read, FileCreation_only_open);
		kh_assert(!f.error);

		u32 file_size = win32_get_file_size(&f);
		void *file_contents = kh_push(&pack->arena, file_size);
		win32_read_bytes_of_file(&f, 0, file_size, file_contents);

		if(first_type == AssetType_trimesh ||
		   first_type == AssetType_meshskin ||
		   first_type == AssetType_skeleton ||
		   first_type == AssetType_animation) {
			load_assimp_file(&g_assimp_importer, file_contents, file_size);
			// if(first_type == AssetType_trimesh) {
			// 	if(has_skin(first_info->trimesh_info.format)) {
			// 		DEBUG_export_assimp_skeleton_hierarchy(&g_assimp_importer, filename, true);
			// 	}
			// } else {
			// 	DEBUG_export_assimp_skeleton_hierarchy(&g_assimp_importer, filename, true);
			// }
		}

		for(u32 i = 0; i < find->asset_count; ++i) {
			FileAsset *asset = find->assets + i;

			AssetInfo *info = pack->assets_info + asset->asset_info_index; 

			kh_assert(strings_equals(info->filename, filename));
			switch(info->type) {
				case AssetType_tex2d : {
					load_tex2d(&asset->contents, &find->arena, file_contents, file_size);
				} break;
				case AssetType_font : {
					u32 first_glyph_in_range = info->font_info.first_glyph_in_range;
					u32 last_glyph_in_range = info->font_info.last_glyph_in_range;
					f32 pixels_height = info->font_info.pixels_height;
					u32 tex_size = info->font_info.texture_size;
					load_font(&asset->contents, &find->arena, file_contents, file_size, 
					          first_glyph_in_range, last_glyph_in_range, pixels_height, tex_size);
				} break;
				case AssetType_trimesh : {
					VertexFormat format = info->trimesh_info.format;
					load_trimesh(&asset->contents, &find->arena, g_assimp_importer.GetScene(), format);
				} break;
				case AssetType_skeleton : {
					load_skeleton_hierarchy(&asset->contents, &find->arena, g_assimp_importer.GetScene());
				} break;
				case AssetType_meshskin : {
					load_skin_for_trimesh(&asset->contents, &find->arena, g_assimp_importer.GetScene());
				} break;
				case AssetType_animation : {	
					load_animation_clip(&asset->contents, &find->arena, g_assimp_importer.GetScene());	

				} break;
				case AssetType_animationskin : {
					char *anim_filename = info->animskin_info.anim_filename;


					u32 anim_hash_key = hash_key_from_djb2(anim_filename);
					u32 anim_hash_slot = anim_hash_key & (MAX_FILES_IN_HASH - 1);
					FileImport **first_anim = files->hash + anim_hash_slot; 

					FileImport *find_anim = 0;
					for(FileImport *search = *first_anim; search; search = search->next_in_hash) {
						if(strings_equals(anim_filename, search->name)) {
							find_anim = search;
							break;
						}
					}
					kh_assert(find_anim);

					AssetInfo *anim_info = 0;
					i32 anim_index = -1;
					for(u32 j = 0; j < find_anim->asset_count; ++j) {
						FileAsset *anim_asset = find_anim->assets + j;
						AssetInfo *anim_info_search = pack->assets_info + anim_asset->asset_info_index;
						if(anim_info_search->type == AssetType_animation) {
							anim_info = anim_info_search;
							anim_index = anim_asset->asset_info_index;
							break;
						}
					}
					kh_assert(anim_info);
					kh_assert(anim_index != -1);
					const aiScene *skin_scene = g_assimp_importer.GetScene();
					const aiScene *anim_scene = skin_scene;
					Assimp::Importer anim_importer;
					if(!strings_equals(filename, anim_filename)) {
						FileHandle anim_f = win32_open_file(anim_filename, FileAccess_read, FileCreation_only_open);
						kh_assert(!anim_f.error);
						u32 anim_file_size = win32_get_file_size(&anim_f);
						void *anim_file_contents = kh_push(&pack->arena, anim_file_size);
						win32_read_bytes_of_file(&anim_f, 0, anim_file_size, anim_file_contents);
						load_assimp_file(&anim_importer, anim_file_contents, anim_file_size);
						anim_scene = anim_importer.GetScene();
					}
					load_animation_for_skin(&asset->contents, &find->arena, skin_scene, anim_scene); 
					asset->contents.type.animskin.animation_id = anim_index;
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

	// TODO(flo): do we really need to do this ?
	close_assimp_file(&g_assimp_importer);
	kh_end_transient(&filecontents_alloc);

	kh_assert(res.file);
	kh_assert(res.contents);

	return(res);
}

KH_INTERN void
clear_contents_from_file(FileImport *file) {
	if(file->loaded_asset_count == file->asset_count) {
		kh_clear(&file->arena);
	}
}

KH_INTERN u32
add_asset_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	AssetNameInfo *name_info = pack->names_info + name_key;
	if(name_info->asset_count == 0) {
		
		kh_assert(!name_info->arr_indices);
		pack->used_type_ids[pack->asset_type_count++] = name_key;
		name_info->arr_indices = (u32 *)kh_push_array(&pack->arena, MAX_ASSETS_PER_NAME, u32);
	}
	kh_assert(name_info->asset_count < MAX_ASSETS_PER_NAME);
	u32 res = pack->asset_count++;
	add_asset_to_import_from_file(pack, filename, res);
	name_info->arr_indices[name_info->asset_count++] = res;
	AssetInfo *info = pack->assets_info + res;
	info->first_tag = 0;
	info->one_past_last_tag = 0;
	return(res);
}

KH_INTERN u32
add_tex2d_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_tex2d;
	info->filename = filename;
	return(res);}


KH_INTERN u32
add_skeleton_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_skeleton;
	info->filename = filename;
	return(res);
}

KH_INTERN u32
add_animation_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_animation;
	info->filename = filename;
	return(res);
}

// TODO(flo): use tag for this?
KH_INTERN u32
add_animation_for_skin_to_pack(AssetPackage *pack, AssetName name_key, char *skin_filename, char *anim_filename) {
	u32 res = add_asset_to_pack(pack, name_key, skin_filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_animationskin;
	info->filename = skin_filename;
	info->animskin_info.anim_filename = anim_filename;
	return(res);
}

KH_INTERN u32
add_skin_to_pack(AssetPackage *pack, AssetName name_key, char *filename) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_meshskin;
	info->filename = filename;
	return(res);
}

KH_INTERN u32
add_trimesh_to_pack(AssetPackage *pack, AssetName name_key, char *filename, VertexFormat format) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_trimesh;
	info->filename = filename;
	info->trimesh_info.format = format;
	return(res);
}

// #define ONE_PAST_MAX_FONT_CODEPOINT (0x10FFFF + 1)
#define MAX_GLYPHS_PER_FONT 4096

KH_INTERN u32
add_font_to_pack(AssetPackage *pack, AssetName name_key, char *filename, 
                 u32 first_glyph, u32 last_glyph, f32 pixels_height, u32 texture_size) {
	u32 res = add_asset_to_pack(pack, name_key, filename);
	AssetInfo *info = pack->assets_info + res;
	info->type = AssetType_font;
	info->filename = filename;
	info->font_info.pixels_height = pixels_height;
	info->font_info.first_glyph_in_range = first_glyph;
	info->font_info.last_glyph_in_range = last_glyph;
	info->font_info.texture_size = texture_size;
	return(res);
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

	res.arena = {};

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
	g_platform.open_file = win32_open_file;
	g_platform.close_file = win32_close_file;
	g_platform.get_file_size = win32_get_file_size;
	g_platform.read_bytes_of_file = win32_read_bytes_of_file;
	g_platform.write_bytes_to_file = win32_write_bytes_to_file;
	g_platform.virtual_alloc = win32_virtual_alloc;
	g_platform.virtual_free = win32_virtual_free;
	// TODO(flo): allocation to virtual memory here!
	AssetPackage pack = init_package();

	add_trimesh_to_pack(&pack, AssetName_randy_mesh_skinned, "data/models/randy.dae", VertexFormat_PNUTBS);
	add_skeleton_to_pack(&pack, AssetName_randy_skeleton, "data/models/randy.dae");
	add_skin_to_pack(&pack, AssetName_randy_skin, "data/models/randy.dae");
	add_animation_to_pack(&pack, AssetName_randy_idle, "data/models/randy.dae");
	add_animation_for_skin_to_pack(&pack, AssetName_randy_idle_skin, "data/models/randy.dae", "data/models/randy.dae");

	add_trimesh_to_pack(&pack, AssetName_lulu, "data/models/sk1_lulu_2.dae", VertexFormat_PNUS);
	add_skeleton_to_pack(&pack, AssetName_lulu_skeleton, "data/models/sk1_lulu_2.dae");
	add_skin_to_pack(&pack, AssetName_lulu_skin, "data/models/sk1_lulu_2.dae");
	add_animation_to_pack(&pack, AssetName_lulu_idle, "data/models/lulu_idle_coolmode.dae");
	add_animation_for_skin_to_pack(&pack, AssetName_lulu_idle_skin, "data/models/sk1_lulu_2.dae", "data/models/lulu_idle_coolmode.dae");
	add_animation_to_pack(&pack, AssetName_lulu_run, "data/models/sk1_lulu_run_coolMode_2.dae");
	add_animation_for_skin_to_pack(&pack, AssetName_lulu_run_skin, "data/models/sk1_lulu_2.dae", "data/models/sk1_lulu_run_coolMode_2.dae");

	add_font_to_pack(&pack, AssetName_font, "c:/Windows/Fonts/arial.ttf", ' ', '~', 38, 256);
	// add_tex2d_to_pack(&pack, AssetName_bricks, "data/textures/bricks2.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 0.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 0.0f);

	// add_tex2d_to_pack(&pack, AssetName_bricks, "data/textures/bricks.jpg");
	// add_asset_tag(&pack, AssetTag_vertical_wall, 1.0f);
	// add_asset_tag(&pack, AssetTag_greyness, 1.0f);
	add_tex2d_to_pack(&pack, AssetName_bricks, "data/textures/bricks4.jpg");

	add_tex2d_to_pack(&pack, AssetName_bricks_normal, "data/textures/bricks4_nmap.jpg");

	add_tex2d_to_pack(&pack, AssetName_wall, "data/textures/brickwall.jpg");
	add_tex2d_to_pack(&pack, AssetName_wall_normal, "data/textures/brickwall_nmap.jpg");


	add_tex2d_to_pack(&pack, AssetName_skybox_back, "data/textures/skybox/back.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_bottom, "data/textures/skybox/bottom.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_front, "data/textures/skybox/front.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_left, "data/textures/skybox/left.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_right, "data/textures/skybox/right.jpg");
	add_tex2d_to_pack(&pack, AssetName_skybox_top, "data/textures/skybox/top.jpg");

	add_trimesh_to_pack(&pack, AssetName_cube, "data/models/cube.dae", VertexFormat_PNU);
	add_trimesh_to_pack(&pack, AssetName_icosphere, "data/models/icosphere.dae", VertexFormat_PNU);

	add_tex2d_to_pack(&pack, AssetName_bricks, "data/textures/bricks3.jpg");
	add_asset_tag(&pack, AssetTag_vertical_wall, 0.5f);
	add_asset_tag(&pack, AssetTag_greyness, 0.0f);

	add_trimesh_to_pack(&pack, AssetName_randy_mesh, "data/models/randy_static.dae", VertexFormat_PNU);
	
	add_trimesh_to_pack(&pack, AssetName_terrain_test, "data/models/terrain.dae", VertexFormat_PNUTB);

	add_trimesh_to_pack(&pack, AssetName_randy_mesh_tangent, "data/models/randy_static.dae", VertexFormat_PNUTB);

	add_tex2d_to_pack(&pack, AssetName_lulu_color_chart, "data/textures/lulu_color_chart.png");

	// TODO(flo): real-time compression ?
	add_tex2d_to_pack(&pack, AssetName_randy_albedo, "data/textures/Randy_albedo.jpg");
	add_tex2d_to_pack(&pack, AssetName_randy_normal, "data/textures/Randy_nmap.jpg");
	add_tex2d_to_pack(&pack, AssetName_randy_ao, "data/textures/Randy_ao.jpg");
	add_tex2d_to_pack(&pack, AssetName_randy_gloss, "data/textures/Randy_gloss2.jpg");
	add_tex2d_to_pack(&pack, AssetName_randy_metalness, "data/textures/Randy_metalness2.jpg");

	add_tex2d_to_pack(&pack, AssetName_gold_albedo, "data/textures/pbr_test/gold/albedo.png");
	add_tex2d_to_pack(&pack, AssetName_gold_normal, "data/textures/pbr_test/gold/normal.png");
	add_tex2d_to_pack(&pack, AssetName_gold_ao, "data/textures/pbr_test/gold/ao.png");
	add_tex2d_to_pack(&pack, AssetName_gold_roughness, "data/textures/pbr_test/gold/roughness.png");
	add_tex2d_to_pack(&pack, AssetName_gold_metallic, "data/textures/pbr_test/gold/metallic.png");

	send_pack_to_file(&pack, "data/datas.khjr");
}

// TODO(flo): how do we handle meshes and materials?
int main(int arg_count, char **args) {
	init_test_package();
	return(0);
}
