#define KH_PACKAGE_SIGNATURE 0x6b686a72 // k h j r
#define KH_PACKAGE_VER 1

#define FOREACH_ASSET_TYPE_KEY(ASSETKEY) \
	ASSETKEY(AssetType_tex2d) \
	ASSETKEY(AssetType_font) \
	ASSETKEY(AssetType_trimesh) \
	ASSETKEY(AssetType_meshskin) \
	ASSETKEY(AssetType_animationskin) \
	ASSETKEY(AssetType_skeleton) \
	ASSETKEY(AssetType_animation) \
	ASSETKEY(AssetType_uncategorized) \

enum AssetTypeKey {
	FOREACH_ASSET_TYPE_KEY(GENERATE_ENUM)
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
		FontRange font;
		TriangleMesh trimesh;
		Skeleton skeleton;
		AnimationClip animation;
		AnimationSkin animskin;
	};
};

struct SourceAsset {
	u64 offset;
	u32 size;
	u32 first_tag;
	u32 one_past_last_tag;
	AssetType type;
};

struct BMPv3Header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct AssetContents {
	AssetType type;
	void *memory;
	u32 size;
};

KH_INTERN void
write_to_bmp_rgb(char *filename, u32 w, u32 h, u8* pixels) {
	u32 out_pixel_size = w * h * 3;
    BMPv3Header bmp_header = {};
    bmp_header.FileType = 0x4D42;
    bmp_header.FileSize = sizeof(bmp_header) + out_pixel_size;
    bmp_header.BitmapOffset = sizeof(bmp_header);
    bmp_header.Size = sizeof(bmp_header) - 14;
    bmp_header.Width = w;
    bmp_header.Height = h;
    bmp_header.Planes = 1;
    bmp_header.BitsPerPixel = 8 * 3;
    bmp_header.Compression = 0;
    bmp_header.SizeOfBitmap = out_pixel_size;
    bmp_header.HorzResolution = 0;
    bmp_header.VertResolution = 0;
    bmp_header.ColorsUsed = 0;
    bmp_header.ColorsImportant = 0;
    FileHandle out_file = g_platform.open_file(filename, FileAccess_write, FileCreation_create_or_open);
    kh_assert(!out_file.error);
    u32 offset = g_platform.write_bytes_to_file(&out_file, 0, sizeof(bmp_header), &bmp_header);
    kh_assert(offset == sizeof(bmp_header));
    g_platform.write_bytes_to_file(&out_file, offset, out_pixel_size, pixels);
    g_platform.close_file(&out_file);
}

KH_INTERN void
write_to_bmp_rgba(char *filename, u32 w, u32 h, u8* pixels) {
	u32 out_pixel_size = w * h * 4;
    BMPv3Header bmp_header = {};
    bmp_header.FileType = 0x4D42;
    bmp_header.FileSize = sizeof(bmp_header) + out_pixel_size;
    bmp_header.BitmapOffset = sizeof(bmp_header);
    bmp_header.Size = sizeof(bmp_header) - 14;
    bmp_header.Width = w;
    bmp_header.Height = h;
    bmp_header.Planes = 1;
    bmp_header.BitsPerPixel = 8 * 4;
    bmp_header.Compression = 0;
    bmp_header.SizeOfBitmap = out_pixel_size;
    bmp_header.HorzResolution = 0;
    bmp_header.VertResolution = 0;
    bmp_header.ColorsUsed = 0;
    bmp_header.ColorsImportant = 0;
    FileHandle out_file = g_platform.open_file(filename, FileAccess_write, FileCreation_create_or_open);
    kh_assert(!out_file.error);
    u32 offset = g_platform.write_bytes_to_file(&out_file, 0, sizeof(bmp_header), &bmp_header);
    kh_assert(offset == sizeof(bmp_header));
    g_platform.write_bytes_to_file(&out_file, offset, out_pixel_size, pixels);
    g_platform.close_file(&out_file);
}