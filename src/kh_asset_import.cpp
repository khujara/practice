#include "kh\kh_platform.h"

#define USE_STB_LIBRARY

#include "kh\kh_mikktspace.h"

#include "kh\import\kh_assimp_import.cpp"
#ifdef USE_STB_LIBRARY
#include "kh\import\kh_stb_import.cpp"
#endif

#include <stdlib.h>

Platform g_platform;

extern "C" InitAssetLoader *
init_asset_loader(Platform *platform) {
	g_platform = *platform;
	return(0);
}

extern "C" LoadTex2dFile *
load_tex2d_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size) {
	load_tex2d(contents, allocator, file_contents, file_size);
	return(0);
}

extern "C" LoadFontFile *
load_font_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size,
               u32 first_glyph, u32 last_glyph, f32 pixels_height, u32 texture_size) {
	load_font(contents, allocator, file_contents, file_size, first_glyph, last_glyph, pixels_height, texture_size);
	return(0);
}

extern "C" LoadTriMeshFile *
load_trimesh_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size, VertexFormat format) {
	Assimp::Importer importer;
	load_assimp_file(&importer, file_contents, file_size);
	load_trimesh(contents, allocator, importer.GetScene(), format);
	close_assimp_file(&importer);
	return(0);
}

extern "C" LoadSkeletonFile *
load_skeleton_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size) {
	Assimp::Importer importer;
	load_assimp_file(&importer, file_contents, file_size);
	load_skeleton_hierarchy(contents, allocator, importer.GetScene());
	close_assimp_file(&importer);
	return(0);
}

extern "C" LoadSkinFile *
load_skin_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size) {
	Assimp::Importer importer;
	load_assimp_file(&importer, file_contents, file_size);
	load_skin_for_trimesh(contents, allocator, importer.GetScene());
	close_assimp_file(&importer);
	return(0);
}

extern "C" LoadAnimationFile *
load_animation_file(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size) {
	Assimp::Importer importer;
	load_assimp_file(&importer, file_contents, file_size);
	load_animation_clip(contents, allocator, importer.GetScene());
	close_assimp_file(&importer);
	return(0);
}

extern "C" LoadAnimationForSkinFiles *
load_animation_for_skin_files(AssetContents *contents, LinearArena *allocator, void *file_contents, u32 file_size,
                              void *anim_file_contents, u32 anim_file_size) {
	Assimp::Importer importer;
	load_assimp_file(&importer, file_contents, file_size);
	Assimp::Importer anim_importer;
	load_assimp_file(&anim_importer, anim_file_contents, anim_file_size);
	load_animation_for_skin(contents, allocator, importer.GetScene(), anim_importer.GetScene());
	close_assimp_file(&anim_importer);
	close_assimp_file(&importer);
	return(0);
}