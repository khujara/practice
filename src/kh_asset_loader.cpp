KH_INTERN MaterialValue 
value_from_string(char *str, u32 str_len, u32 *advance) {
	MaterialValue res = MaterialValue_count;
	*advance = 0;
	if(strings_equals_on_size(str_len, "AssetID", str)) {
		res = MaterialValue_texture;
		*advance = sizeof(AssetID);
	} else if(strings_equals_on_size(str_len, "f32", str)) {
		res = MaterialValue_f32;
		*advance = sizeof(f32);
	} else if(strings_equals_on_size(str_len, "v4", str)) {
		res = MaterialValue_v4;
		*advance = sizeof(v4);
	}
	return(res);
}

KH_INTERN void
load_materials(Assets *assets, RenderManager *render, char *filename) {
	LinearArena tmp_arena = {};

	FileHandle file_hdl = g_platform.open_file(filename, FileAccess_read, FileCreation_only_open);	
	kh_assert(!file_hdl.error);
	u32 size = g_platform.get_file_size(&file_hdl);
	char *file_contents = (char *)kh_push(&tmp_arena, size);
	g_platform.read_bytes_of_file(&file_hdl, 0, size, file_contents);
	g_platform.close_file(&file_hdl);


	StringTokenizer str_tok = {file_contents};
	Token tok = get_token_and_next(&str_tok);

	// NOTE(flo): zero for Material_none
	u32 param_count = 1;
	while(!token_fit(tok, Token_end_of_file)) {
		if(token_fit(tok, Token_semicolon)) {
			param_count++;
		}
		tok = get_token_and_next(&str_tok);
	}

	render->mat_params_count = param_count;
	render->mat_params = kh_push_array(&assets->arena, param_count, MaterialParam);
	render->mat_params[0] = {};

	str_tok.pos = file_contents;
	tok = get_token_and_next(&str_tok);

	// NOTE(flo): zero for Material_none
	u32 mat_count = 1;
	u32 param_index = 1;
	char test_buffer[64];
	while(!token_fit(tok, Token_end_of_file)) {
		if(token_fit(tok, Token_colon)) {
			MaterialInfo *mat_info = render->mat_infos + mat_count++;
			mat_info->first_param_index = param_index;
			mat_info->param_count = 0;
			mat_info->texture_count = 0;
			mat_info->size = 0;

			tok = get_token_and_next(&str_tok);

			char *name = tok.text;
			umm name_len = tok.text_length;

			// TODO(flo): remove this once we're done with material generation
			kh_printf(test_buffer, "%.*s", name_len, name);
			kh_assert(strings_equals(G_MATERIAL_TYPES[mat_count-1], test_buffer));

			tok = get_token_and_next(&str_tok);
			kh_assert(token_fit(tok, Token_open_brace));
			tok = get_token_and_next(&str_tok);

			while(!token_fit(tok, Token_close_brace)) {
				MaterialParam *param = render->mat_params + param_index++;

				char *param_type = tok.text;
				umm param_type_len = tok.text_length;

				param->type = value_from_string(param_type, param_type_len, &param->advance_size);
				if(param->type == MaterialValue_texture) {
					mat_info->texture_count++;
				} else {
					mat_info->size += param->advance_size;
				}
				tok = get_token_and_next(&str_tok);

				char *param_name = tok.text;
				umm param_name_len = tok.text_length;

				param->name = (char *)kh_push(&assets->arena, param_name_len + 1);
				strings_copy(param_name_len, param_name, param->name);

				tok = get_token_and_next(&str_tok);
				kh_assert(token_fit(tok, Token_semicolon));
				mat_info->param_count++;

				tok = get_token_and_next(&str_tok);
			}
		}
		tok = get_token_and_next(&str_tok);
	}



	kh_clear(&tmp_arena);
}


KH_INTERN Assets *
load_assets_infos(char *filename, WorkQueue *work_queue, u32 cache_size, u32 additionnal_asset_count = 0)
{
	Assets *assets = 0;
	// NOTE(flo): check if the block for assets as already been loaded
	assets = boot_strap_push_struct(Assets, arena);
	for(u32 t_i = 0; t_i < array_count(assets->load_tasks); ++t_i)
	{
		TaskWithMemory *task = assets->load_tasks + t_i;
		task->unuse = true;
		task->arena = {};
	}

	assets->load_queue = work_queue;

	// TODO(flo): load all files of types .kh
	FileHandle file_hdl = g_platform.open_file(filename, FileAccess_read, FileCreation_only_open);

	assets->file_count = 1;
	assets->file_arr   = kh_push_array(&assets->arena, assets->file_count, AssetFile);
	assets->cache      = init_data_cache(cache_size);

	assets->primitive_names[Primitive_cube] = "cube";
	assets->primitive_names[Primitive_plane] = "plane";
	assets->primitive_names[Primitive_double_sided_plane] = "double_side_plane";
	assets->primitive_names[Primitive_sphere] = "sphere";

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
		assets->tag_arr = (AssetTag *)kh_push(&assets->arena, src_tag_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.tags_offset, src_tag_size, assets->tag_arr);

		u32 src_type_size = file->header.asset_type_count*sizeof(SourceAssetType); 
		file->asset_type_arr = (SourceAssetType *)kh_push(&assets->arena, src_type_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.asset_types_offset, src_type_size, file->asset_type_arr);
		for(u32 ind = 0; ind < file->header.asset_type_count; ++ind)
		{
			SourceAssetType *src = file->asset_type_arr + ind;

			if(src->type_id < AssetName_count)
			{
				AssetNameArray *dst = assets->name_arr + src->type_id;
				kh_assert(dst->first_asset == 0 && dst->one_past_last_asset == 0);
				dst->first_asset = src->first_asset;
				dst->one_past_last_asset = src->one_past_last_asset;
			}
		}

		assets->count_from_package     = file->header.asset_count;
		assets->count_additional       = additionnal_asset_count;
		assets->total_count            = assets->count_from_package + assets->count_additional;
		assets->arr                    = kh_push_array(&assets->arena, assets->total_count, Asset);
		assets->one_past_last_asset_id = assets->count_from_package;

		TransientLinear temp = kh_begin_transient(&assets->arena);
		u32 src_asset_size = sizeof(SourceAsset)*assets->count_from_package;
		SourceAsset *src_asset_arr = (SourceAsset *)kh_push(&assets->arena, src_asset_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.assets_offset, src_asset_size, src_asset_arr);
		for(u32 ind = 0; ind < assets->count_from_package; ++ind)
		{
			SourceAsset *src = src_asset_arr + ind;
			Asset *dst = assets->arr + ind;
			dst->source             = *src;
			dst->header.data 		= 0;
			dst->header.gpu_index   = INVALID_GPU_INDEX;
			dst->header.gpu_reload  = false;
			dst->state              = AssetState_not_loaded;
		}

#ifdef KH_IN_DEVELOPMENT
		kh_lu0(name_i, AssetName_count) {
			AssetNameArray *name = assets->name_arr + name_i;	
			kh_lu(asset_i, name->first_asset, name->one_past_last_asset) {
				Asset *asset = assets->arr + asset_i;
				asset->name = (AssetName)name_i;
			}
		}
#endif
		kh_end_transient(&temp);
	}
	create_dummy_asset(assets);

	#ifdef KH_EDITOR_MODE
	AssetEnumHash *hash = &assets->enum_hash;	
	#ifdef KH_IN_DEVELOPMENT
	hash->remaining_slots = array_count(hash->elements);
	#endif
	add_enum_to_hash(assets, G_ASSET_NAMES, array_count(G_ASSET_NAMES));
	add_enum_to_hash(assets, G_ASSET_TAGS, array_count(G_ASSET_TAGS));
	add_enum_to_hash(assets, G_ASSET_TYPES, array_count(G_ASSET_TYPES));
	add_enum_to_hash(assets, G_VERTEX_FORMATS, array_count(G_VERTEX_FORMATS));
	add_enum_to_hash(assets, G_SHADING_TYPES, array_count(G_SHADING_TYPES));
	add_enum_to_hash(assets, G_MATERIAL_TYPES, array_count(G_MATERIAL_TYPES));
	#endif
	
	return(assets);
}