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
	assets->tag_vector.matches[tag_key] = match;
	assets->tag_vector.weights[tag_key] = weight;
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
			int debugp = 4;
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
	kh_assert(res.val);
	return(res);
}

JOB_ENTRY_POINT(load_asset_sched) {
	LoadAssetTask *task = (LoadAssetTask *)user_data;

	g_platform.read_bytes_of_file(task->hdl, task->offset, task->bytes_to_read, task->dst);

	COMPILER_WRITE_BARRIER;

	task->asset->state = task->final_state;

	end_sched_TaskWithMemory(task->mem_task);
}

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
				LoadAssetTask *load_task = kh_push_struct(&task->stack, LoadAssetTask);
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
	TriangleMesh res;

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
asset_loaded(Assets *assets, AssetID id) {
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
asset_load_force_immediate(Assets *assets, AssetID id) {
	kh_assert(id.val);
	Asset *asset = assets->arr + id.val;
	if(asset->state != AssetState_loaded) {
		load_asset_immediate(assets, id);
	}
}







// TODO(flo): re-enable this we need to load the glyph infos (equivalent of the font infos) here
#if 0
KH_INTERN void
load_font(Assets *assets, FontID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val && id.val <= assets->count_from_file) {

		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) 
		   == AssetState_not_loaded) 
		{
			TaskWithMemory *task = begin_sched_task_with_memory(assets->load_tasks, array_count(assets->load_tasks));
			if(task) {

				FileHandle *hdl = get_file_handle(assets, 0);
				FontInfos *infos = asset->f_infos;
				SourceAsset *src = &asset->source;
				Font font_src = src->font;

				u64 offset = src->offset;
				u32 glyph_count = font_src.glyph_count;
				u32 source_font_glyph_size = glyph_count * sizeof(FontGlyph);
				u32 kernel_size = sizeof(f32)*glyph_count*glyph_count;
				u32 add_offset = source_font_glyph_size + kernel_size;

				u32 texture_size = font_src.tex_w * font_src.tex_h * font_src.tex_bytes_per_pixel;

				DataCache *cache = &assets->cache;
				add_asset_to_data_cache(asset, cache);

				// TODO(flo): do something clever with our buffer object 
				// TODO(flo) IMPORTANT(flo): we are not thread safe here
				u8 *dst = (u8 *)kh_pack(&cache->buffer, texture_size);

				// u64 data_offset = offset + add_offset;
				// g_platform.read_bytes_of_file(hdl, data_offset, total_datas_size, texture->memory);

				LoadAssetTask *load_task = kh_push_struct(&task->stack, LoadAssetTask);
				load_task->mem_task = task;
				load_task->asset = assets->arr + id.val;
				load_task->hdl = get_file_handle(assets, 0);
				load_task->final_state = AssetState_loaded;
				load_task->bytes_to_read = texture_size;
				load_task->offset = offset + add_offset;
				load_task->dst = dst;
				g_platform.add_work_to_queue(assets->load_queue, load_asset_sched, load_task);

			}
		}
	}
}
#endif

