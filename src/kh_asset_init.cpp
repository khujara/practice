KH_INTERN Assets *
load_assets_infos(char *filename, WorkQueue *work_queue, u32 cache_size, u32 additionnal_asset_count = 0)
{
	Assets *assets = 0;
	// NOTE(flo): check if the block for assets as already been loaded
	StackAllocator boot_strap = {};
	assets = kh_push_struct(&boot_strap, Assets);
	assets->memory = boot_strap;
	for(u32 t_i = 0; t_i < array_count(assets->load_tasks); ++t_i)
	{
		TaskWithMemory *task = assets->load_tasks + t_i;
		task->unuse = true;
		task->stack = {};
	}

	assets->load_queue = work_queue;

// TODO(flo): load all files of types .kh
	FileHandle file_hdl = g_platform.open_file(filename, &assets->memory, FileAccess_read, FileCreation_only_open);

	assets->file_count = 1;
	assets->file_arr = kh_push_array(&assets->memory, assets->file_count, AssetFile);
	assets->cache = init_data_cache(cache_size);

	kh_assert(!file_hdl.error);
	if(!file_hdl.error)
	{
		AssetFile *file = assets->file_arr + 0;
		file->hdl = file_hdl;
		g_platform.read_bytes_of_file(&file_hdl, 0, sizeof(AssetFileHeader), &file->header);

		kh_assert(file->header.signature == KH_PACKAGE_SIGNATURE);
		kh_assert(file->header.version == KH_PACKAGE_VER);

		assets->tag_count = file->header.tag_count;
		u32 src_tag_size = sizeof(AssetTag)*assets->tag_count; 
		assets->tag_arr = (AssetTag *)kh_push(&assets->memory, src_tag_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.tags_offset, src_tag_size, assets->tag_arr);

		u32 src_type_size = file->header.asset_type_count*sizeof(SourceAssetType); 
		file->AssetType_arr = (SourceAssetType *)kh_push(&assets->memory, src_type_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.asset_types_offset, src_type_size, file->AssetType_arr);
		for(u32 ind = 0; ind < file->header.asset_type_count; ++ind)
		{
			SourceAssetType *src = file->AssetType_arr + ind;

			if(src->type_id < AssetName_count)
			{
				AssetNameArray *dst = assets->name_arr + src->type_id;
				kh_assert(dst->first_asset == 0 && dst->one_past_last_asset == 0);
				dst->first_asset = src->first_asset;
				dst->one_past_last_asset = src->one_past_last_asset;
			}
		}

		assets->count_from_package = file->header.asset_count;
		assets->count_from_files = additionnal_asset_count;
		assets->total_count = assets->count_from_package + assets->count_from_files;
		assets->arr = kh_push_array(&assets->memory, assets->total_count, Asset);
		assets->one_past_last_asset_id = assets->count_from_package + 1;

		TransientStack temp = kh_begin_transient(&assets->memory);
		u32 src_asset_size = sizeof(SourceAsset)*assets->count_from_package;
		SourceAsset *src_asset_arr = (SourceAsset *)kh_push(&assets->memory, src_asset_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.assets_offset, src_asset_size, src_asset_arr);
		for(u32 ind = 0; ind < assets->count_from_package; ++ind)
		{
			SourceAsset *src = src_asset_arr + ind;
			Asset *dst = assets->arr + ind;
			dst->source = *src;
			dst->header.data_offset = INVALID_U32_OFFSET;
			dst->header.gpu_index = INVALID_GPU_INDEX;
			dst->state = AssetState_not_loaded;
		}
		kh_end_transient(&temp);
	}
	create_dummy_asset(assets);
	
	return(assets);
}