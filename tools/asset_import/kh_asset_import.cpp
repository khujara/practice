#include "kh\kh_platform.h"

#define USE_STB_LIBRARY
#include "kh\import\kh_assimp_import.cpp"
#ifdef USE_STB_LIBRARY
#include "kh\import\kh_stb_import.cpp"
#endif

#include <stdlib.h>

Assimp::Importer g_assimp_importer;
Platform g_platform;

extern "C" InitAssetLoader *
init_asset_loader(Platform *platform) {
	g_platform = *platform;
	return(0);
}

extern "C" LoadTex2dFile *
load_tex2d_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size) {
	load_tex2d(contents, allocator, file_contents, file_size);
	return(0);
}

extern "C" LoadFontFile *
load_font_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size, FontParam *f_param) {
	load_font(contents, allocator, file_contents, file_size, f_param);
	return(0);
}

extern "C" LoadTriMeshFile *
load_trimesh_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size, VertexFormat format) {
	load_assimp_file(&g_assimp_importer, file_contents, file_size);
	load_trimesh(contents, allocator, g_assimp_importer.GetScene(), format);
	close_assimp_file(&g_assimp_importer);
	return(0);
}

extern "C" LoadSkeletonFile *
load_skeleton_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size) {
	load_assimp_file(&g_assimp_importer, file_contents, file_size);
	AssimpFileInfo fileinfo = get_assimp_file_info(g_assimp_importer.GetScene());
	load_skeleton_hierarchy(contents, allocator, g_assimp_importer.GetScene(), &fileinfo);
	close_assimp_file(&g_assimp_importer);
	return(0);
}

extern "C" LoadSkinFile *
load_skin_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size) {
	load_assimp_file(&g_assimp_importer, file_contents, file_size);
	AssimpFileInfo fileinfo = get_assimp_file_info(g_assimp_importer.GetScene());
	load_skin_for_trimesh(contents, allocator, g_assimp_importer.GetScene(), &fileinfo);
	close_assimp_file(&g_assimp_importer);
	return(0);
}

extern "C" LoadAnimationFile *
load_animation_file(AssetContents *contents, StackAllocator *allocator, void *file_contents, u32 file_size) {
	load_assimp_file(&g_assimp_importer, file_contents, file_size);
	AssimpFileInfo fileinfo = get_assimp_file_info(g_assimp_importer.GetScene());
	load_animation_clip(contents, allocator, g_assimp_importer.GetScene(), &fileinfo);
	close_assimp_file(&g_assimp_importer);
	return(0);
}