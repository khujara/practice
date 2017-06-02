#ifndef KH_ASSET_H

// TODO(flo): we need to have different buffers that can fit to our vertex formats to
// so that we could have a map pointer directly to gpu. We'll need to split our datas and our headers
struct DataCache {
	// GeneralPurposeAllocator headers;
	GeneralPurposeAllocator buffer;

};

struct TextGroup
{
	f32 align_y;
	f32 at_y;
	f32 scale;

	v3 start_pos;
	v3 end_pos;

	u32 font_id;
};

enum AssetState
{
	AssetState_not_loaded,
	AssetState_in_queue,
	AssetState_loaded,
};

struct AssetNameArray
{
	u32 first_asset;
	u32 one_past_last_asset;
};

struct AssetFile
{
	FileHandle hdl;
	AssetFileHeader header;
	SourceAssetType *AssetType_arr;
};

struct AssetHeader {
	u32 data_offset;
	u32 gpu_index;
};

struct Asset
{
	AssetHeader header;
	u32 state;
	SourceAsset source;
}; 

struct LoadAssetTask
{
	TaskWithMemory *mem_task;
	Asset *asset;
	FileHandle *hdl;
	void *dst;

	u64 bytes_to_read;
	u64 offset;
	u32 final_state;
};

struct LoadedAsset {
	AssetType *type;
	u8 *data;
};

struct AssetFromFile
{
	u32 id;
	char *name;
	AssetFromFile *next_in_hash;
};

struct AssetTagVector {
	f32 matches[AssetTag_count];
	f32 weights[AssetTag_count];
};

#define MAX_IN_GAME_ASSETS 128
#define INVALID_GPU_INDEX 0xFFFFFFFF

struct Assets
{
	StackAllocator memory;

	u32 file_count;
	AssetFile *file_arr;
	
	u32 tag_count;
	AssetTag *tag_arr;


	u32 count_from_package;
	u32 count_from_files;
	u32 total_count;
	Asset *arr;

#ifdef KH_DEBUG
	u32 hash_collision_count;
#endif
	u32 one_past_last_asset_id;
	AssetFromFile *assets_from_game[MAX_IN_GAME_ASSETS];

	TaskWithMemory load_tasks[4];
	WorkQueue *load_queue;

	AssetNameArray name_arr[AssetName_count];

	AssetTagVector tag_vector;

	DataCache cache;
};

KH_INLINE u32
get_or_create_asset_id_from_name(Assets *assets, char *name, b32 should_not_exist = false)
{
	u32 hash_key = hash_key_from_djb2(name);
	u32 hash_ind = hash_key & (MAX_IN_GAME_ASSETS - 1);

	AssetFromFile **first = assets->assets_from_game + hash_ind;

	AssetFromFile *find = 0;
#ifdef KH_DEBUG
	u32 i = 0;
#endif
	for(AssetFromFile *search = *first; search; search = search->next_in_hash)
	{
		if(strings_equals(name, search->name))
		{
			find = search;
			break;
		}
#ifdef KH_DEBUG
		i++;
#endif
	}

	if(should_not_exist && find) {
		kh_assert(!"it seems that we're loading the same name twice for another asset");
	}

	if(!find)
	{
#ifdef KH_DEBUG	
	if(i > 0) assets->hash_collision_count++;
#endif

		u32 length = string_length(name);
		find = kh_push_struct(&assets->memory, AssetFromFile);
		kh_assert(assets->one_past_last_asset_id <= assets->total_count);
		find->id = assets->one_past_last_asset_id++;
		find->name = (char *)kh_push_size_(&assets->memory, length + 1);
		strings_copy(length, name, find->name);

		find->next_in_hash = *first;
		*first = find;
	}

	kh_assert(find);
	u32 res = find->id;
	return(res);
}

KH_INLINE b32 
asset_is_valid(Asset *asset, AssetTypeKey type) {
	b32 res = ((asset->state == AssetState_loaded) && (asset->source.type.key == type));
	return(res);
}

KH_INLINE LoadedAsset
get_loaded_asset(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	LoadedAsset res;
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, expected_type));
	res.type = &asset->source.type;
	res.data = assets->cache.buffer.base + asset->header.data_offset;
	return(res);
}

KH_INLINE Texture2D
get_texture(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	Texture2D res = asset->source.type.tex2d;
	return(res);
}

KH_INLINE TriangleMesh
get_trimesh(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, AssetType_trimesh));
	TriangleMesh res = asset->source.type.trimesh;
	return(res);
}

KH_INLINE Skeleton
get_skeleton(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr +id.val;
	kh_assert(asset_is_valid(asset, AssetType_skeleton));
	Skeleton res = asset->source.type.skeleton;
	return(res);
}

KH_INLINE AnimationClip
get_animation(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, AssetType_animation));
	AnimationClip res = asset->source.type.animation;
	return(res);
}

KH_INLINE u8*
get_datas(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, expected_type));
	u8 *res = assets->cache.buffer.base + asset->header.data_offset;
	return(res);
}

KH_INLINE Asset*
get_asset(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	Asset *res = assets->arr + id.val;
	kh_assert(asset_is_valid(res, expected_type));
	return(res);
}

KH_INLINE u8*
get_datas_from_asset(Assets *assets, Asset *asset) {
	u8 *res = assets->cache.buffer.base + asset->header.data_offset;
	return(res);
}

KH_INLINE u32
get_asset_state(Assets *assets, AssetID id)
{
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	u32 res = asset->state;
	return(res);
}

KH_INLINE u8 *
add_asset_to_data_cache(Assets *assets, Asset *asset, u32 size) {
	asset->header.data_offset = (u32)assets->cache.buffer.used;
	asset->header.gpu_index = INVALID_GPU_INDEX;
	u8 *res = (u8 *)kh_pack(&assets->cache.buffer, size);
	return(res);
}

KH_INLINE void
create_dummy_asset(Assets *assets) {
	Asset *asset = assets->arr + 0;
	asset->state = AssetState_loaded;
}

KH_INLINE DataCache
init_data_cache(umm data_size) {
	DataCache res;

	res.buffer.base = (u8 *)g_platform.virtual_alloc(data_size, 0);
	res.buffer.used = 0;
	res.buffer.size = data_size;

	return(res);
}

KH_INTERN b32 asset_loaded(Assets *assets, AssetID id);
KH_INTERN void asset_load_force_immediate(Assets *assets, AssetID id);


#define KH_ASSET_H 
#endif
