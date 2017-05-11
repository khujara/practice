#ifndef KH_ASSET_PACKAGING_H

struct TriangleMeshContents {
	u32 tri_count;
	u32 vertices_count;
	u32 indices_count;
	VertexFormat format;
	u8 *vertices;
	u8 *indices;
};

struct Texture2DContents {
	u32 width;
	u32 height;
	u32 bytes_per_pixel;
	void *memory;
	void *contents;
};

struct Texture2dFileInfo {
	char *filename;
};

enum MeshFileType {
	MeshFile_obj,
	MeshFile_dae,
};

struct TriangleMeshFileInfo {
	char *filename;
	MeshFileType mft;
	VertexFormat format;
};

struct FontParam {
	char *fontname;
	f32 pixels_height;

	u32 max_glyph_count;
	u32 glyph_count;
	// TODO(flo): do we need this ????
	// u32 *glyph_index_from_cp;
	u32 *cp_arr;
};

struct FontFileInfo {
	char *filename;
	FontParam *param;
};

#define MAX_TAG_COUNT 4096
#define MAX_ASSET_COUNT 4096

struct AssetFileInfo {
	u32 first_tag;
	u32 one_past_last_tag;
	AssetFileType filetype;
	union {
		Texture2dFileInfo tex_2d_info;
		TriangleMeshFileInfo tri_mesh_info;
		FontFileInfo font_info;
	};
};

struct AssetTypeInfo {
	u32 asset_count;
	u32 *arr_indices;
};

#define MAX_ASSETS_PER_TYPE 256

struct asset_package {
	StackAllocator mem_infos;

	// sched_queue *queue;
	// memory_block mem_libs;

	u32 tag_count;
	AssetTag tag_arr[MAX_TAG_COUNT];

	u32 asset_type_count;
	AssetTypeInfo types_info[AssetName_count];
	u32 used_type_ids[AssetName_count];

	u32 asset_count;
	AssetFileInfo assets_info[MAX_ASSET_COUNT];

	u32 total_font_glyphs;
};

#define KH_ASSET_PACKAGING_H
#endif
