#define MAX_TRI_MESH_RENDER_TASK 2568
// static u32 g_asked_tri;
// static u32 g_clip_tri;
// static u32 g_cull_tri;
// static u32 g_rasterized_tri;

inline void
debug_draw_texture(SoftwarePixelsBuffer *target, Texture2D *tex, v3 pos) {
	u32 min_x = (u32)pos.x;
	u32 min_y = (u32)pos.y;
	u32 max_x = min_x + tex->width;
	u32 max_y = min_y + tex->height;

	u8 *dst_row = (u8 *)target->memory + min_x * FRAME_BUFFER_BYTES_PER_PIXEL + min_y * target->pitch;;
	u8 *src_row = (u8 *)tex->memory;
	for(u32 y = min_y; y < max_y; ++y) {
		u32 *dst_pixel = (u32 *)dst_row;;
		for(u32 x = min_x; x < max_x; ++x) {
			u8 b = *src_row++;
			*dst_pixel++ = ((u32)(b << 24) | (u32)(b << 16) | (u32)(b << 8) | (u32)(b << 0));
		}

		dst_row += target->pitch;
	}
}

KH_INLINE void
tri_mesh_render(SoftwareFrameBuffer *target, f32 min_x, f32 max_x, f32 min_y, f32 max_y, DataCache *cache, u32 mesh_off, u32 diffuse_off, VertexAttribute *attrib, DirectionalLight *light, mat4 mvp, mat4 wld, v4 color) {

	DataHashElement *mesh_el = cache->hash + mesh_off;
	const TriangleMesh *mesh = get_datas(cache, mesh_el, TriangleMesh);

	u32 tri_count = 0;

	__m128 clip_min_x = _mm_set1_ps(min_x);
	__m128 clip_max_x = _mm_set1_ps(max_x);
	__m128 clip_min_y = _mm_set1_ps(min_y);
	__m128 clip_max_y = _mm_set1_ps(max_y);

	__m128 half_w_SSE = _mm_set1_ps((f32)target->pixels.w * 0.5f);
	__m128 half_h_SSE = _mm_set1_ps((f32)target->pixels.h * 0.5f);
	__m128 mvp_r0 = _mm_set_ps(mvp.m00, mvp.m10, mvp.m20, mvp.m30);
	__m128 mvp_r1 = _mm_set_ps(mvp.m01, mvp.m11, mvp.m21, mvp.m31);
	__m128 mvp_r2 = _mm_set_ps(mvp.m02, mvp.m12, mvp.m22, mvp.m32);
	__m128 mvp_r3 = _mm_set_ps(mvp.m03, mvp.m13, mvp.m23, mvp.m33);

	__m128 normal_x[3];
	__m128 normal_y[3];
	__m128 normal_z[3];
	__m128 proj_x[3];
	__m128 proj_y[3];
	__m128 proj_z[3];
	__m128 proj_w[3];

	Vertex_SSE in[3];

	u8 *vertices = (u8 *)mesh->memory;
	u32 *indices = (u32 *)(mesh->memory + mesh->indices_offset);

	for(u32 tri_i = 0; tri_i < mesh->tri_c; ++tri_i) {
		// g_asked_tri++;
		u32 *tri_ind = indices + tri_i * 3;
		u8* vertex_0 = vertices + (tri_ind[0] * attrib->vertex_size);
		u8 *vertex_1 = vertices + (tri_ind[1] * attrib->vertex_size);
		u8 *vertex_2 = vertices + (tri_ind[2] * attrib->vertex_size);
		v3 *pos_0 = (v3 *)(vertex_0 + attrib->pos_offset);
		v3 *pos_1 = (v3 *)(vertex_1 + attrib->pos_offset);
		v3 *pos_2 = (v3 *)(vertex_2 + attrib->pos_offset);
		__m128 p_0 = _mm_set_ps(pos_0->x, pos_0->y, pos_0->z, 1.0f);
		__m128 p_1 = _mm_set_ps(pos_1->x, pos_1->y, pos_1->z, 1.0f);
		__m128 p_2 = _mm_set_ps(pos_2->x, pos_2->y, pos_2->z, 1.0f);

		__m128 proj_0 = _mm_hadd_ps(_mm_hadd_ps(_mm_mul_ps(p_0, mvp_r0), _mm_mul_ps(p_0, mvp_r1)), 
		                            _mm_hadd_ps(_mm_mul_ps(p_0, mvp_r2), _mm_mul_ps(p_0, mvp_r3)));
		__m128 proj_1 = _mm_hadd_ps(_mm_hadd_ps(_mm_mul_ps(p_1, mvp_r0), _mm_mul_ps(p_1, mvp_r1)), 
		                            _mm_hadd_ps(_mm_mul_ps(p_1, mvp_r2), _mm_mul_ps(p_1, mvp_r3)));
		__m128 proj_2 = _mm_hadd_ps(_mm_hadd_ps(_mm_mul_ps(p_2, mvp_r0), _mm_mul_ps(p_2, mvp_r1)), 
		                            _mm_hadd_ps(_mm_mul_ps(p_2, mvp_r2), _mm_mul_ps(p_2, mvp_r3)));

		f32 *proj_0_ptr = (f32 *)&proj_0;
		f32 *proj_1_ptr = (f32 *)&proj_1;
		f32 *proj_2_ptr = (f32 *)&proj_2;

		__m128 px = _mm_set_ps(*(proj_0_ptr + 0), *(proj_2_ptr + 0), *(proj_1_ptr + 0), *(proj_0_ptr + 0));
		__m128 py = _mm_set_ps(*(proj_0_ptr + 1), *(proj_2_ptr + 1), *(proj_1_ptr + 1), *(proj_0_ptr + 1));
		__m128 pz = _mm_set_ps(*(proj_0_ptr + 2), *(proj_2_ptr + 2), *(proj_1_ptr + 2), *(proj_0_ptr + 2));
		__m128 pw = _mm_set_ps(*(proj_0_ptr + 3), *(proj_2_ptr + 3), *(proj_1_ptr + 3), *(proj_0_ptr + 3));
		/*NOTE(flo): 
		xndc = xc / wc, yndc = yc / wc, zndc = zc / wc
		we just skip the ndc and divide after we do have the need to follow graphics card rule for now.
		*/
		// NOTE(flo): xscreen = (xc * width/2 + wc * width/2) / wc;
		__m128 sx = _mm_div_ps(_mm_add_ps(_mm_mul_ps(px, half_w_SSE), _mm_mul_ps(pw, half_w_SSE)), pw);
		__m128 sy = _mm_div_ps(_mm_add_ps(_mm_mul_ps(py, half_h_SSE), _mm_mul_ps(pw, half_h_SSE)), pw);

		__m128 clip_x = _mm_and_ps(_mm_cmple_ps(sx, clip_max_x), _mm_cmpge_ps(sx, clip_min_x));
		__m128 clip_y = _mm_and_ps(_mm_cmple_ps(sy, clip_max_y), _mm_cmpge_ps(sy, clip_min_y));
		__m128 clip = _mm_and_ps(clip_x, clip_y);
		__m128i clip_mask = *(__m128i *)(&clip);
		b32 one_inside = !_mm_test_all_zeros(clip_mask, _mm_set1_epi32(0xFFFFFFFF)); 
	
		if(one_inside) {
			// g_clip_tri++;
			f32 x1mx0 = sx.m128_f32[2] - sx.m128_f32[1];
			f32 x2mx0 = sx.m128_f32[3] - sx.m128_f32[1];
			f32 y1my0 = sy.m128_f32[2] - sy.m128_f32[1];
			f32 y2my0 = sy.m128_f32[3] - sy.m128_f32[1];

			f32 triangle_area = (x1mx0 * y2my0)  - (x2mx0 * y1my0);
			b32 back_face_cull = triangle_area >= 0;
			if(back_face_cull) {
				// g_cull_tri++;
				__m128 abs_mask = _mm_set1_ps(-0.f); //1 << 31
				__m128 abs_0 = _mm_andnot_ps(abs_mask, proj_0);
				__m128 abs_1 = _mm_andnot_ps(abs_mask, proj_1);
				__m128 abs_2 = _mm_andnot_ps(abs_mask, proj_2);

				__m128 w_0 = _mm_set1_ps(proj_0.m128_f32[3]);
				__m128 w_1 = _mm_set1_ps(proj_1.m128_f32[3]);
				__m128 w_2 = _mm_set1_ps(proj_2.m128_f32[3]);

				__m128 mask_0_ps = _mm_cmple_ps(abs_0, w_0);
				__m128 mask_1_ps = _mm_cmple_ps(abs_1, w_1);
				__m128 mask_2_ps = _mm_cmple_ps(abs_2, w_2);

				__m128i mask_0 = *(__m128i *)(&mask_0_ps);
				__m128i mask_1 = *(__m128i *)(&mask_1_ps);
				__m128i mask_2 = *(__m128i *)(&mask_2_ps);

				b32 frustum_cull = (_mm_test_all_ones(mask_0) || _mm_test_all_ones(mask_1) || _mm_test_all_ones(mask_2));
		
				if(frustum_cull) {
					// g_rasterized_tri++;

					for(u32 vert_i = 0; vert_i < 3; ++vert_i) {
						u32 index = tri_ind[vert_i];
						u8 *vertex = vertices + (index * attrib->vertex_size);
						v2 *uv0 = (v2 *)(vertex + attrib->uv0_offset);
						v3 *normal = (v3 *)(vertex + attrib->nor_offset);

						proj_x[vert_i].m128_f32[tri_count] = sx.m128_f32[vert_i];
						proj_y[vert_i].m128_f32[tri_count] = sy.m128_f32[vert_i];
						proj_z[vert_i].m128_f32[tri_count] = pz.m128_f32[vert_i];
						proj_w[vert_i].m128_f32[tri_count] = pw.m128_f32[vert_i];

						in[vert_i].texcoord_x.m128_f32[tri_count] =	uv0->x;
						in[vert_i].texcoord_y.m128_f32[tri_count] = uv0->y;

						normal_x[vert_i].m128_f32[tri_count] = normal->x;
						normal_y[vert_i].m128_f32[tri_count] = normal->y;
						normal_z[vert_i].m128_f32[tri_count] = normal->z;
					}

					tri_count++;
					if(tri_count == 4) {
						__m128 wld_00 = _mm_set1_ps(wld.m00), wld_10 = _mm_set1_ps(wld.m10), wld_20 = _mm_set1_ps(wld.m20);
						__m128 wld_01 = _mm_set1_ps(wld.m01), wld_11 = _mm_set1_ps(wld.m11), wld_21 = _mm_set1_ps(wld.m21);
						__m128 wld_02 = _mm_set1_ps(wld.m02), wld_12 = _mm_set1_ps(wld.m12), wld_22 = _mm_set1_ps(wld.m22);

						for(u32 vert_i = 0; vert_i < 3; ++vert_i) {
							__m128 screen_x = proj_x[vert_i];
							__m128 screen_y = proj_y[vert_i];
							__m128 screen_z = proj_w[vert_i];
							in[vert_i].pos_x = screen_x;
							in[vert_i].pos_y = screen_y;
							in[vert_i].pos_z = screen_z;
							
							__m128 tmp_nx0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_00), _mm_mul_ps(normal_y[vert_i], wld_10));
							__m128 tmp_nx1 = _mm_mul_ps(normal_z[vert_i], wld_20);
							in[vert_i].normal_x = _mm_add_ps(tmp_nx0, tmp_nx1);

							__m128 tmp_ny0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_01), _mm_mul_ps(normal_y[vert_i], wld_11));
							__m128 tmp_ny1 = _mm_mul_ps(normal_z[vert_i], wld_21);
							in[vert_i].normal_y = _mm_add_ps(tmp_ny0, tmp_ny1);

							__m128 tmp_nz0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_02), _mm_mul_ps(normal_y[vert_i], wld_12));
							__m128 tmp_nz1 = _mm_mul_ps(normal_z[vert_i], wld_22);
							in[vert_i].normal_z = _mm_add_ps(tmp_nz0, tmp_nz1);

										// NOTE(flo): normalize normals!
							__m128 dot_x = _mm_mul_ps(in[vert_i].normal_x, in[vert_i].normal_x);
							__m128 dot_y = _mm_mul_ps(in[vert_i].normal_y, in[vert_i].normal_y);
							__m128 dot_z = _mm_mul_ps(in[vert_i].normal_z, in[vert_i].normal_z);
							__m128 dot = _mm_add_ps(_mm_add_ps(dot_x, dot_y), dot_z);
										// TODO(flo): change this
							__m128 length = _mm_sqrt_ps(dot);
										// __m128 length = _mm_mul_ps(dot, _mm_rsqrt_ps(dot));
										// TODO(flo): check division by 0
							__m128 inverse_length = _mm_div_ps(_mm_set1_ps(1.0f), length);

							in[vert_i].normal_x = _mm_mul_ps(in[vert_i].normal_x, inverse_length);
							in[vert_i].normal_y = _mm_mul_ps(in[vert_i].normal_y, inverse_length);
							in[vert_i].normal_z = _mm_mul_ps(in[vert_i].normal_z, inverse_length);
						}
						rasterize_triangle_SSE_4x4(target, cache, diffuse_off, color, in[0], in[1], in[2], light);
						tri_count = 0;
					}
				}
			}
		}
	}
	// NOTE(flo): at worst, we redo the calculation for 3 triangles, but not the rasterization "filling" part
	if(tri_count != 0) {
		__m128 wld_00 = _mm_set1_ps(wld.m00), wld_10 = _mm_set1_ps(wld.m10), wld_20 = _mm_set1_ps(wld.m20);
		__m128 wld_01 = _mm_set1_ps(wld.m01), wld_11 = _mm_set1_ps(wld.m11), wld_21 = _mm_set1_ps(wld.m21);
		__m128 wld_02 = _mm_set1_ps(wld.m02), wld_12 = _mm_set1_ps(wld.m12), wld_22 = _mm_set1_ps(wld.m22);
		for(u32 vert_i = 0; vert_i < 3; ++vert_i) {
			__m128 screen_x = proj_x[vert_i];
			__m128 screen_y = proj_y[vert_i];
			__m128 screen_z = proj_w[vert_i];

			in[vert_i].pos_x = screen_x;
			in[vert_i].pos_y = screen_y;
			in[vert_i].pos_z = screen_z;

			__m128 tmp_nx0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_00), _mm_mul_ps(normal_y[vert_i], wld_10));
			__m128 tmp_nx1 = _mm_mul_ps(normal_z[vert_i], wld_20);
			in[vert_i].normal_x = _mm_add_ps(tmp_nx0, tmp_nx1);

			__m128 tmp_ny0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_01), _mm_mul_ps(normal_y[vert_i], wld_11));
			__m128 tmp_ny1 = _mm_mul_ps(normal_z[vert_i], wld_21);
			in[vert_i].normal_y = _mm_add_ps(tmp_ny0, tmp_ny1);

			__m128 tmp_nz0 = _mm_add_ps(_mm_mul_ps(normal_x[vert_i], wld_02), _mm_mul_ps(normal_y[vert_i], wld_12));
			__m128 tmp_nz1 = _mm_mul_ps(normal_z[vert_i], wld_22);
			in[vert_i].normal_z = _mm_add_ps(tmp_nz0, tmp_nz1);

			// NOTE(flo): normalize normals!
			__m128 dot_x = _mm_mul_ps(in[vert_i].normal_x, in[vert_i].normal_x);
			__m128 dot_y = _mm_mul_ps(in[vert_i].normal_y, in[vert_i].normal_y);
			__m128 dot_z = _mm_mul_ps(in[vert_i].normal_z, in[vert_i].normal_z);
			__m128 dot = _mm_add_ps(_mm_add_ps(dot_x, dot_y), dot_z);
			// TODO(flo): change this
			__m128 length = _mm_sqrt_ps(dot);
			// TODO(flo): check division by 0
			__m128 inverse_length = _mm_div_ps(_mm_set1_ps(1.0f), length);

			in[vert_i].normal_x = _mm_mul_ps(in[vert_i].normal_x, inverse_length);
			in[vert_i].normal_y = _mm_mul_ps(in[vert_i].normal_y, inverse_length);
			in[vert_i].normal_z = _mm_mul_ps(in[vert_i].normal_z, inverse_length);

		}
		rasterize_triangle_SSE_4x4(target, cache, diffuse_off, color, in[0], in[1], in[2], light, tri_count);
	}
}

KH_INTERN void
scanline_render(const TriangleMesh *mesh, const Texture2D *texture, mat4 mvp, SoftwareFrameBuffer *target, v4 color,
                VertexAttribute *attrib) {
	f32 half_w = (f32)target->pixels.w * 0.5f;
	f32 half_h = (f32)target->pixels.h * 0.5f;

	u32 *indices = (u32 *)(mesh->memory + mesh->indices_offset);
	u8 *vertices = (u8 *)mesh->memory;

	for(u32 tri_i = 0; tri_i < mesh->tri_c; ++tri_i) {

		u32 *tri_ind = indices + (tri_i * 3);
		u32 index_0 = tri_ind[0];
		u32 index_1 = tri_ind[1];
		u32 index_2 = tri_ind[2];

		u8 *src_0 = vertices + (index_0 * attrib->vertex_size);
		u8 *src_1 = vertices + (index_1 * attrib->vertex_size);
		u8 *src_2 = vertices + (index_2 * attrib->vertex_size);

		Vertex_PNU vert_0, vert_1, vert_2;

		vert_0.pos    = *(v3 *)(src_0 + attrib->pos_offset);
		vert_0.uv0    = *(v2 *)(src_0 + attrib->uv0_offset);
		vert_0.normal = *(v3 *)(src_0 + attrib->nor_offset);
		
		vert_1.pos    = *(v3 *)(src_1 + attrib->pos_offset);
		vert_1.uv0    = *(v2 *)(src_1 + attrib->uv0_offset);
		vert_1.normal = *(v3 *)(src_1 + attrib->nor_offset);
		
		vert_2.pos    = *(v3 *)(src_2 + attrib->pos_offset);
		vert_2.uv0    = *(v2 *)(src_2 + attrib->uv0_offset);
		vert_2.normal = *(v3 *)(src_2 + attrib->nor_offset);

		v4 pos_0 = project_position(mvp, vert_0.pos);
		v4 pos_1 = project_position(mvp, vert_1.pos);
		v4 pos_2 = project_position(mvp, vert_2.pos);

		b32 inside_view = (inside_view_frustum(pos_0.xyz, pos_0.w) || 
		                   inside_view_frustum(pos_1.xyz, pos_1.w) || 
		                   inside_view_frustum(pos_2.xyz, pos_2.w));
		if(inside_view) {
			vert_0.pos = from_mvp_to_screen_space(pos_0, half_w, half_h);
			vert_1.pos = from_mvp_to_screen_space(pos_1, half_w, half_h);
			vert_2.pos = from_mvp_to_screen_space(pos_2, half_w, half_h);

			rasterize_triangle_scanline(&target->pixels, &target->zbuffer, vert_2, vert_1, vert_0, color, texture);
		}
	}
}

JOB_ENTRY_POINT(clear_zbuffer_task) {
	DepthBufferClearWork *work = (DepthBufferClearWork *)user_data;
	clear_depth_buffer(work->buffer, work->start, work->end);
}

JOB_ENTRY_POINT(clear_pixels_buffer_task) {
	PixelsBufferClearWork *work = (PixelsBufferClearWork *)user_data;
	clear_pixels_buffer(work->target, work->color, work->min_y, work->max_y);
}

JOB_ENTRY_POINT(tri_mesh_render_sched) {
	TriMeshRenderWork *task = (TriMeshRenderWork *)user_data;
	SoftwareFrameBuffer *target = task->target;
	tri_mesh_render(task->target, task->min_x, task->max_x, task->min_y, task->max_y, task->cache, task->mesh_off, task->diff_off, &task->attrib, task->light, task->mvp, task->wld, task->color);
}

// TODO(flo): skybox support
KH_INTERN void
software_render(SoftwareFrameBuffer *dst, RenderManager *render, WorkQueue *queue, TriMeshRenderWork *works, 
                const u32 work_count) {

	// u32 total_tri = 0;
	// g_asked_tri = 0;
	// g_clip_tri = 0;
	// g_cull_tri = 0;
	// g_rasterized_tri = 0;

	char DEBUG_txt_buff[256];
	// u64 frame_buffer_clear = __rdtsc();

	const u32 clear_rect_count = 4;
	DepthBufferClearWork zbuffer_clears[clear_rect_count];
	PixelsBufferClearWork pbuffer_clears[clear_rect_count];
	u32 pwork_count = 0;
	u32 zwork_count = 0;

	u32 dst_w = dst->pixels.w;
	u32 dst_h = dst->pixels.h;
	u32 size = dst_w * dst_h;
	u32 size_y = dst_h / clear_rect_count;
	u32 min_y = 0;
	u32 max_y = size_y;
	u32 zcount = size / clear_rect_count;
	u32 zstart = 0;
	u32 zend = zcount;
	for(u32 c_i = 0; c_i < clear_rect_count; ++c_i)	{
		PixelsBufferClearWork *pbuffer_work = pbuffer_clears + pwork_count++;
		pbuffer_work->target = &dst->pixels;
		pbuffer_work->color = kh_vec4(0,0,0,1);
		pbuffer_work->min_y = min_y;
		pbuffer_work->max_y = max_y;
		g_platform.add_work_to_queue(queue, clear_pixels_buffer_task, pbuffer_work);
		min_y = max_y;
		max_y += size_y;
		DepthBufferClearWork *zbuffer_work = zbuffer_clears + zwork_count++;
		zbuffer_work->buffer = &dst->zbuffer;
		zbuffer_work->start = zstart;
		zbuffer_work->end = zend;
		g_platform.add_work_to_queue(queue, clear_zbuffer_task, zbuffer_work);
		zstart = zend;
		zend += zcount;
	}

	g_platform.complete_all_queue_works(queue);
	// u64 frame_buffer_clear_elapsed = __rdtsc() - frame_buffer_clear;
	// stbsp_sprintf(DEBUG_txt_buff, "framebuffer clear : %llu \n", frame_buffer_clear_elapsed);
	// OutputDebugStringA(DEBUG_txt_buff);

	DataCache *cache = render->cache;

	mat4 view = render->camera.view;
	mat4 proj = render->camera.projection;
	mat4 vp = view * proj;

	kh_assert(render->light_count > 0);
	DirectionalLight *light = render->lights + 0;

	// TODO(flo) : really implement this
	u32 first_free_work = 0;

	for(VertexBuffer *buffer = render->first_vertex_buffer; buffer; buffer = buffer->next) {
		VertexFormat format = buffer->format;
		VertexAttribute attrib = get_vertex_attribute(format);
		for(Material *mat = buffer->first; mat; mat = mat->next) {
			for(MaterialInstance *instance = mat->first; instance; instance = instance->next) {

				v4 color = instance->color;
				color.rgb *= color.a;
				for(MeshRenderer *meshr = instance->first; meshr; meshr = meshr->next) {

					// DataHashElement *mesh_el = cache->hash + meshr->mesh;
					// TriangleMesh *mesh = get_datas(cache, mesh_el, TriangleMesh);
					// kh_assert(mesh->format == buffer->format);

					u32 entry_ind = meshr->first_entry;
					for(u32 i = 0; i < meshr->entry_count; ++i) {

						// total_tri += mesh->tri_c;

						RenderEntry *entry = render->render_entries + entry_ind;

						mat4 wld = entry->tr;
						mat4 mvp = wld * vp;

						entry_ind = entry->next_in_mesh_renderer;

						for(u32 work_i = 0; work_i < 4; ++work_i) {
							kh_assert(first_free_work <= work_count);
							TriMeshRenderWork *work = works + first_free_work++;
							work->light = light;
							work->mesh_off = meshr->mesh;
							work->diff_off = instance->diffuse;
							work->attrib = attrib;
							work->mvp = mvp;
							work->wld = wld;
							work->color = color;
							// TODO(flo): PixelsBuffer and DepthBuffer are not thread safe
							g_platform.add_work_to_queue(queue, tri_mesh_render_sched, work);
						}
					}
				}
			}
		}
	}

	g_platform.complete_all_queue_works(queue);

	// if(g_asked_tri > 0) {
	// 	f32 r = (1.0f / (f32)g_asked_tri) * 100.0f;
	// 	g_clip_tri = g_asked_tri - g_clip_tri; 
	// 	g_cull_tri = g_asked_tri - g_clip_tri - g_cull_tri;
	// 	u32 frustum_tri = g_asked_tri - g_clip_tri - g_cull_tri - g_rasterized_tri;
	// 	stbsp_sprintf(DEBUG_txt_buff, "Triangles : \n - Total : %d\n - Asked : %d, %f%%\n - Clipped : %d, %f%%\n - BFCulled : %d, %f%%\n - FCulled : %d, %f%%\n - Rasterized : %d, %f%%\n", total_tri,
	// 	              g_asked_tri, (f32)g_asked_tri * r, g_clip_tri, (f32)g_clip_tri * r, g_cull_tri, (f32)g_cull_tri * r,
	// 	              frustum_tri, frustum_tri * r, g_rasterized_tri, (f32)g_rasterized_tri * r);
	// 	OutputDebugStringA(DEBUG_txt_buff);


	// }
}
