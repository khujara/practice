#define MAX_TRI_MESH_RENDER_TASK 2568
// static u32 g_asked_tri;
// static u32 g_clip_tri;
// static u32 g_cull_tri;
// static u32 g_rasterized_tri;

inline void
debug_draw_texture(SoftwarePixelsBuffer *target, Texture2D *tex, u8 *data, v3 pos) {
	u32 min_x = (u32)pos.x;
	u32 min_y = (u32)pos.y;
	u32 max_x = min_x + tex->width;
	u32 max_y = min_y + tex->height;

	u8 *dst_row = (u8 *)target->memory + min_x * FRAME_BUFFER_BYTES_PER_PIXEL + min_y * target->pitch;;
	u8 *src_row = (u8 *)data;
	for(u32 y = min_y; y < max_y; ++y) {
		u32 *dst_pixel = (u32 *)dst_row;;
		for(u32 x = min_x; x < max_x; ++x) {
			u8 b = *src_row++;
			*dst_pixel++ = ((u32)(b << 24) | (u32)(b << 16) | (u32)(b << 8) | (u32)(b << 0));
		}

		dst_row += target->pitch;
	}
}

KH_INTERN void
scanline_render(const TriangleMesh *mesh, const u8 *mesh_memory, const Texture2D *texture, const u8 *texture_memory, 
                mat4 mvp, SoftwareFrameBuffer *target, v4 color, VertexAttribute *attrib) {
	f32 half_w = (f32)target->pixels.w * 0.5f;
	f32 half_h = (f32)target->pixels.h * 0.5f;

	u32 *indices = (u32 *)(mesh_memory + mesh->indices_offset);
	u8 *vertices = (u8 *)mesh_memory;

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

		b32 inside_view = (inside_view_frustum(pos_0) || 
		                   inside_view_frustum(pos_1) || 
		                   inside_view_frustum(pos_2));
		if(inside_view) {
			vert_0.pos = from_mvp_to_screen_space(pos_0, half_w, half_h);
			vert_1.pos = from_mvp_to_screen_space(pos_1, half_w, half_h);
			vert_2.pos = from_mvp_to_screen_space(pos_2, half_w, half_h);

			rasterize_triangle_scanline(&target->pixels, &target->zbuffer, vert_2, vert_1, vert_0, color, texture, texture_memory);
		}
	}
}

JOB_ENTRY_POINT(clear_zbuffer_task) {
	DepthBufferClearWork *work = (DepthBufferClearWork *)user_data;
	clear_depth_buffer(work->buffer, work->start, work->end);
}

JOB_ENTRY_POINT(clear_pixels_buffer_task) {
	PixelsBufferClearWork *work = (PixelsBufferClearWork *)user_data;
	clear_pixels_buffer(work->target, work->color, work->start, work->end);
}


KH_INLINE void
tri_mesh_render(TriMeshRenderWork *task) {
	Assets *assets = task->assets;
	SoftwareFrameBuffer *target = task->target;
	mat4 *bone_transformations = task->bone_transformations;

	LoadedAsset loaded_mesh = get_loaded_asset(assets, task->mesh_id, AssetType_trimesh);
	const TriangleMesh mesh = loaded_mesh.type->trimesh;
	u8 *mesh_memory = loaded_mesh.data;

	__m128 clip_min_x = _mm_set1_ps(task->min_x);
	__m128 clip_max_x = _mm_set1_ps(task->max_x);
	__m128 clip_min_y = _mm_set1_ps(task->min_y);
	__m128 clip_max_y = _mm_set1_ps(task->max_y);

	__m128 half_w_SSE = _mm_set1_ps((f32)target->pixels.w * 0.5f);
	__m128 half_h_SSE = _mm_set1_ps((f32)target->pixels.h * 0.5f);

	__m128 mvp_c0 = _mm_setr_ps(task->mvp.c0.x, task->mvp.c0.y, task->mvp.c0.z, task->mvp.c0.w);
	__m128 mvp_c1 = _mm_setr_ps(task->mvp.c1.x, task->mvp.c1.y, task->mvp.c1.z, task->mvp.c1.w);
	__m128 mvp_c2 = _mm_setr_ps(task->mvp.c2.x, task->mvp.c2.y, task->mvp.c2.z, task->mvp.c2.w);
	__m128 mvp_c3 = _mm_setr_ps(task->mvp.c3.x, task->mvp.c3.y, task->mvp.c3.z, task->mvp.c3.w);

	__m128 normal_x[3];
	__m128 normal_y[3];
	__m128 normal_z[3];
	__m128 proj_x[3];
	__m128 proj_y[3];
	__m128 proj_z[3];
	__m128 proj_w[3];

	Vertex_SSE in[3];

	u8 *vertices = mesh_memory;
	u32 *indices = (u32 *)(mesh_memory + mesh.indices_offset);


	v3 pos_0, pos_1, pos_2;
	v3 normal_0, normal_1, normal_2;

	VertexSkinnedAttribute skin_attrib = {};
	if(bone_transformations) {
		skin_attrib = get_skinned_attribute_offset(mesh.format);
	}
	Vertex_PNUTBS tamere;

	u32 tri_count = 0;
	for(u32 tri_i = 0; tri_i < mesh.tri_c; ++tri_i) {
		// g_asked_tri++;
		u32 *tri_ind = indices + tri_i * 3;
		u8* vertex_0 = vertices + (tri_ind[0] * task->attrib.vertex_size);
		u8 *vertex_1 = vertices + (tri_ind[1] * task->attrib.vertex_size);
		u8 *vertex_2 = vertices + (tri_ind[2] * task->attrib.vertex_size);
		pos_0 = *(v3 *)(vertex_0 + task->attrib.pos_offset);
		pos_1 = *(v3 *)(vertex_1 + task->attrib.pos_offset);
		pos_2 = *(v3 *)(vertex_2 + task->attrib.pos_offset);
		normal_0 = *(v3 *)(vertex_0 + task->attrib.nor_offset);
		normal_1 = *(v3 *)(vertex_1 + task->attrib.nor_offset);
		normal_2 = *(v3 *)(vertex_2 + task->attrib.nor_offset);

		v2 *uv0_0 = (v2 *)(vertex_0 + task->attrib.uv0_offset);
		v2 *uv0_1 = (v2 *)(vertex_1 + task->attrib.uv0_offset);
		v2 *uv0_2 = (v2 *)(vertex_2 + task->attrib.uv0_offset);

		if(bone_transformations) {

			u32 *ids_0 = (u32 *)(vertex_0 + skin_attrib.offset);
			f32 *weight_0 = (f32 *)(ids_0 + MAX_JOINTS_PER_VERTEX);

			u32 *ids_1 = (u32 *)(vertex_1 + skin_attrib.offset);
			f32 *weight_1 = (f32 *)(ids_1 + MAX_JOINTS_PER_VERTEX);

			u32 *ids_2 = (u32 *)(vertex_2 + skin_attrib.offset);
			f32 *weight_2 = (f32 *)(ids_2 + MAX_JOINTS_PER_VERTEX);


			kh_assert(has_skin(mesh.format));

			__m128 c0_0 = _mm_set1_ps(0.0f);
			__m128 c1_0 = _mm_set1_ps(0.0f);
			__m128 c2_0 = _mm_set1_ps(0.0f);
			__m128 c3_0 = _mm_set1_ps(0.0f);
			__m128 c0_1 = _mm_set1_ps(0.0f);
			__m128 c1_1 = _mm_set1_ps(0.0f);
			__m128 c2_1 = _mm_set1_ps(0.0f);
			__m128 c3_1 = _mm_set1_ps(0.0f);
			__m128 c0_2 = _mm_set1_ps(0.0f);
			__m128 c1_2 = _mm_set1_ps(0.0f);
			__m128 c2_2 = _mm_set1_ps(0.0f);
			__m128 c3_2 = _mm_set1_ps(0.0f);

			for(u32 j = 0; j < MAX_JOINTS_PER_VERTEX; ++j) {
				if(weight_0[j] > 0.0f) {
					mat4 *final_tr = bone_transformations + ids_0[j];
					__m128 w = _mm_set1_ps(weight_0[j]);

					__m128 tmpc0 = _mm_setr_ps(final_tr->c0.x, final_tr->c0.y, final_tr->c0.z, final_tr->c0.w);
					__m128 tmpc1 = _mm_setr_ps(final_tr->c1.x, final_tr->c1.y, final_tr->c1.z, final_tr->c1.w);
					__m128 tmpc2 = _mm_setr_ps(final_tr->c2.x, final_tr->c2.y, final_tr->c2.z, final_tr->c2.w);
					__m128 tmpc3 = _mm_setr_ps(final_tr->c3.x, final_tr->c3.y, final_tr->c3.z, final_tr->c3.w);

					c0_0 = _mm_add_ps(c0_0, _mm_mul_ps(tmpc0, w));
					c1_0 = _mm_add_ps(c1_0, _mm_mul_ps(tmpc1, w));
					c2_0 = _mm_add_ps(c2_0, _mm_mul_ps(tmpc2, w));
					c3_0 = _mm_add_ps(c3_0, _mm_mul_ps(tmpc3, w));

				}
				if(weight_1[j] > 0.0f) {
					mat4 *final_tr = bone_transformations + ids_1[j];
					__m128 w = _mm_set1_ps(weight_1[j]);

					__m128 tmpc0 = _mm_setr_ps(final_tr->c0.x, final_tr->c0.y, final_tr->c0.z, final_tr->c0.w);
					__m128 tmpc1 = _mm_setr_ps(final_tr->c1.x, final_tr->c1.y, final_tr->c1.z, final_tr->c1.w);
					__m128 tmpc2 = _mm_setr_ps(final_tr->c2.x, final_tr->c2.y, final_tr->c2.z, final_tr->c2.w);
					__m128 tmpc3 = _mm_setr_ps(final_tr->c3.x, final_tr->c3.y, final_tr->c3.z, final_tr->c3.w);

					c0_1 = _mm_add_ps(c0_1, _mm_mul_ps(tmpc0, w));
					c1_1 = _mm_add_ps(c1_1, _mm_mul_ps(tmpc1, w));
					c2_1 = _mm_add_ps(c2_1, _mm_mul_ps(tmpc2, w));
					c3_1 = _mm_add_ps(c3_1, _mm_mul_ps(tmpc3, w));

				}
				if(weight_2[j] > 0.0f) {
					mat4 *final_tr = bone_transformations + ids_2[j];
					__m128 w = _mm_set1_ps(weight_2[j]);

					__m128 tmpc0 = _mm_setr_ps(final_tr->c0.x, final_tr->c0.y, final_tr->c0.z, final_tr->c0.w);
					__m128 tmpc1 = _mm_setr_ps(final_tr->c1.x, final_tr->c1.y, final_tr->c1.z, final_tr->c1.w);
					__m128 tmpc2 = _mm_setr_ps(final_tr->c2.x, final_tr->c2.y, final_tr->c2.z, final_tr->c2.w);
					__m128 tmpc3 = _mm_setr_ps(final_tr->c3.x, final_tr->c3.y, final_tr->c3.z, final_tr->c3.w);

					c0_2 = _mm_add_ps(c0_2, _mm_mul_ps(tmpc0, w));
					c1_2 = _mm_add_ps(c1_2, _mm_mul_ps(tmpc1, w));
					c2_2 = _mm_add_ps(c2_2, _mm_mul_ps(tmpc2, w));
					c3_2 = _mm_add_ps(c3_2, _mm_mul_ps(tmpc3, w));
				}

			}

			__m128 p0x = _mm_set1_ps(pos_0.x);
			__m128 p0y = _mm_set1_ps(pos_0.y);
			__m128 p0z = _mm_set1_ps(pos_0.z);
			__m128 p0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p0x, c0_0), _mm_mul_ps(p0y, c1_0)), 
			                       _mm_add_ps(_mm_mul_ps(p0z, c2_0), c3_0));
			__m128 p1x = _mm_set1_ps(pos_1.x);
			__m128 p1y = _mm_set1_ps(pos_1.y);
			__m128 p1z = _mm_set1_ps(pos_1.z);
			__m128 p1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p1x, c0_1), _mm_mul_ps(p1y, c1_1)), 
			                       _mm_add_ps(_mm_mul_ps(p1z, c2_1), c3_1));
			__m128 p2x = _mm_set1_ps(pos_2.x);
			__m128 p2y = _mm_set1_ps(pos_2.y);
			__m128 p2z = _mm_set1_ps(pos_2.z);
			__m128 p2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p2x, c0_2), _mm_mul_ps(p2y, c1_2)), 
			                       _mm_add_ps(_mm_mul_ps(p2z, c2_2), c3_2));

			__m128 n0x = _mm_set1_ps(normal_0.x);
			__m128 n0y = _mm_set1_ps(normal_0.y);
			__m128 n0z = _mm_set1_ps(normal_0.z);
			__m128 n0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(n0x, c0_0), _mm_mul_ps(n0y, c1_0)), 
			                       _mm_mul_ps(n0z, c2_0));
			__m128 n1x = _mm_set1_ps(normal_1.x);
			__m128 n1y = _mm_set1_ps(normal_1.y);
			__m128 n1z = _mm_set1_ps(normal_1.z);
			__m128 n1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(n1x, c0_1), _mm_mul_ps(n1y, c1_1)), 
			                       _mm_mul_ps(n1z, c2_1));
			__m128 n2x = _mm_set1_ps(normal_2.x);
			__m128 n2y = _mm_set1_ps(normal_2.y);
			__m128 n2z = _mm_set1_ps(normal_2.z);
			__m128 n2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(n2x, c0_2), _mm_mul_ps(n2y, c1_2)), 
			                       _mm_mul_ps(n2z, c2_2));

			pos_0 = *(v3 *)(&p0);
			pos_1 = *(v3 *)(&p1);
			pos_2 = *(v3 *)(&p2);

			normal_0 = *(v3 *)(&n0);
			normal_1 = *(v3 *)(&n1);
			normal_2 = *(v3 *)(&n2);
		}

		normal_x[0].m128_f32[tri_count] = normal_0.x;
		normal_y[0].m128_f32[tri_count] = normal_0.y;
		normal_z[0].m128_f32[tri_count] = normal_0.z;
		normal_x[1].m128_f32[tri_count] = normal_1.x;
		normal_y[1].m128_f32[tri_count] = normal_1.y;
		normal_z[1].m128_f32[tri_count] = normal_1.z;
		normal_x[2].m128_f32[tri_count] = normal_2.x;
		normal_y[2].m128_f32[tri_count] = normal_2.y;
		normal_z[2].m128_f32[tri_count] = normal_2.z;

		__m128 p0x = _mm_set1_ps(pos_0.x);
		__m128 p0y = _mm_set1_ps(pos_0.y);
		__m128 p0z = _mm_set1_ps(pos_0.z);
		__m128 p1x = _mm_set1_ps(pos_1.x);
		__m128 p1y = _mm_set1_ps(pos_1.y);
		__m128 p1z = _mm_set1_ps(pos_1.z);
		__m128 p2x = _mm_set1_ps(pos_2.x);
		__m128 p2y = _mm_set1_ps(pos_2.y);
		__m128 p2z = _mm_set1_ps(pos_2.z);

		__m128 proj_0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p0x, mvp_c0), _mm_mul_ps(p0y, mvp_c1)),
		                           _mm_add_ps(_mm_mul_ps(p0z, mvp_c2), mvp_c3));
		__m128 proj_1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p1x, mvp_c0), _mm_mul_ps(p1y, mvp_c1)), 
		                           _mm_add_ps(_mm_mul_ps(p1z, mvp_c2), mvp_c3));
		__m128 proj_2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(p2x, mvp_c0), _mm_mul_ps(p2y, mvp_c1)), 
		                           _mm_add_ps(_mm_mul_ps(p2z, mvp_c2), mvp_c3));

		v4 *proj_0_ptr = (v4 *)&proj_0;
		v4 *proj_1_ptr = (v4 *)&proj_1;
		v4 *proj_2_ptr = (v4 *)&proj_2;

		// TODO(flo): do we need to duplicate here ?
		__m128 px = _mm_set_ps(proj_0_ptr->x, proj_2_ptr->x, proj_1_ptr->x, proj_0_ptr->x);
		__m128 py = _mm_set_ps(proj_0_ptr->y, proj_2_ptr->y, proj_1_ptr->y, proj_0_ptr->y);
		__m128 pz = _mm_set_ps(proj_0_ptr->z, proj_2_ptr->z, proj_1_ptr->z, proj_0_ptr->z);
		__m128 pw = _mm_set_ps(proj_0_ptr->w, proj_2_ptr->w, proj_1_ptr->w, proj_0_ptr->w);

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
					in[0].texcoord_x.m128_f32[tri_count] = uv0_0->x;
					in[0].texcoord_y.m128_f32[tri_count] = uv0_0->y;
					in[1].texcoord_x.m128_f32[tri_count] = uv0_1->x;
					in[1].texcoord_y.m128_f32[tri_count] = uv0_1->y;
					in[2].texcoord_x.m128_f32[tri_count] = uv0_2->x;
					in[2].texcoord_y.m128_f32[tri_count] = uv0_2->y;

					for(u32 vert_i = 0; vert_i < 3; ++vert_i) {

						proj_x[vert_i].m128_f32[tri_count] = sx.m128_f32[vert_i];
						proj_y[vert_i].m128_f32[tri_count] = sy.m128_f32[vert_i];
						proj_z[vert_i].m128_f32[tri_count] = pz.m128_f32[vert_i];
						proj_w[vert_i].m128_f32[tri_count] = pw.m128_f32[vert_i];

					}

					tri_count++;
					if(tri_count == 4) {
						__m128 wld_00 = _mm_set1_ps(task->wld.m00), wld_10 = _mm_set1_ps(task->wld.m10), wld_20 = _mm_set1_ps(task->wld.m20);
						__m128 wld_01 = _mm_set1_ps(task->wld.m01), wld_11 = _mm_set1_ps(task->wld.m11), wld_21 = _mm_set1_ps(task->wld.m21);
						__m128 wld_02 = _mm_set1_ps(task->wld.m02), wld_12 = _mm_set1_ps(task->wld.m12), wld_22 = _mm_set1_ps(task->wld.m22);

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
							// __m128 length = _mm_sqrt_ps(dot);
							// __m128 length = _mm_mul_ps(dot, _mm_rsqrt_ps(dot));
							// __m128 inverse_length = _mm_div_ps(_mm_set1_ps(1.0f), length);
							__m128 inverse_length = _mm_rsqrt_ps(dot);

							in[vert_i].normal_x = _mm_mul_ps(in[vert_i].normal_x, inverse_length);
							in[vert_i].normal_y = _mm_mul_ps(in[vert_i].normal_y, inverse_length);
							in[vert_i].normal_z = _mm_mul_ps(in[vert_i].normal_z, inverse_length);
						}
						rasterize_triangle_SSE_4x4(target, assets, task->diff_id, task->color, in[0], in[1], in[2], task->light);
						tri_count = 0;
					}
				}
			}
		}
	}
	// NOTE(flo): at worst, we redo the calculation for 3 triangles, but not the rasterization "filling" part
	if(tri_count != 0) {
		__m128 wld_00 = _mm_set1_ps(task->wld.m00), wld_10 = _mm_set1_ps(task->wld.m10), wld_20 = _mm_set1_ps(task->wld.m20);
		__m128 wld_01 = _mm_set1_ps(task->wld.m01), wld_11 = _mm_set1_ps(task->wld.m11), wld_21 = _mm_set1_ps(task->wld.m21);
		__m128 wld_02 = _mm_set1_ps(task->wld.m02), wld_12 = _mm_set1_ps(task->wld.m12), wld_22 = _mm_set1_ps(task->wld.m22);
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
		rasterize_triangle_SSE_4x4(target, assets, task->diff_id, task->color, in[0], in[1], in[2], task->light, tri_count);
	}
}

// TODO(flo): we need to profile this
JOB_ENTRY_POINT(tri_mesh_render_sched) {
	TriMeshRenderWork *task = (TriMeshRenderWork *)user_data;
	tri_mesh_render(task);
}

// TODO(flo): skybox support
KH_INTERN void
software_render(SoftwareFrameBuffer *dst, Assets *assets, RenderManager *render, WorkQueue *queue, 
                TriMeshRenderWork *works, const u32 work_count) {

	// u32 total_tri = 0;
	// g_asked_tri = 0;
	// g_clip_tri = 0;
	// g_cull_tri = 0;
	// g_rasterized_tri = 0;
	DataCache *cache = &assets->cache;

	// char DEBUG_txt_buff[256];
	// u64 frame_buffer_clear = __rdtsc();


	v4 clear_color = kh_vec4(0.1f,0.1f,0.1f,1);
	clear_color *= 255.0f;
	u32 final_color = ((kh_round_f32_to_u32(clear_color.a) << 24) |
		(kh_round_f32_to_u32(clear_color.r) << 16) |
		(kh_round_f32_to_u32(clear_color.g) << 8) |
		(kh_round_f32_to_u32(clear_color.b) << 0));

	const u32 clear_rect_count = 4;
	DepthBufferClearWork zbuffer_clears[clear_rect_count];
	PixelsBufferClearWork pbuffer_clears[clear_rect_count];
	u32 pwork_count = 0;
	u32 zwork_count = 0;

	u32 dst_w = dst->pixels.w;
	u32 dst_h = dst->pixels.h;

	u32 psize = (dst_h / clear_rect_count) * dst->pixels.w;
	u32 pstart = 0;
	u32 pend = psize;

	u32 zsize = dst_w * dst_h;
	u32 zcount = zsize / clear_rect_count;
	u32 zstart = 0;
	u32 zend = zcount;

	for(u32 c_i = 0; c_i < clear_rect_count; ++c_i)	{
		PixelsBufferClearWork *pbuffer_work = pbuffer_clears + pwork_count++;
		pbuffer_work->target = &dst->pixels;
		pbuffer_work->color = final_color;
		pbuffer_work->start = pstart;
		pbuffer_work->end = pend;
		g_platform.add_work_to_queue(queue, clear_pixels_buffer_task, pbuffer_work);
		pstart = pend;
		pend += psize;
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
				color.r *= color.a;
				color.g *= color.a;
				color.b *= color.a;
				for(MeshRenderer *meshr = instance->first; meshr; meshr = meshr->next) {

					u32 entry_ind = meshr->first_entry;
					const TriangleMesh mesh = get_trimesh(assets, meshr->mesh);
					kh_assert(mesh.format == format);

					for(u32 i = 0; i < meshr->entry_count; ++i) {

						RenderEntry *entry = render->render_entries + entry_ind;

						mat4 wld = entry->tr;
						mat4 mvp = wld * vp;
						mat4 *bone_transformations = 0;
						if(entry->bone_transform_offset < render->bone_tr.count) {
							bone_transformations = render->bone_tr.data + entry->bone_transform_offset;
						}

#if 1
						for(u32 work_i = 0; work_i < 4; ++work_i) {
							kh_assert(first_free_work <= work_count);
							TriMeshRenderWork *work = works + first_free_work++;
							work->bone_transformations = bone_transformations;
							work->light = light;
							work->mesh_id = meshr->mesh;
							work->diff_id = instance->diffuse;
							work->attrib = attrib;
							work->mvp = mvp;
							work->wld = wld;
							work->color = color;
							// TODO(flo): PixelsBuffer and DepthBuffer are not thread safe
							g_platform.add_work_to_queue(queue, tri_mesh_render_sched, work);
						}
#else

						TriMeshRenderWork work;
						work.target = dst;
						work.min_x = 0;
						work.max_x = (f32)render->width;
						work.min_y = 0;
						work.max_y = (f32)render->height;
						work.assets = assets;
						work.bone_transformations = bone_transformations;
						work.light = light;
						work.mesh_id = meshr->mesh;
						work.diff_id = instance->diffuse;
						work.attrib = attrib;
						work.mvp = mvp;
						work.wld = wld;
						work.color = color;
						tri_mesh_render(&work);
#endif
						entry_ind = entry->next_in_mesh_renderer;
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
