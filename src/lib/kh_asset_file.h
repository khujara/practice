#ifndef AssetFile_H

#define KH_PACKAGE_SIGNATURE 0x6b686a72 // k h j r
#define KH_PACKAGE_VER 1

enum AssetFileType {
	AssetFileType_texture_2d,
	AssetFileType_font,
	AssetFileType_triangle_mesh,
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

	AssetName_icosphere,
	AssetName_terrain_test,
	AssetName_cube,

	AssetName_font,

	AssetName_count,
};

#pragma pack(push, 1)
struct TextureID {
	u32 val;
};
struct FontID {
	u32 val;
};
struct TriangleMeshID {
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

struct SourceTexture2d {
	u32 width;
	u32 height;
	u32 bytes_per_pixel;
};

struct SourceTriangleMesh {
	u32 count;
	u32 tri_count;
	u32 interleave;
	VertexFormat format;
};

struct SourceFontGlyph {
	u32 code_point;
	f32 advance_width;
	u32 x0, x1;
	u32 y0, y1;
	f32 xoff, yoff;
	//TODO(flo): for what purposes ? 
	f32 xoff1, yoff1;
};

struct SourceFont {
	u32 glyph_count;
	u32 highest_codepoint;
	f32 advance_y;
	u32 tex_w;
	u32 tex_h;
	u32 tex_bytes_per_pixel;
};

struct SourceAsset {
	u64 offset;
	u32 first_tag;
	u32 one_past_last_tag;
	AssetFileType format;
	union {
		SourceTexture2d src_texture_2d;
		SourceFont src_font;
		SourceTriangleMesh src_tri_mesh;
	};
};
#pragma pack(pop)

#define AssetFile_H
#endif
