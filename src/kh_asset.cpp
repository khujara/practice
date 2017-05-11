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

#if 0
// TODO(flo): implement this!
KH_INTERN u32
get_best_asset(Assets *assets, AssetName name_key, f32 *match, f32 *weight)
{
	AssetType *type = assets->type_arr + name_key;
	for(u32 asset_ind = type->first_asset; asset_ind < type->one_past_last_asset; ++asset_ind)
	{
		Asset *cur_asset = assets->arr + asset_ind;
		for(u32 tag_ind = cur_src->.first_tag; tag_ind < cur_src->.one_past_last_tag; ++tag_ind)
		{
			AssetTag *tag = assets->tag_arr + tag_ind;
		}
	}
}
#endif

inline u32
get_first_asset(Assets *assets, AssetName name_key) {
	u32 res = 0;
	AssetType *type = assets->type_arr + name_key;
	res = type->first_asset;
	return(res);
}

inline TextureID
get_first_texture_2d(Assets *assets, AssetName name_key) {
	TextureID res = {get_first_asset(assets, name_key)};
	return(res);
}

inline FontID
get_first_font(Assets *assets, AssetName name_key) {
	FontID res = {get_first_asset(assets, name_key)};
	return(res);
}

inline TriangleMeshID
get_first_TriangleMeshID(Assets *assets, AssetName name_key) {
	TriangleMeshID res = {get_first_asset(assets, name_key)};
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
load_font(Assets *assets, DataCache *cache, FontID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val) {

		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) 
		   == AssetState_not_loaded) 
		{
			TaskWithMemory *task = begin_sched_TaskWithMemory(assets->load_tasks, array_count(assets->load_tasks));
			if(task) {

				FileHandle *hdl = get_file_handle(assets, 0);
				FontInfos *infos = asset->f_infos;
				SourceAsset *src = asset->source;
				SourceFont font_src = src->src_font;

				u64 offset = src->offset;
				u32 glyph_count = font_src.glyph_count;
				u32 SourceFontGlyph_size = glyph_count * sizeof(SourceFontGlyph);
				u32 kernel_size = sizeof(f32)*glyph_count*glyph_count;
				u32 add_offset = SourceFontGlyph_size + kernel_size;

				u32 texture_size = font_src.tex_w * font_src.tex_h * font_src.tex_bytes_per_pixel;

				add_handle_to_data_cache(cache, id.val);

				// TODO(flo): do something clever with our buffer object 
				// TODO(flo) IMPORTANT(flo): we are not thread safe here
				Texture2D *texture = (Texture2D *)kh_pack(&cache->buffer, sizeof(Texture2D) + texture_size);
				texture->width = font_src.tex_w;
				texture->height = font_src.tex_h;
				texture->bytes_per_pixel = font_src.tex_bytes_per_pixel;
				texture->memory = (u8 *)texture + sizeof(Texture2D);

				// u64 data_offset = offset + add_offset;
				// g_platform.read_bytes_of_file(hdl, data_offset, total_datas_size, texture->memory);

				LoadAssetTask *load_task = kh_push_struct(&task->stack, LoadAssetTask);
				load_task->mem_task = task;
				load_task->asset = assets->arr + id.val;
				load_task->hdl = get_file_handle(assets, 0);
				load_task->final_state = AssetState_loaded;
				load_task->bytes_to_read = texture_size;
				load_task->offset = offset + add_offset;
				load_task->dst = texture->memory;
				g_platform.add_work_to_queue(assets->load_queue, load_asset_sched, load_task);

			}
		}
	}
}

// @TODO(flo): general purpose allocator
KH_INTERN void
load_texture_2d(Assets *assets, DataCache *cache, TextureID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val) {
		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) == AssetState_not_loaded)
		{
			TaskWithMemory *task = begin_sched_TaskWithMemory(assets->load_tasks, array_count(assets->load_tasks));
			if(task) {
				FileHandle *hdl = get_file_handle(assets, 0);
				SourceAsset *src = asset->source;
				SourceTexture2d texture_src = src->src_texture_2d;

				u32 width = texture_src.width;
				u32 height = texture_src.height;
				u32 bytes_per_pixel = texture_src.bytes_per_pixel;

				u64 offset = src->offset;
				u32 data_size = (width * bytes_per_pixel) * height;

				add_handle_to_data_cache(cache, id.val);

				// TODO(flo): do something clever with our buffer object 
				Texture2D *texture = (Texture2D *)kh_pack(&cache->buffer, sizeof(Texture2D) + data_size);
				texture->width = width;
				texture->height = height;
				texture->bytes_per_pixel = bytes_per_pixel;
				texture->memory = (u8 *)texture + sizeof(Texture2D);

				LoadAssetTask *load_task = kh_push_struct(&task->stack, LoadAssetTask);
				load_task->mem_task = task;
				load_task->asset = assets->arr + id.val;
				load_task->hdl = get_file_handle(assets, 0);
				load_task->final_state = AssetState_loaded;
				load_task->bytes_to_read = data_size;
				load_task->offset = offset;
				load_task->dst = texture->memory;

				g_platform.add_work_to_queue(assets->load_queue, load_asset_sched, load_task);
			}
			else {
				asset->state = AssetState_not_loaded;
			}
		}
	}
}

KH_INTERN void
load_tri_mesh(Assets *assets, DataCache *cache, TriangleMeshID id) {
	Asset *asset = assets->arr + id.val;
	if(id.val) {
		if(interlocked_compare_exchange_u32(&asset->state, AssetState_in_queue, AssetState_not_loaded) == AssetState_not_loaded)
		{
			TaskWithMemory *task = begin_sched_TaskWithMemory(assets->load_tasks, array_count(assets->load_tasks));
			if(task) {
				FileHandle *hdl = get_file_handle(assets, 0);

				SourceAsset *src = asset->source;
				SourceTriangleMesh tri_mesh_src = src->src_tri_mesh;
				add_handle_to_data_cache(cache, id.val);

				u64 offset = src->offset;
				u32 tri_count       = tri_mesh_src.tri_count;
				u32 vert_count      = tri_mesh_src.count;
				u32 interleave      = tri_mesh_src.interleave;
				VertexFormat format = tri_mesh_src.format;

				u32 indices_size = tri_count * 3 * sizeof(u32);
				u32 vertex_size = get_size_from_vertex_format(format);
				u32 vertices_size = vertex_size * vert_count;

				u32 total_size = indices_size + vertices_size; 
				TriangleMesh *tri_mesh = (TriangleMesh *)kh_pack(&cache->buffer, sizeof(TriangleMesh) + total_size);
				tri_mesh->vert_c         = vert_count;
				tri_mesh->tri_c          = tri_count;
				tri_mesh->indices_offset = vertices_size;
				tri_mesh->format         = format;
				tri_mesh->vertex_size    = vertex_size;
				tri_mesh->memory         = (u8 *)tri_mesh + sizeof(TriangleMesh);

				LoadAssetTask *load_task = kh_push_struct(&task->stack, LoadAssetTask);
				load_task->mem_task      = task;
				load_task->asset         = assets->arr + id.val;
				load_task->hdl           = get_file_handle(assets, 0);
				load_task->final_state   = AssetState_loaded;
				load_task->bytes_to_read = total_size;
				load_task->offset        = offset;
				load_task->dst           = tri_mesh->memory;

				g_platform.add_work_to_queue(assets->load_queue, load_asset_sched, load_task);
			}
			else {
				asset->state = AssetState_not_loaded;
			}
		}
	}
}

KH_INTERN void
load_tri_mesh_immediate(Assets *assets, DataCache *cache, TriangleMeshID id) {
	TriangleMesh res;

	Asset *asset = assets->arr + id.val;
	if(asset->state == AssetState_not_loaded) {
		SourceAsset *src = asset->source;
		SourceTriangleMesh tri_mesh_src = src->src_tri_mesh;

		add_handle_to_data_cache(cache, id.val);

		u64 offset = src->offset;
		u32 tri_count       = tri_mesh_src.tri_count; 
		u32 vert_count      = tri_mesh_src.count;
		u32 interleave      = tri_mesh_src.interleave;
		VertexFormat format = tri_mesh_src.format;

		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 vertex_size = get_size_from_vertex_format(tri_mesh_src.format);
		u32 vertices_size = vertex_size * vert_count;

		u32 total_size = vertices_size + indices_size;

		TriangleMesh *tri_mesh = (TriangleMesh *)kh_pack(&cache->buffer, sizeof(TriangleMesh) + total_size);
		tri_mesh->vert_c         = vert_count;
		tri_mesh->tri_c          = tri_count;
		tri_mesh->indices_offset = vertices_size;
		tri_mesh->format         = format;
		tri_mesh->vertex_size    = vertex_size;
		tri_mesh->memory         = (u8 *)tri_mesh + sizeof(TriangleMesh);

		FileHandle *hdl = get_file_handle(assets, 0);
		g_platform.read_bytes_of_file(hdl, offset, total_size, tri_mesh->memory);

		asset->state = AssetState_loaded;
	} else {
		kh_assert(!"we should not go there");
	}
}

KH_INTERN void
load_texture_immediate(Assets *assets, DataCache *cache, TextureID id) {

	Asset *asset = assets->arr + id.val;
	if(asset->state == AssetState_not_loaded) {
		SourceAsset *src = asset->source;
		SourceTexture2d texture_src = src->src_texture_2d;
		add_handle_to_data_cache(cache, id.val);

		u64 offset = src->offset;
		u32 data_size = (texture_src.width * texture_src.bytes_per_pixel) * texture_src.height;

		Texture2D *texture = (Texture2D *)kh_pack(&cache->buffer, sizeof(Texture2D) + data_size);
		texture->width           = texture_src.width;
		texture->height          = texture_src.height;
		texture->bytes_per_pixel = texture_src.bytes_per_pixel;
		texture->memory          = (u8 *)texture + sizeof(Texture2D);
		FileHandle *hdl = get_file_handle(assets, 0);
		g_platform.read_bytes_of_file(hdl, offset, data_size, texture->memory);

		asset->state = AssetState_loaded;
	} else {
		kh_assert(!"we should not go there");
	}
}

KH_INLINE void
create_dummy_texture(DataCache *cache) {
	add_handle_to_data_cache(cache, 0);
	Texture2D *texture = (Texture2D *)kh_pack(&cache->buffer, sizeof(Texture2D));
	texture->width = 0;
	texture->height = 0;
	texture->bytes_per_pixel = 0;
	texture->memory = 0;
}

