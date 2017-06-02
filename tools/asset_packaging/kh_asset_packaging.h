#ifndef KH_ASSET_PACKAGING_H

struct TriangleMeshFileInfo {
	VertexFormat format;
};

struct FontFileInfo {
	FontParam *param;
};

#define MAX_TAG_COUNT 4096
#define MAX_ASSET_COUNT 4096

struct AssetInfo {
	u32 first_tag;
	u32 one_past_last_tag;
	AssetTypeKey type;
	char *filename;
	union {
		TriangleMeshFileInfo trimesh_info;
		FontFileInfo font_info;
	};
};

struct AssetNameInfo {
	u32 asset_count;
	u32 *arr_indices;
};

#define MAX_ASSETS_PER_NAME 256

#define MAX_ASSET_PER_FILE 32
#define MAX_FILES_IN_HASH 64


struct FileAsset {
	u32 asset_info_index;
	AssetContents contents;
};

struct FileImport {
	char *name;
	u32 asset_count;
	u32 loaded_asset_count;
	StackAllocator allocator;
	FileAsset assets[MAX_ASSET_PER_FILE];
	FileImport *next_in_hash;
};

struct LoadAssetResult {
	AssetContents *contents;
	FileImport *file;
};

struct FileHash {
	StackAllocator mem;
	FileImport *hash[MAX_FILES_IN_HASH];
};

struct AssetPackage {
	StackAllocator mem_infos;

	// sched_queue *queue;
	// memory_block mem_libs;

	u32 tag_count;
	AssetTag tag_arr[MAX_TAG_COUNT];

	u32 asset_type_count;
	AssetNameInfo names_info[AssetName_count];
	u32 used_type_ids[AssetName_count];

	u32 asset_count;
	AssetInfo assets_info[MAX_ASSET_COUNT];

	u32 total_font_glyphs;

	FileHash file_hash;

};

#define KH_ASSET_PACKAGING_H
#endif
