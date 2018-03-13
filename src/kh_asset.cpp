KH_INTERN void
add_enum_to_hash(Assets *assets, char *arr[], u32 arr_count) {
    AssetEnumHash *hash = &assets->enum_hash;
    const u32 slot_count = array_count(hash->elements);
    kh_assert(is_pow2(slot_count));
    for(u32 i = 0; i < arr_count; ++i) {
        u32 str_len = string_length(arr[i]);
        u32 hash_key = hash_key_from_djb2(arr[i], str_len);
        u32 hash_slot = hash_key & (slot_count - 1);
        AssetEnumHashEl **first = hash->elements + hash_slot;
#if KH_IN_DEVELOPMENT
        if(*first) {
            hash->collision_count++; 
        } else {
            hash->remaining_slots--; 
        }
#endif
        AssetEnumHashEl *new_el = kh_push_struct(&assets->arena, AssetEnumHashEl);
        new_el->name = arr[i];
        new_el->value = i;
        new_el->next_in_hash = *first;
        *first = new_el;
    }
}

KH_INTERN u32
get_enum_value_from_hash(Assets *assets, char *name, u32 name_len) {
	AssetEnumHash *hash = &assets->enum_hash;
	const u32 slot_count = array_count(hash->elements);
	kh_assert(is_pow2(slot_count));
	u32 hash_key = hash_key_from_djb2(name, name_len);
	u32 hash_slot = hash_key & (slot_count - 1);

	AssetEnumHashEl **first = hash->elements + hash_slot;
	AssetEnumHashEl *find = 0;
	for(AssetEnumHashEl *search = *first; search; search = search->next_in_hash) {
		if(strings_equals_on_size(name_len, search->name, name)) {
			find = search;
			break;
		}
	}
	kh_assert(find);
	u32 res = find->value;
	return(res);
}

KH_INLINE u32
get_or_create_asset_id_from_name(Assets *assets, char *name, b32 should_not_exist = false) {
	u32 str_len = string_length(name);

	u32 hash_key = hash_key_from_djb2(name, str_len);
	u32 hash_ind = hash_key & (MAX_IN_GAME_ASSETS - 1);

	AdditionalAsset **first = assets->additional_assets + hash_ind;

	AdditionalAsset *find = 0;
#ifdef KH_DEBUG
	u32 i = 0;
#endif
	for(AdditionalAsset *search = *first; search; search = search->next_in_hash)
	{
		if(strings_equals_on_size(str_len, name, search->name))
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

		find = kh_push_struct(&assets->arena, AdditionalAsset);
		kh_assert(assets->one_past_last_asset_id <= assets->total_count);
		find->id = assets->one_past_last_asset_id++;
		find->name = (char *)kh_push_size_(&assets->arena, str_len + 1);
		strings_copy(str_len, name, find->name);

		find->next_in_hash = *first;
		*first = find;
#if defined(KH_EDITOR_MODE) || defined(KH_IN_DEVELOPMENT)
		assets->arr[find->id].cname = find->name;
#endif
	}

	kh_assert(find);
	u32 res = find->id;

	return(res);
}

KH_INLINE u32
get_asset_id_from_name(Assets *assets, char *name, u32 name_len) {
	u32 hash_key = hash_key_from_djb2(name, name_len);
	u32 hash_slot = hash_key & (MAX_IN_GAME_ASSETS - 1);

	char *test_str = "pathtracer_texture";
	u32 test = hash_key_from_djb2(test_str, string_length(test_str));


	AdditionalAsset **first = assets->additional_assets + hash_slot;
	AdditionalAsset *find = 0;
	for(AdditionalAsset *search = *first; search; search = search->next_in_hash)
	{
		if(strings_equals_on_size(name_len, name, search->name))
		{
			find = search;
			break;
		}
	}
	kh_assert(find);
	u32 res = find->id;
	return(res);
}

KH_INLINE b32
is_loaded(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	b32 res = (asset->state == AssetState_loaded);
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
	// res.data = assets->cache.base + asset->header.data_offset;
	res.data = asset->header.data;
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

KH_INLINE AnimationSkin
get_animation_skin(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr +id.val;
	kh_assert(asset_is_valid(asset, AssetType_animationskin));
	AnimationSkin res = asset->source.type.animskin;
	return(res);
}

KH_INLINE FontRange
get_font_range(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, AssetType_font));
	FontRange res = asset->source.type.font;
	return(res);
}

KH_INLINE u8*
get_datas(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	kh_assert(asset_is_valid(asset, expected_type));
	u8 *res = asset->header.data;
	return(res);
}

KH_INLINE Asset*
get_asset(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	Asset *res = assets->arr + id.val;
	kh_assert(asset_is_valid(res, expected_type));
	return(res);
}

KH_INLINE Asset *
get_unloaded_asset(Assets *assets, AssetID id, AssetTypeKey expected_type) {
	kh_assert(id.val <= assets->total_count);
	Asset *res = assets->arr + id.val;
	kh_assert(res->source.type.key == expected_type);
	return(res);
}

KH_INLINE u8*
get_datas_from_asset(Assets *assets, Asset *asset) {
	kh_assert(asset->state == AssetState_loaded);
	u8 *res = asset->header.data;
	return(res);
}

KH_INLINE u32
get_asset_state(Assets *assets, AssetID id) {
	kh_assert(id.val <= assets->total_count);
	Asset *asset = assets->arr + id.val;
	u32 res = asset->state;
	return(res);
}

// TODO(flo): we should allow the gpu to load directly the asset without loading it to the cpu if possible (mapbufferrange)
KH_INLINE u8 *
add_asset_to_data_cache(Assets *assets, Asset *asset, u32 size) {
	u8 *res = (u8 *)kh_push(&assets->cache.arena, size);
	asset->header.data = res;
	asset->header.gpu_index = INVALID_GPU_INDEX;
	// kh_assert(assets->cache.used + size < assets->cache.size);
	// u8 *res = assets->cache.base + assets->cache.used;
	// assets->cache.used += size;
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
	// kh_push(&res.arena, data_size);
	kh_reserve_size(&res.arena, data_size);
	return(res);
}

inline AssetFile *
get_file(Assets *assets, u32 file_index) {
	kh_assert(file_index < assets->file_count);
	AssetFile *res = assets->file_arr + file_index;
	return(res);
}

inline FileHandle *
get_file_handle(Assets *assets, u32 file_index) {
	FileHandle *res = &get_file(assets, file_index)->hdl;
	return(res);
}

// TODO(flo): implement this!
KH_INLINE void
set_asset_vector(Assets *assets, AssetTagKeys tag_key, f32 match, f32 weight) {
	assets->tag_vectors[tag_key].matches = match;
	assets->tag_vectors[tag_key].weights = weight;
}

KH_INTERN AssetID
get_best_asset(Assets *assets, AssetName name_key)
{
	AssetID res = {};
	AssetNameArray *name = assets->name_arr + name_key;
	for(u32 asset_ind = name->first_asset; asset_ind < name->one_past_last_asset; ++asset_ind)
	{
		Asset *cur_asset = assets->arr + asset_ind;
		for(u32 tag_ind = cur_asset->source.first_tag; tag_ind < cur_asset->source.one_past_last_tag; ++tag_ind)
		{
			AssetTag *tag = assets->tag_arr + tag_ind;
		}
	}
	NOT_IMPLEMENTED;
	kh_assert(res.val);
	return(res);
}

inline AssetID
get_first_asset(Assets *assets, AssetName name_key) {
	AssetID res = {};
	AssetNameArray *name = assets->name_arr + name_key;
	res.val = name->first_asset;
#ifdef KH_IN_DEVELOPMENT
	kh_assert(assets->arr[res.val].name == name_key);
#endif
	kh_assert(res.val);
	return(res);
}

JOB_ENTRY_POINT(load_asset_sched) {
	LoadAssetTask *task = (LoadAssetTask *)user_data;

	g_platform.read_bytes_of_file(task->hdl, task->offset, task->bytes_to_read, task->dst);

	COMPILER_WRITE_BARRIER;

	task->asset->state = task->final_state;

	end_sched_task_with_memory(task->mem_task);
}

KH_INTERN u32
get_or_create_empty_texture(Assets *assets, u32 w, u32 h, char *name) {
	u32 id = get_or_create_asset_id_from_name(assets, name);
	Asset *asset = assets->arr + id;
	if(asset->state != AssetState_loaded) {
		u32 bpp = 3;
		asset->source = {};
		asset->source.type.key = AssetType_tex2d;
		asset->state = AssetState_loaded;
		asset->source.type.tex2d.width           = w;
		asset->source.type.tex2d.height          = h;
		asset->source.type.tex2d.bytes_per_pixel = bpp;

		u32 size = w * h * bpp;
		u8 *dst = add_asset_to_data_cache(assets, asset, size);
	}
	return(id);
}

KH_INTERN void
set_texture_pixel_color(Assets *assets, AssetID id, u8 red, u8 green, u8 blue) {
	LoadedAsset tex2d = get_loaded_asset(assets, id, AssetType_tex2d);
	u8 *pixels = tex2d.data;
	Texture2D texture = get_texture(assets, id);
	for(u32 i = 0; i < texture.width*texture.height; ++i) {
		pixels[0] = blue;
		pixels[1] = green;
		pixels[2] = red;
		pixels += texture.bytes_per_pixel;
	}
	Asset *asset = get_asset(assets, id, AssetType_tex2d);
	asset->header.gpu_reload = true;
}

KH_INTERN AssetID
get_or_create_texture_single_color(Assets *assets, char *name, u32 w, u32 h, u8 red, u8 green, u8 blue) {
	AssetID id = {get_or_create_asset_id_from_name(assets, name)};
	Asset *asset = assets->arr + id.val;
	if(asset->state != AssetState_loaded) {
		get_or_create_empty_texture(assets, w, h, name);
		set_texture_pixel_color(assets, id, red, green, blue);
	}
	return(id);
}

// TODO(flo): we should allow the gpu to load the asset without passing by cpu memory (mapbufferrange);
KH_INTERN void
load_asset(Assets *assets, AssetID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val && id.val <= assets->count_from_package) {
		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) == AssetState_not_loaded)
		{
			TaskWithMemory *task = begin_sched_task_with_memory(assets->load_tasks, array_count(assets->load_tasks));
			if(task) {
				FileHandle *hdl = get_file_handle(assets, 0);
				SourceAsset *src = &asset->source;

				u32 size = src->size;
				u64 offset = src->offset;

				u8 *dst = add_asset_to_data_cache(assets, asset, size);
				LoadAssetTask *load_task = kh_push_struct(&task->arena, LoadAssetTask);
				load_task->mem_task = task;
				load_task->asset = assets->arr + id.val;
				load_task->hdl = get_file_handle(assets, 0);
				load_task->final_state = AssetState_loaded;
				load_task->bytes_to_read = size;
				load_task->offset = offset;
				load_task->dst = dst;

				g_platform.add_work_to_queue(assets->load_queue, load_asset_sched, load_task);
			}
			else {
				asset->state = AssetState_not_loaded;
			}
		}
	} else {
		kh_assert(!"this asset is not present in our package");	
	}
}

KH_INTERN void
load_asset_immediate(Assets *assets, AssetID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val && id.val <= assets->count_from_package) {
		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) == AssetState_not_loaded)
		{
			SourceAsset *src = &asset->source;

			u64 offset = src->offset;
			u32 size = src->size;

			u8 *dst = add_asset_to_data_cache(assets, asset, size);

			FileHandle *hdl = get_file_handle(assets, 0);
			g_platform.read_bytes_of_file(hdl, offset, size, dst);

			asset->state = AssetState_loaded;
		} else {
			kh_assert(!"we should not go there");
		}
	} else {
		kh_assert(!"this asset is not present in our package");	
	}
}

KH_INTERN b32
ask_for_asset(Assets *assets, AssetID id, b32 on_gpu = false) {
	// kh_assert(id.val);
	b32 res = false;
	u32 state = get_asset_state(assets, id); 
	if(state == AssetState_not_loaded) {
		load_asset(assets, id);
	} else if(state == AssetState_loaded) {
		res = true;
	} 
	return(res);
}

KH_INTERN void
ask_for_asset_force_immediate(Assets *assets, AssetID id) {
	kh_assert(id.val);
	Asset *asset = assets->arr + id.val;
	if(asset->state != AssetState_loaded) {
		load_asset_immediate(assets, id);
	}
}