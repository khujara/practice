#ifndef AssetFile_H

#define KH_PACKAGE_SIGNATURE 0x6b686a72 // k h j r
#define KH_PACKAGE_VER 1

enum AssetTypeKey {
	AssetType_tex2d,
	AssetType_font,
	AssetType_trimesh,
	AssetType_meshskin,
	AssetType_skeleton,
	AssetType_animation,
	AssetType_uncategorized,
};

enum AssetTagKeys {
	AssetTag_vertical_wall,
	AssetTag_greyness,

	AssetTag_count,
};

enum AssetName {
	AssetName_bricks,
	AssetName_bricks_normal,

	AssetName_wall,
	AssetName_wall_normal,

	AssetName_randy_albedo,
	AssetName_randy_normal,
	
	AssetName_skybox_back,
	AssetName_skybox_bottom,
	AssetName_skybox_front,
	AssetName_skybox_left,
	AssetName_skybox_right,
	AssetName_skybox_top,

	AssetName_randy_mesh,
	AssetName_randy_mesh_tangent,
	AssetName_randy_mesh_skinned,

	AssetName_randy_skeleton,
	AssetName_randy_skin,
	AssetName_randy_idle,

	AssetName_icosphere,
	AssetName_terrain_test,
	AssetName_cube,

	AssetName_font,

	AssetName_count,
};

#pragma pack(push, 1)
struct AssetID {
	u32 val;
};


struct AssetFileHeader {
	u32 signature;
	u32 version;

	u32 tag_count;
	u32 asset_type_count;
	u32 asset_count;

	u64 tags_offset;
	u64 asset_types_offset;
	u64 assets_offset;
};

struct AssetTag {
	u32 key;
	f32 val;
};

struct SourceAssetType {
	u32 type_id;
	u32 first_asset;
	u32 one_past_last_asset;
};

struct AssetType {
	AssetTypeKey key;
	union {
		Texture2D tex2d;
		Font font;
		TriangleMesh trimesh;
		Skeleton skeleton;
		AnimationClip animation;
	};
};

struct SourceAsset {
	u64 offset;
	u32 size;
	u32 first_tag;
	u32 one_past_last_tag;
	AssetType type;
};
#pragma pack(pop)

struct AssetContents {
	AssetType type;
	void *memory;
	u32 size;
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

#define AssetFile_H
#endif
