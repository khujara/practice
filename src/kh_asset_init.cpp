KH_INTERN Assets *
load_assets_infos(WorkQueue *queue)
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

	assets->load_queue = queue;

// TODO(flo): load all files of types .kh
	FileHandle file_hdl = g_platform.open_file("datas.khjr", &assets->memory, FileAccess_read, FileCreation_only_open);

	assets->file_count = 1;
	assets->file_arr = kh_push_array(&assets->memory, assets->file_count, AssetFile);


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
				AssetType *dst = assets->type_arr + src->type_id;
				kh_assert(dst->first_asset == 0 && dst->one_past_last_asset == 0);
				dst->first_asset = src->first_asset;
				dst->one_past_last_asset = src->one_past_last_asset;
			}
		}

		assets->count = file->header.asset_count;
		assets->arr = kh_push_array(&assets->memory, assets->count, Asset);
		assets->one_past_last_asset = assets->count + 1;

		u32 src_asset_size = sizeof(SourceAsset)*assets->count;

		SourceAsset *src_asset_arr = (SourceAsset *)kh_push(&assets->memory, src_asset_size);
		g_platform.read_bytes_of_file(&file_hdl, file->header.assets_offset, src_asset_size, src_asset_arr);
		for(u32 ind = 0; ind < assets->count; ++ind)
		{
			SourceAsset *src = src_asset_arr + ind;
			Asset *dst = assets->arr + ind;
			dst->source = src;
			dst->state = AssetState_not_loaded;
			dst->infos = 0;
			if(src->format == AssetFileType_font)
			{
				dst->f_infos = kh_push_struct(&assets->memory, FontInfos);
				FontInfos *infos = dst->f_infos;
				SourceFont font_src = src->src_font;
				u32 glyph_count = font_src.glyph_count;
				u32 highest_cp = font_src.highest_codepoint;
				u32 kernel_size = sizeof(f32)*glyph_count*glyph_count;
				u32 SourceFontGlyph_size = glyph_count * sizeof(SourceFontGlyph);
				u32 map_size = (highest_cp + 1) * sizeof(u32);

				infos->tex_w = font_src.tex_w;
				infos->tex_h = font_src.tex_h;
				infos->glyph_count = glyph_count;
				infos->glyphs = (FontGlyph *)kh_push(&assets->memory, glyph_count * sizeof(FontGlyph));
				infos->codepoints_map = (u32 *)kh_push(&assets->memory, map_size);
				infos->advance_y = font_src.advance_y;
				infos->kernel_advance =  (f32 *)kh_push(&assets->memory, kernel_size);

				TransientStack temp = kh_begin_transient(&assets->memory);
				SourceFontGlyph *glyph_src_memory = (SourceFontGlyph *)kh_push(temp.stack, SourceFontGlyph_size);
				g_platform.read_bytes_of_file(&file_hdl, src->offset, SourceFontGlyph_size, glyph_src_memory);
				for(u32 glyph_ind = 0; glyph_ind < glyph_count; ++glyph_ind)
				{
					SourceFontGlyph *glyph_src = glyph_src_memory + glyph_ind;
					FontGlyph *glyph = infos->glyphs + glyph_ind;
					glyph->x0 = glyph_src->x0;
					glyph->x1 = glyph_src->x1;
					glyph->y0 = glyph_src->y0;
					glyph->y1 = glyph_src->y1;
					glyph->xoff = glyph_src->xoff;
					glyph->yoff = glyph_src->yoff;
					glyph->xoff1 = glyph_src->xoff1;
					glyph->yoff1 = glyph_src->yoff1;
					glyph->xadvance = glyph_src->advance_width;

					u32 codepoint = glyph_src->code_point;
					infos->codepoints_map[codepoint] = glyph_ind;
				}
				kh_end_transient(&temp);
			}
		}
	}
	return(assets);
}