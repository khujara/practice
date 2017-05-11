#ifndef KH_ASSET_H

struct FontGlyph
{
	f32 xadvance;
	u32 x0, x1;
	u32 y0, y1;
	f32 xoff, yoff;
	f32 xoff1, yoff1;
};

struct FontInfos
{
	u32 tex_w, tex_h;
	u32 glyph_count;
	FontGlyph *glyphs;
	u32 *codepoints_map;
	f32 *kernel_advance;
	f32 advance_y;
};

struct text_group
{
	f32 align_y;
	f32 at_y;
	f32 scale;

	v3 start_pos;
	v3 end_pos;

	FontID f_id;
	FontInfos *f_infos;
};

enum AssetState
{
	AssetState_not_loaded,
	AssetState_in_queue,
	AssetState_loaded,
};

struct AssetType
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

struct Asset
{
	u32 state;
	SourceAsset *source;
	union
	{
		void *infos;
		FontInfos *f_infos;
	};
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

struct InGameAssets
{
	InGameAssets *next_in_hash;
	u32 id;
	char *name;
};

#define MAX_IN_GAME_ASSETS 128
#define IN_GAME_ASSETS_HASH_MODULO 127

struct Assets
{
	StackAllocator memory;

	u32 file_count;
	AssetFile *file_arr;
	
	u32 tag_count;
	AssetTag *tag_arr;

	u32 count;
	Asset *arr;

#ifdef KH_DEBUG
	u32 hash_collision_count;
#endif
	u32 one_past_last_asset;
	// TODO(flo): double tabling to avoid collision ?
	// TODO(flo): do not like the idea of __ingame__ assets
	InGameAssets *assets_from_game[MAX_IN_GAME_ASSETS];

	TaskWithMemory load_tasks[4];
	WorkQueue *load_queue;

	AssetType type_arr[AssetName_count];
};

KH_INLINE u32
get_AssetState(Assets *assets, u32 id)
{
	Asset *asset = assets->arr + id;
	kh_assert(asset);
	u32 res = asset->state;
	return(res);
}

KH_INLINE void *
get_asset_infos(Assets *assets, u32 id)
{
	Asset *asset = assets->arr + id;
	kh_assert(asset);
	void *res = asset->infos;
	kh_assert(res);
	return(res);
}

KH_INLINE FontInfos *
get_FontInfos(Assets *assets, u32 id)
{
	FontInfos *res = (FontInfos *)get_asset_infos(assets, id);
	return(res);
}

KH_INLINE u32
get_or_create_asset_id_from_string(Assets *assets, char *str)
{
	u32 hash_key = hash_key_from_djb2(str);
	u32 hash_ind = hash_key & IN_GAME_ASSETS_HASH_MODULO;

	InGameAssets **first = assets->assets_from_game + hash_ind;

	InGameAssets *find = 0;
#ifdef KH_DEBUG
	u32 i = 0;
#endif
	for(InGameAssets *search = *first; search; search = search->next_in_hash)
	{
		if(strings_equals(str, search->name))
		{
			find = search;
			break;
		}
#ifdef KH_DEBUG
		i++;
#endif
	}


	if(!find)
	{
#ifdef KH_DEBUG	
	if(i > 0) assets->hash_collision_count++;
#endif

		u32 length = string_length(str);
		find = kh_push_struct(&assets->memory, InGameAssets);
		find->id = assets->one_past_last_asset++;
		find->name = (char *)kh_push_size_(&assets->memory, length + 1);
		strings_copy(length, str, find->name);

		find->next_in_hash = *first;
		*first = find;
	}

	kh_assert(find);
	u32 res = find->id;
	return(res);
}

KH_INTERN struct TriangleMesh *load_tri_mesh_to_data_cache(struct DataCache *cache, u32 tri_count, u32 count, u32 interleave, u32 *out_size = 0);
KH_INTERN void load_font(Assets *assets, DataCache *cache, FontID id);
KH_INTERN void load_texture_2d(Assets *assets, DataCache *cache, TextureID id);
KH_INTERN void load_tri_mesh(Assets *assets, DataCache *cache, TriangleMeshID id);
KH_INTERN void load_texture_immediate(Assets *assets, DataCache *cache, TextureID id);
KH_INTERN void load_tri_mesh_immediate(Assets *assets, DataCache *cache, TriangleMeshID id);
KH_INLINE void create_dummy_texture(DataCache *cache);

#define KH_ASSET_H 
#endif
