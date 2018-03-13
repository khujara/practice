// TODO(flo): support multiple vertex format and 
// we need to have different name specified for each vertex format

KH_INTERN void
set_cube_vertices_indices(Vertex_PNU *vertices, u32 *indices, f32 s) {
		// NOTE(flo): front
		vertices[0].pos    = kh_vec3(-s, -s, -s);
		vertices[0].uv0    = kh_vec2(0,0);
		vertices[0].normal = kh_vec3(0, 0, -1);
		vertices[1].pos    = kh_vec3(s, -s, -s);
		vertices[1].uv0    = kh_vec2(1,0);
		vertices[1].normal = kh_vec3(0, 0, -1);
		vertices[2].pos    = kh_vec3(s, s, -s);
		vertices[2].uv0    = kh_vec2(1,1);
		vertices[2].normal = kh_vec3(0, 0, -1);
		vertices[3].pos    = kh_vec3(-s, s, -s);
		vertices[3].uv0    = kh_vec2(0,1);
		vertices[3].normal = kh_vec3(0, 0, -1);

		// NOTE(flo): back
		vertices[4].pos    = kh_vec3(-s, s, s);
		vertices[4].uv0    = kh_vec2(0,0);
		vertices[4].normal = kh_vec3(0, 0, 1);
		vertices[5].pos    = kh_vec3(s, s, s);
		vertices[5].uv0    = kh_vec2(1,0);
		vertices[5].normal = kh_vec3(0, 0, 1);
		vertices[6].pos    = kh_vec3(s, -s, s);
		vertices[6].uv0    = kh_vec2(1,1);
		vertices[6].normal = kh_vec3(0, 0, 1);
		vertices[7].pos    = kh_vec3(-s, -s, s);
		vertices[7].uv0    = kh_vec2(0,1);
		vertices[7].normal = kh_vec3(0, 0, 1);


		// NOTE(flo): left
		vertices[8].pos    = kh_vec3(-s, -s, s);
		vertices[8].uv0    = kh_vec2(0,0);
		vertices[8].normal = kh_vec3(-1, 0, 0);
		vertices[9].pos    = kh_vec3(-s, -s, -s);
		vertices[9].uv0    = kh_vec2(1,0);
		vertices[9].normal = kh_vec3(-1, 0, 0);
		vertices[10].pos    = kh_vec3(-s, s, -s);
		vertices[10].uv0    = kh_vec2(1,1);
		vertices[10].normal = kh_vec3(-1, 0, 0);
		vertices[11].pos    = kh_vec3(-s, s, s);
		vertices[11].uv0    = kh_vec2(0,1);
		vertices[11].normal = kh_vec3(-1, 0, 0);

		// NOTE(flo): right
		vertices[12].pos    = kh_vec3(s, -s, -s);
		vertices[12].uv0    = kh_vec2(0,0);
		vertices[12].normal = kh_vec3(1, 0, 0);
		vertices[13].pos    = kh_vec3(s, -s, s);
		vertices[13].uv0    = kh_vec2(1,0);
		vertices[13].normal = kh_vec3(1, 0, 0);
		vertices[14].pos    = kh_vec3(s, s, s);
		vertices[14].uv0    = kh_vec2(1,1);
		vertices[14].normal = kh_vec3(1, 0, 0);
		vertices[15].pos    = kh_vec3(s, s, -s);
		vertices[15].uv0    = kh_vec2(0,1);
		vertices[15].normal = kh_vec3(1, 0, 0);

		// NOTE(flo): bottom
		vertices[16].pos    = kh_vec3(-s, -s, s);
		vertices[16].uv0    = kh_vec2(0,0);
		vertices[16].normal = kh_vec3(0, -1, 0);
		vertices[17].pos    = kh_vec3(s, -s, s);
		vertices[17].uv0    = kh_vec2(1,0);
		vertices[17].normal = kh_vec3(0, -1, 0);
		vertices[18].pos    = kh_vec3(s, -s, -s);
		vertices[18].uv0    = kh_vec2(1,1);
		vertices[18].normal = kh_vec3(0, -1, 0);
		vertices[19].pos    = kh_vec3(-s, -s, -s);
		vertices[19].uv0    = kh_vec2(0,1);
		vertices[19].normal = kh_vec3(0, -1, 0);

		// NOTE(flo): top
		vertices[20].pos    = kh_vec3(-s, s, -s);
		vertices[20].uv0    = kh_vec2(0,0);
		vertices[20].normal = kh_vec3(0, 1, 0);
		vertices[21].pos    = kh_vec3(s, s, -s);
		vertices[21].uv0    = kh_vec2(1,0);
		vertices[21].normal = kh_vec3(0, 1, 0);
		vertices[22].pos    = kh_vec3(s, s, s);
		vertices[22].uv0    = kh_vec2(1,1);
		vertices[22].normal = kh_vec3(0, 1, 0);
		vertices[23].pos    = kh_vec3(-s, s, s);
		vertices[23].uv0    = kh_vec2(0,1);
		vertices[23].normal = kh_vec3(0, 1, 0);

		u32 faces_count = 6;
		u32 *index = indices;
		u32 base_index = 0;
		for(u32 i = 0; i < faces_count; ++i) {
			index[0] = base_index + 0;
			index[1] = base_index + 1;
			index[2] = base_index + 2;
			index[3] = base_index + 0;
			index[4] = base_index + 2;
			index[5] = base_index + 3;
			index += 6;
			base_index += 4;
		}

}

KH_INTERN u32
get_or_create_tri_mesh_cube(Assets *assets) {
	u32 id = get_or_create_asset_id_from_name(assets, assets->primitive_names[Primitive_cube]);
	Asset *asset = assets->arr +id;
	if(asset->state != AssetState_loaded) {
		asset->source = {};
		asset->source.type.key = AssetType_trimesh;
		asset->state = AssetState_loaded;

		VertexFormat format = VertexFormat_PNU;
		u32 vertex_size = get_size_from_vertex_format(format);

		u32 tri_count = 24;
		u32 vertices_count = 24;

		u32 vertices_size = vertices_count * vertex_size; 
		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 size = vertices_size + indices_size;

		u8 *dst = add_asset_to_data_cache(assets, asset, size);
		Vertex_PNU *vertices = (Vertex_PNU *)dst;
		u32 *indices = (u32 *)(dst + vertices_size);

		asset->source.type.trimesh.vert_c = vertices_count;
		asset->source.type.trimesh.tri_c = tri_count;
		asset->source.type.trimesh.vertex_size = vertex_size;
		asset->source.type.trimesh.format = format;
		asset->source.type.trimesh.indices_offset = vertices_size;

		f32 s = 1;
		set_cube_vertices_indices(vertices, indices, s);
	}
	return(id);
}

KH_INTERN void
set_vertex(v3 *pos, v3 *normal, v2 *uv, i32 x, i32 y, i32 z, i32 grid_size) {
	v3 v = kh_vec3(x, y, z) * 2.0f / (f32)grid_size - kh_vec3(1,1,1);
	v3 vn;
	// vn = kh_normalize_v3(v);

	// NOTE(flo): uniform mapping
	f32 x_sqr = v.x * v.x;
	f32 y_sqr = v.y * v.y;
	f32 z_sqr = v.z * v.z;
	vn.x = v.x * kh_sqrt_f32(1.0f - y_sqr / 2.0f - z_sqr / 2.0f + y_sqr * z_sqr / 3.0f);
	vn.y = v.y * kh_sqrt_f32(1.0f - x_sqr / 2.0f - z_sqr / 2.0f + x_sqr * z_sqr / 3.0f);
	vn.z = v.z * kh_sqrt_f32(1.0f - x_sqr / 2.0f - y_sqr / 2.0f + x_sqr * y_sqr / 3.0f);

	*normal = vn;
	*pos = vn;
	uv->x = 0.5f + kh_atan2_f32(vn.z, vn.x) / (2.0f*PI32);
	uv->y = 0.5f + 0.5f * vn.y;
	
	// NOTE(flo): unity (correct) texture coordinates style
	// vert->uv0.y = 0.5f - kh_asin_f32(vn.y)/PI32;
}

KH_INTERN u32
set_quad_indices(u32 *triangles, u32 index, u32 v_00, u32 v_10, u32 v_01, u32 v_11) {
	triangles[index + 0] = v_00;
	triangles[index + 1] = v_10;
	triangles[index + 2] = v_01;

	triangles[index + 3] = v_01;
	triangles[index + 4] = v_10;
	triangles[index + 5] = v_11;

	u32 res = index + 6;
	return(res);
}

KH_INTERN void
set_vertex_pnu(Vertex_PNU *vert, i32 x, i32 y, i32 z, i32 grid_size) {
	set_vertex(&vert->pos, &vert->normal, &vert->uv0, x, y, z, grid_size);
}

KH_INTERN u32
get_or_create_tri_mesh_sphere(Assets *assets, char *name = 0) {
	u32 id;
	if(name) {
		id = get_or_create_asset_id_from_name(assets, name);
	} else {
		id = get_or_create_asset_id_from_name(assets, assets->primitive_names[Primitive_sphere]);
	}
	Asset *asset = assets->arr +id;
	if(asset->state != AssetState_loaded) {

		asset->source = {};
		asset->source.type.key = AssetType_trimesh;
		asset->state = AssetState_loaded;

		const i32 grid_size = 6;
		b32 even = (grid_size % 2 == 0);

		u32 corner_vertex_count = 8;
		u32 edge_vertex_count = (grid_size + grid_size + grid_size - 3) * 4;
		u32 face_vertex_count = ((grid_size - 1) * (grid_size - 1) +
		                         (grid_size - 1) * (grid_size - 1) +
		                         (grid_size - 1) * (grid_size - 1)) * 2;

		u32 tri_count = grid_size * grid_size * 12;
		u32 add_vertices_count = 0;
		if(even) {
			add_vertices_count = (grid_size * 2) - 1;
		}
		u32 vertices_count = corner_vertex_count + edge_vertex_count + face_vertex_count + add_vertices_count;

		VertexFormat format = VertexFormat_PNU;
		u32 vertex_size = get_size_from_vertex_format(format);

		u32 vertices_size = vertices_count * vertex_size; 
		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 size = vertices_size + indices_size;

		asset->source.type.trimesh.vert_c = vertices_count;
		asset->source.type.trimesh.tri_c = tri_count;
		asset->source.type.trimesh.vertex_size = vertex_size;
		asset->source.type.trimesh.format = format;
		asset->source.type.trimesh.indices_offset = vertices_size;

		u8 *dst = add_asset_to_data_cache(assets, asset, size);
		Vertex_PNU *vertices = (Vertex_PNU *)dst;
		u32 *indices = (u32 *)(dst + vertices_size);

		i32 v = 0;
		for (i32 y = 0; y <= grid_size; y++) {
			for (i32 x = 0; x <= grid_size; x++) {
				set_vertex_pnu(&vertices[v++], x, y, 0, grid_size);
			}
			for (i32 z = 1; z <= grid_size; z++) {
				set_vertex_pnu(&vertices[v++], grid_size, y, z, grid_size);
			}
			for (i32 x = grid_size - 1; x >= 0; x--) {
				set_vertex_pnu(&vertices[v++], x, y, grid_size, grid_size);
			}
			for (i32 z = grid_size - 1; z > 0; z--) {
				set_vertex_pnu(&vertices[v++], 0, y, z, grid_size);
			}
		}
		for (i32 z = 1; z < grid_size; z++) {
			for (i32 x = 1; x < grid_size; x++) {
				set_vertex_pnu(&vertices[v++], x, grid_size, z, grid_size);
			}
		}
		for (i32 z = 1; z < grid_size; z++) {
			for (i32 x = 1; x < grid_size; x++) {
				set_vertex_pnu(&vertices[v++], x, 0, z, grid_size);
			}
		}

		u32 add_vertices = v;
		kh_assert(add_vertices == vertices_count - add_vertices_count);
		if(even) {
			for(i32 y = 0; y <= grid_size; ++y) {
				Vertex_PNU *vert = vertices + v++;
				set_vertex_pnu(vert, 0, y, (grid_size / 2), grid_size);
				kh_assert(vert->uv0.x == 1.0f);
				vert->uv0.x = 1.0f - vert->uv0.x;
			}

			for(i32 x = 1; x < (grid_size / 2); ++x) {
				Vertex_PNU *vert = vertices + v++;
				set_vertex_pnu(vert, x, grid_size, (grid_size / 2), grid_size);
				kh_assert(vert->uv0.x == 1.0f);
				vert->uv0.x = 1.0f - vert->uv0.x;
			}

			for(i32 x = 1; x < (grid_size / 2); ++x) {
				Vertex_PNU *vert = vertices + v++;
				set_vertex_pnu(vert, x, 0, (grid_size / 2), grid_size);
				kh_assert(vert->uv0.x == 1.0f);
				vert->uv0.x = 1.0f - vert->uv0.x;
			}
		}

		kh_assert((u32)v == vertices_count);

		u32 *triangles_z = indices;
		u32 *triangles_x = triangles_z + (grid_size * grid_size * 12);
		u32 *triangles_y = triangles_x + (grid_size * grid_size * 12);
		i32 ring = (grid_size + grid_size) * 2;
		i32 tz = 0, tx = 0, ty = 0; 
		v = 0;

		for (i32 y = 0; y < grid_size; y++, v++) {
			for (i32 q = 0; q < grid_size; q++, v++) {
				tz = set_quad_indices(triangles_z, tz, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size; q++, v++) {
				tx = set_quad_indices(triangles_x, tx, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size; q++, v++) {
				tz = set_quad_indices(triangles_z, tz, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size - 1; q++, v++) {
				tx = set_quad_indices(triangles_x, tx, v, v + 1, v + ring, v + ring + 1);
			}
			tx = set_quad_indices(triangles_x, tx, v, v - ring + 1, v + ring, v + 1);
		}

		v = ring * grid_size;
		for (i32 x = 0; x < grid_size - 1; x++, v++) {
			ty = set_quad_indices(triangles_y, ty, v, v + 1, v + ring - 1, v + ring);
		}
		ty = set_quad_indices(triangles_y, ty, v, v + 1, v + ring - 1, v + 2);

		i32 v_min = ring * (grid_size + 1) - 1;
		i32 v_mid = v_min + 1;
		i32 v_max = v + 2;

		for (i32 z = 1; z < grid_size - 1; z++, v_min--, v_mid++, v_max++) {
			ty = set_quad_indices(triangles_y, ty, v_min, v_mid, v_min - 1, v_mid + grid_size - 1);
			for (i32 x = 1; x < grid_size - 1; x++, v_mid++) {
				ty = set_quad_indices(
				                      triangles_y, ty,
				                      v_mid, v_mid + 1, v_mid + grid_size - 1, v_mid + grid_size);
			}
			ty = set_quad_indices(triangles_y, ty, v_mid, v_max, v_mid + grid_size - 1, v_max + 1);
		}

		i32 v_top = v_min - 2;
		ty = set_quad_indices(triangles_y, ty, v_min, v_mid, v_top + 1, v_top);
		for (i32 x = 1; x < grid_size - 1; x++, v_top--, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_mid, v_mid + 1, v_top, v_top - 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_mid, v_top - 2, v_top, v_top - 1);
		v = 1;
		v_mid = (vertices_count - add_vertices_count) - (grid_size - 1) * (grid_size - 1);
		ty = set_quad_indices(triangles_y, ty, ring - 1, v_mid, 0, 1);
		for (int x = 1; x < grid_size - 1; x++, v++, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_mid, v_mid + 1, v, v + 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_mid, v + 2, v, v + 1);

		v_min = ring - 2;
		v_mid -= grid_size - 2;
		v_max = v + 2;
		for (int z = 1; z < grid_size - 1; z++, v_min--, v_mid++, v_max++) {
			ty = set_quad_indices(triangles_y, ty, v_min, v_mid + grid_size - 1, v_min + 1, v_mid);
			for (int x = 1; x < grid_size - 1; x++, v_mid++) {
				ty = set_quad_indices(
				                      triangles_y, ty,
				                      v_mid + grid_size - 1, v_mid + grid_size, v_mid, v_mid + 1);
			}
			ty = set_quad_indices(triangles_y, ty, v_mid + grid_size - 1, v_max + 1, v_mid, v_max);
		}
		v_top = v_min - 1;
		ty = set_quad_indices(triangles_y, ty, v_top + 1, v_top, v_top + 2, v_mid);
		for (int x = 1; x < grid_size - 1; x++, v_top--, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_top, v_top - 1, v_mid, v_mid + 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_top, v_top - 1, v_mid, v_top - 2);

		if(even) {
			for(u32 i = 0; i < tri_count; ++i) {

				u32 *inds = indices + (i * 3);
				Vertex_PNU *vert_0 = vertices + inds[0];
				Vertex_PNU *vert_1 = vertices + inds[1];
				Vertex_PNU *vert_2 = vertices + inds[2];

				f32 dist_10 = kh_abs_f32(vert_1->uv0.x - vert_0->uv0.x);
				f32 dist_20 = kh_abs_f32(vert_2->uv0.x - vert_0->uv0.x);
				f32 dist_21 = kh_abs_f32(vert_2->uv0.x - vert_1->uv0.x);

				if(dist_10 > 0.5f) {
					if(vert_0->uv0.x == 1.0f) {
						kh_assert(vert_1->uv0.x < vert_0->uv0.x);
						for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
							if(vert_0->uv0.y == vertices[j].uv0.y) {
								inds[0] = j;
								break;
							}
						}
						if(vert_2->uv0.x == 1.0f) {
							for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
								if(vert_2->uv0.y == vertices[j].uv0.y) {
									inds[2] = j;
									break;
								}
							}
						}
					} else {
						kh_assert(vert_1->uv0.x == 1.0f);
						kh_assert(vert_2->uv0.x != 1.0f);
						for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
							if(vert_1->uv0.y == vertices[j].uv0.y) {
								inds[1] = j;
								break;
							}
						}
					}
				} else if(dist_20 > 0.5f) {
					if(vert_0->uv0.x == 1.0f) {
						kh_assert(vert_2->uv0.x < vert_0->uv0.x);
						for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
							if(vert_0->uv0.y == vertices[j].uv0.y) {
								inds[0] = j;
								break;
							}
						}
						if(vert_1->uv0.x == 1.0f) {
							for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
								if(vert_1->uv0.y == vertices[j].uv0.y) {
									inds[1] = j;
									break;
								}
							}
						}
					} else {
						kh_assert(vert_2->uv0.x == 1.0f);
						kh_assert(vert_1->uv0.x != 1.0f);
						for(u32 j = add_vertices; j < (add_vertices + add_vertices_count); ++j) {
							if(vert_2->uv0.y == vertices[j].uv0.y) {
								inds[2] = j;
								break;
							}
						}

					}
				} else {
					kh_assert(dist_21 < 0.5f);
				}
			}
		}
	}
	return(id);
}

KH_INTERN u32
get_or_create_tri_mesh_plane(Assets *assets) {
	u32 id = get_or_create_asset_id_from_name(assets, assets->primitive_names[Primitive_plane]);
	Asset *asset = assets->arr +id;
	if(asset->state != AssetState_loaded) {

		asset->source = {};
		asset->source.type.key = AssetType_trimesh;
		asset->state = AssetState_loaded;

		VertexFormat format = VertexFormat_PNU;
		u32 vertex_size = get_size_from_vertex_format(format);

		u32 tri_count = 2;
		u32 vertices_count = 4;

		u32 vertices_size = vertices_count * vertex_size; 
		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 size = vertices_size + indices_size;

		u8 *dst = add_asset_to_data_cache(assets, asset, size);
		Vertex_PNU *vertices = (Vertex_PNU *)dst;
		u32 *indices = (u32 *)(dst + vertices_size);

		asset->source.type.trimesh.vert_c = vertices_count;
		asset->source.type.trimesh.tri_c = tri_count;
		asset->source.type.trimesh.vertex_size = vertex_size;
		asset->source.type.trimesh.format = format;
		asset->source.type.trimesh.indices_offset = vertices_size;

		vertices[0].pos    = kh_vec3(-1, -1, 0);
		vertices[0].uv0    = kh_vec2(0,0);
		vertices[0].normal = kh_vec3(0, 0, -1);
		vertices[1].pos    = kh_vec3(1, -1, 0);
		vertices[1].uv0    = kh_vec2(1,0);
		vertices[1].normal = kh_vec3(0, 0, -1);
		vertices[2].pos    = kh_vec3(1, 1, 0);
		vertices[2].uv0    = kh_vec2(1,1);
		vertices[2].normal = kh_vec3(0, 0, -1);
		vertices[3].pos    = kh_vec3(-1, 1, 0);
		vertices[3].uv0    = kh_vec2(0,1);
		vertices[3].normal = kh_vec3(0, 0, -1);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;

		indices[3] = 3;
		indices[4] = 1;
		indices[5] = 2;

		// indices[6] = 0;
		// indices[7] = 3;
		// indices[8] = 1;

		// indices[9] = 3;
		// indices[10] = 2;
		// indices[11] = 1;
	}

	return(id);
}

KH_INTERN u32
get_or_create_tri_mesh_double_sided_plane(Assets *assets, char *name = 0) {
	u32 id; 
	if(name) {
		id = get_or_create_asset_id_from_name(assets, name); 
	} else {
		id = get_or_create_asset_id_from_name(assets, assets->primitive_names[Primitive_double_sided_plane]);
	}
	Asset *asset = assets->arr +id;
	if(asset->state != AssetState_loaded) {

		asset->source = {};
		asset->source.type.key = AssetType_trimesh;
		asset->state = AssetState_loaded;

		VertexFormat format = VertexFormat_PNU;
		u32 vertex_size = get_size_from_vertex_format(format);

		u32 tri_count = 4;
		u32 vertices_count = 8;

		u32 vertices_size = vertices_count * vertex_size; 
		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 size = vertices_size + indices_size;

		u8 *dst = add_asset_to_data_cache(assets, asset, size);
		Vertex_PNU *vertices = (Vertex_PNU *)dst;
		u32 *indices = (u32 *)(dst + vertices_size);

		asset->source.type.trimesh.vert_c = vertices_count;
		asset->source.type.trimesh.tri_c = tri_count;
		asset->source.type.trimesh.vertex_size = vertex_size;
		asset->source.type.trimesh.format = format;
		asset->source.type.trimesh.indices_offset = vertices_size;

		vertices[0].pos    = kh_vec3(-1, -1, 0);
		vertices[0].uv0    = kh_vec2(0,0);
		vertices[0].normal = kh_vec3(0, 0, -1);
		vertices[1].pos    = kh_vec3(1, -1, 0);
		vertices[1].uv0    = kh_vec2(1,0);
		vertices[1].normal = kh_vec3(0, 0, -1);
		vertices[2].pos    = kh_vec3(1, 1, 0);
		vertices[2].uv0    = kh_vec2(1,1);
		vertices[2].normal = kh_vec3(0, 0, -1);
		vertices[3].pos    = kh_vec3(-1, 1, 0);
		vertices[3].uv0    = kh_vec2(0,1);
		vertices[3].normal = kh_vec3(0, 0, -1);
		vertices[4].pos    = kh_vec3(-1, -1, 0);
		vertices[4].uv0    = kh_vec2(0,0);
		vertices[4].normal = kh_vec3(0, 0, 1);
		vertices[5].pos    = kh_vec3(1, -1, 0);
		vertices[5].uv0    = kh_vec2(1,0);
		vertices[5].normal = kh_vec3(0, 0, 1);
		vertices[6].pos    = kh_vec3(1, 1, 0);
		vertices[6].uv0    = kh_vec2(1,1);
		vertices[6].normal = kh_vec3(0, 0, 1);
		vertices[7].pos    = kh_vec3(-1, 1, 0);
		vertices[7].uv0    = kh_vec2(0,1);
		vertices[7].normal = kh_vec3(0, 0, 1);

		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 3;

		indices[3] = 3;
		indices[4] = 1;
		indices[5] = 2;

		indices[6] = 7;
		indices[7] = 5;
		indices[8] = 4;

		indices[9] = 6;
		indices[10] = 5;
		indices[11] = 7;
	}

	return(id);
}

KH_INTERN void
set_vertex_pnuc(Vertex_PNUC *vert, i32 x, i32 y, i32 z, i32 grid_size) {
	set_vertex(&vert->pos, &vert->normal, &vert->uv0, x, y, z, grid_size);
	vert->color = kh_vec3(1,1,1);
}

KH_INTERN u32
get_or_create_tri_mesh_sphere_pnuc(Assets *assets, char *name, const i32 grid_size) {
	u32 id = get_or_create_asset_id_from_name(assets, name);
	Asset *asset = assets->arr +id;
	if(asset->state != AssetState_loaded) {

		asset->source = {};
		asset->source.type.key = AssetType_trimesh;
		asset->state = AssetState_loaded;

		u32 corner_vertex_count = 8;
		u32 edge_vertex_count = (grid_size + grid_size + grid_size - 3) * 4;
		u32 face_vertex_count = ((grid_size - 1) * (grid_size - 1) +
		                         (grid_size - 1) * (grid_size - 1) +
		                         (grid_size - 1) * (grid_size - 1)) * 2;

		u32 tri_count = grid_size * grid_size * 12;
		u32 vertices_count = corner_vertex_count + edge_vertex_count + face_vertex_count;

		VertexFormat format = VertexFormat_PNUT;
		u32 vertex_size = get_size_from_vertex_format(format);

		u32 vertices_size = vertices_count * vertex_size; 
		u32 indices_size = tri_count * 3 * sizeof(u32);
		u32 size = vertices_size + indices_size;

		asset->source.type.trimesh.vert_c = vertices_count;
		asset->source.type.trimesh.tri_c = tri_count;
		asset->source.type.trimesh.vertex_size = vertex_size;
		asset->source.type.trimesh.format = format;
		asset->source.type.trimesh.indices_offset = vertices_size;

		u8 *dst = add_asset_to_data_cache(assets, asset, size);
		Vertex_PNUC *vertices = (Vertex_PNUC *)dst;
		u32 *indices = (u32 *)(dst + vertices_size);

		i32 v = 0;
		for (i32 y = 0; y <= grid_size; y++) {
			for (i32 x = 0; x <= grid_size; x++) {
				set_vertex_pnuc(&vertices[v++], x, y, 0, grid_size);
			}
			for (i32 z = 1; z <= grid_size; z++) {
				set_vertex_pnuc(&vertices[v++], grid_size, y, z, grid_size);
			}
			for (i32 x = grid_size - 1; x >= 0; x--) {
				set_vertex_pnuc(&vertices[v++], x, y, grid_size, grid_size);
			}
			for (i32 z = grid_size - 1; z > 0; z--) {
				set_vertex_pnuc(&vertices[v++], 0, y, z, grid_size);
			}
		}
		for (i32 z = 1; z < grid_size; z++) {
			for (i32 x = 1; x < grid_size; x++) {
				set_vertex_pnuc(&vertices[v++], x, grid_size, z, grid_size);
			}
		}
		for (i32 z = 1; z < grid_size; z++) {
			for (i32 x = 1; x < grid_size; x++) {
				set_vertex_pnuc(&vertices[v++], x, 0, z, grid_size);
			}
		}

		kh_assert((u32)v == vertices_count);

		u32 *triangles_z = indices;
		u32 *triangles_x = triangles_z + (grid_size * grid_size * 12);
		u32 *triangles_y = triangles_x + (grid_size * grid_size * 12);
		i32 ring = (grid_size + grid_size) * 2;
		i32 tz = 0, tx = 0, ty = 0; 
		v = 0;

		for (i32 y = 0; y < grid_size; y++, v++) {
			for (i32 q = 0; q < grid_size; q++, v++) {
				tz = set_quad_indices(triangles_z, tz, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size; q++, v++) {
				tx = set_quad_indices(triangles_x, tx, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size; q++, v++) {
				tz = set_quad_indices(triangles_z, tz, v, v + 1, v + ring, v + ring + 1);
			}
			for (i32 q = 0; q < grid_size - 1; q++, v++) {
				tx = set_quad_indices(triangles_x, tx, v, v + 1, v + ring, v + ring + 1);
			}
			tx = set_quad_indices(triangles_x, tx, v, v - ring + 1, v + ring, v + 1);
		}

		v = ring * grid_size;
		for (i32 x = 0; x < grid_size - 1; x++, v++) {
			ty = set_quad_indices(triangles_y, ty, v, v + 1, v + ring - 1, v + ring);
		}
		ty = set_quad_indices(triangles_y, ty, v, v + 1, v + ring - 1, v + 2);

		i32 v_min = ring * (grid_size + 1) - 1;
		i32 v_mid = v_min + 1;
		i32 v_max = v + 2;

		for (i32 z = 1; z < grid_size - 1; z++, v_min--, v_mid++, v_max++) {
			ty = set_quad_indices(triangles_y, ty, v_min, v_mid, v_min - 1, v_mid + grid_size - 1);
			for (i32 x = 1; x < grid_size - 1; x++, v_mid++) {
				ty = set_quad_indices(
				                      triangles_y, ty,
				                      v_mid, v_mid + 1, v_mid + grid_size - 1, v_mid + grid_size);
			}
			ty = set_quad_indices(triangles_y, ty, v_mid, v_max, v_mid + grid_size - 1, v_max + 1);
		}

		i32 v_top = v_min - 2;
		ty = set_quad_indices(triangles_y, ty, v_min, v_mid, v_top + 1, v_top);
		for (i32 x = 1; x < grid_size - 1; x++, v_top--, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_mid, v_mid + 1, v_top, v_top - 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_mid, v_top - 2, v_top, v_top - 1);
		v = 1;
		v_mid = (vertices_count) - (grid_size - 1) * (grid_size - 1);
		ty = set_quad_indices(triangles_y, ty, ring - 1, v_mid, 0, 1);
		for (int x = 1; x < grid_size - 1; x++, v++, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_mid, v_mid + 1, v, v + 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_mid, v + 2, v, v + 1);

		v_min = ring - 2;
		v_mid -= grid_size - 2;
		v_max = v + 2;
		for (int z = 1; z < grid_size - 1; z++, v_min--, v_mid++, v_max++) {
			ty = set_quad_indices(triangles_y, ty, v_min, v_mid + grid_size - 1, v_min + 1, v_mid);
			for (int x = 1; x < grid_size - 1; x++, v_mid++) {
				ty = set_quad_indices(
				                      triangles_y, ty,
				                      v_mid + grid_size - 1, v_mid + grid_size, v_mid, v_mid + 1);
			}
			ty = set_quad_indices(triangles_y, ty, v_mid + grid_size - 1, v_max + 1, v_mid, v_max);
		}
		v_top = v_min - 1;
		ty = set_quad_indices(triangles_y, ty, v_top + 1, v_top, v_top + 2, v_mid);
		for (int x = 1; x < grid_size - 1; x++, v_top--, v_mid++) {
			ty = set_quad_indices(triangles_y, ty, v_top, v_top - 1, v_mid, v_mid + 1);
		}
		ty = set_quad_indices(triangles_y, ty, v_top, v_top - 1, v_mid, v_top - 2);
	}
	return(id);
}
