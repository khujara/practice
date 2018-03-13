inline f32
path_rand(PathSolution *solution) {
	f32 res = rand_01_f32(&solution->entropy);
	return(res);
}

#if 1
static Triangle
get_triangle_from_prim(const PathSolution *solution, const Primitive *prim) {
	Triangle res;
	Entity *entity = get_entity(solution->scene, prim->pool_entity);
	MeshRenderer *meshr = get_entity_meshr(solution->render, entity);
	Asset *asset = get_asset(solution->assets, meshr->mesh, AssetType_trimesh);
	VertexFormat fmt = asset->source.type.trimesh.format;
	VertexAttribute attrib = get_vertex_attribute(fmt);

	u8 *mesh = get_datas(solution->assets, meshr->mesh, AssetType_trimesh);
	u32 *indices = (u32 *)(mesh + asset->source.type.trimesh.vert_c * attrib.vertex_size);	


	v3 v_0 = *(v3 *)(mesh + (indices[prim->first_index_offset + 0] * attrib.vertex_size) + attrib.pos_offset);
	v3 v_1 = *(v3 *)(mesh + (indices[prim->first_index_offset + 1] * attrib.vertex_size) + attrib.pos_offset);
	v3 v_2 = *(v3 *)(mesh + (indices[prim->first_index_offset + 2] * attrib.vertex_size) + attrib.pos_offset);

	res.v_0 = entity->transform * v_0;
	res.v_1 = entity->transform * v_1;
	res.v_2 = entity->transform * v_2;
	return(res);
}
#else
static Triangle
get_triangle_from_prim(const PathSolution *solution, const Primitive *prim) {
	Triangle res = prim->tri;
	return(res);
}
#endif

KH_INTERN void 
bvh_leaf_node(BVHTree *tree, BVHNode *node, u32 first_prim, u32 prim_count, AABB *box) {
	node->first_prim_offset = first_prim;
	node->prim_count = prim_count;
	node->box = *box;
	node->left = 0;
	node->right = 0;

	tree->leaf_count++;
}

KH_INTERN void 
bvh_interior_node(BVHTree *tree, BVHNode *node, u32 axis, BVHNode *left, BVHNode *right) {
	node->left = left;
	node->right = right;
	node->box = aabb_union(left->box, right->box);
	node->split_axis = axis;
	node->prim_count = 0;
	tree->interior_count++;
}

static v3 
random_in_unit_circle(PathSolution *solution) {
	v3 p;
	 do {
	 	p = 2.0f*kh_vec3(path_rand(solution), path_rand(solution), 0) - kh_vec3(1,1,0);
	 }while(kh_dot_v3(p,p) >= 1.0f);
	 return(p);
}

static v3
random_in_unit_sphere(PathSolution *solution) {
	v3 res;
#if 0
	res = 2.0f*kh_normalize_v3(kh_vec3(drand48(), drand48(), drand48())) - kh_vec3(1,1,1);
#else
	do {
		res = 2.0f*kh_vec3(path_rand(solution), path_rand(solution), path_rand(solution)) - kh_vec3(1,1,1); 
	} while(kh_dot_v3(res, res) >= 1.0f);
#endif
	return(res);
}

static PathRay
get_ray_from_camera(PathSolution *solution, LensCamera *cam, f32 u, f32 v) {
	PathRay res;
	v3 rd = cam->lens_radius * random_in_unit_circle(solution);
	v3 offset = cam->x_axis * rd.x + cam->y_axis * rd.y;
	res.orig = cam->origin + offset;
	res.dir = cam->lower_left_corner + u*cam->horizontal + v*cam->vertical - cam->origin - offset;
	res.tmin = 0.0f;
	res.tmax = F32_MAX; 
	return(res);
}

static LensCamera
ray_camera_lookat(Camera *cam, f32 ar, f32 aperture, f32 focus_dist, f32 t0 = 0.0f, f32 t1 = 0.0f) {
	LensCamera res;
	// NOTE(flo): used for depth of field/defocus blur effect
	// f32 ar = (f32)cam->w/(f32)cam->h;
	res.lens_radius = aperture / 2.0f; 
	f32 theta = cam->fov * TO_RADIANS * 0.5f;
	f32 half_h = kh_tan_f32(theta);
	f32 half_w = ar * half_h;

	res.origin = cam->tr.pos;
	res.x_axis = kh_vec3(cam->view.c0.x, cam->view.c1.x, cam->view.c2.x);
	res.y_axis = kh_vec3(cam->view.c0.y, cam->view.c1.y, cam->view.c2.y);
	res.z_axis = kh_vec3(cam->view.c0.z, cam->view.c1.z, cam->view.c2.z);
	res.lower_left_corner = cam->tr.pos - half_w*focus_dist*res.x_axis - half_h*focus_dist*res.y_axis + focus_dist*res.z_axis;
	res.horizontal = 2*half_w*focus_dist*res.x_axis;
	res.vertical = 2*half_h*focus_dist*res.y_axis;

	return(res);
}

static f32
fresnel_schlick_approximation(f32 cosine, f32 refract_index) {
	// Shclick Approximation R(a) = R0 + (1 - R0)(1 - cos(a))^5
// where R0 = ((n1 - n2) / (n1 + n2))^2 is reflectivity (for common glass in air n1 = 1, n2 = 1.5f)
	f32 n1 = 1.0f;
	f32 n2 = refract_index;
	// f32 n2 = 1.0f;
	// f32 n1 = refract_index;
	f32 r0 = (n1 - n2) / (n1 + n2);
	r0 = r0*r0;
	// t = 1 - r0 = probability of transmission
	f32 res = r0 + (1 - r0)*kh_pow_f32((1 - cosine), 5.0f);
	return(res);
}

static Refraction
refract(v3 v, v3 n, f32 n1_over_n2) {
	Refraction res = {};
	v3 uv = kh_normalize_v3(v);
	f32 dt = kh_dot_v3(uv, n);
	f32 snell = 1.0f - n1_over_n2*n1_over_n2*(1-dt*dt);
	if(snell > 0) {
		res.dir = n1_over_n2*(uv - n*dt) - n*kh_sqrt_f32(snell);
		res.refracted = true;
	}
	return(res);
}

inline v3 
reflect(v3 v, v3 n) {
	v3 res = v - 2 * kh_dot_v3(v,n)*n;
	return(res);
}

static b32 
hit_AABB(const PathRay *ray, const AABB *box) {
	f32 tmin = ray->tmin;
	f32 tmax = ray->tmax;
	// NOTE(flo): clip line by line
	kh_lu0(axis_i, 3) {
		f32 inv_dir = 1.0f / ray->dir.e[axis_i];
		f32 t0 = (box->min.e[axis_i] - ray->orig.e[axis_i]) * inv_dir;
		f32 t1 = (box->max.e[axis_i] - ray->orig.e[axis_i]) * inv_dir;
		if(inv_dir < 0.0f) {
			kh_swap_val(f32, t0, t1, tmp);
			// f32 tmp = t0;
			// t0 = t1;
			// t1 = tmp;
		}
		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;
		if(tmax < tmin) {
			return(false);
		}
	}
	return(true);
}

static b32
primitive_ray_intersect(const PathSolution *solution, const Primitive *prim, const PathRay *ray, SurfaceInteraction *isect) {
	b32 res = false;
	Triangle tri = get_triangle_from_prim(solution, prim);
	v3 e10 = tri.v_1 - tri.v_0;
	v3 e20 = tri.v_2 - tri.v_0;
	v3 n_nn = kh_cross_v3(e10, e20);
	f32 n_len = kh_length_v3(n_nn);
	v3 n = n_nn / n_len; 
	f32 nom = kh_dot_v3(tri.v_0 - ray->orig, n);	
	f32 n_dot_dir = kh_dot_v3(ray->dir, n);
	f32 t = kh_safe_ratio0_f32(nom, n_dot_dir);
	if(t > ray->tmin && t < ray->tmax) {
		v3 p = ray->orig + ray->dir * t;
		v3 c;

		v3 vp0 = p - tri.v_0;
		c = kh_cross_v3(tri.v_1 - tri.v_0, vp0);
		f32 test = kh_dot_v3(n, c);
		if(test <= 0.0f) return(false);

		v3 vp1 = p - tri.v_1;
		c = kh_cross_v3(tri.v_2 - tri.v_1, vp1);
		test = kh_dot_v3(n, c);
		if(test <= 0.0f) return(false);

		v3 vp2 = p - tri.v_2;
		c = kh_cross_v3(tri.v_0 - tri.v_2, vp2);
		test = kh_dot_v3(n, c);
		if(test <= 0.0f) return(false);

		isect->pos = p;
		isect->t = t;
		isect->normal_nn = n_nn;
		isect->normal_len = n_len;
		res = true;
	}
	return(res);
}

static SurfaceInteraction
intersect_bvh(PathSolution *solution, PathRay *ray, u32 src_index) {
	SurfaceInteraction res = {};
	BVHNode *nodes_to_visit[64];
	BVHNode *current_node = solution->bvh.root;
	u32 to_visit_offset = 0;
	while(true) {
		const BVHNode *node = current_node;
		if(hit_AABB(ray, &node->box)) {
			if(node->prim_count > 0) {
				for(u32 prim_i = 0; prim_i < node->prim_count; ++prim_i) {
					u32 prim_index = node->first_prim_offset + prim_i;
					if(prim_index != src_index) {
						Primitive *prim = solution->prim_list.ptr + prim_index;
						b32 shape_hit = primitive_ray_intersect(solution, prim, ray, &res);
						if(shape_hit) {
							res.prim_index = prim_index;	
							res.hit = true;
							ray->tmax = res.t;
						}
					}
				}
				if(to_visit_offset == 0) break;
				current_node = nodes_to_visit[--to_visit_offset];
			} else {
				kh_assert(to_visit_offset < array_count(nodes_to_visit));
				nodes_to_visit[to_visit_offset++] = node->right;
				current_node = node->left;
			}
		} else {
			if(to_visit_offset == 0) break;
			current_node = nodes_to_visit[--to_visit_offset];
		}
	}
	return(res);
}

static AABB
get_bounding_box(const PathSolution *solution, const Primitive *prim) {
	Triangle tri = get_triangle_from_prim(solution, prim);
	AABB res = get_triangle_bounding_box(&tri);
	return(res);
}

static BVHNode *
bvh_recursive_build(PathSolution *solution, u32 start, u32 end) {
	BVHNode *res = 0;
	u32 axis = kh_clamp_u32(0, 2, (u32)(3*path_rand(solution)));
	u32 n = end - start;
	res = kh_push_struct(solution->bvh.arena, BVHNode);
	*res = {};

	u32 mid = (start + end) / 2;

	if(n == 1) {
		res->first_prim_offset = start;
		res->prim_count = 1;
		res->box = get_bounding_box(solution, solution->prim_list.ptr + start);
		res->type = BVHNode_leaf;
		solution->bvh.leaf_count++;
		return(res);
	} else {
		std::nth_element(solution->prim_list.ptr + start, solution->prim_list.ptr + mid, solution->prim_list.ptr + end - 1,[solution, axis](const Primitive &prim_a, const Primitive &prim_b) {
			Triangle a = get_triangle_from_prim(solution, &prim_a);
			Triangle b = get_triangle_from_prim(solution, &prim_b);

			AABB box_left = get_triangle_bounding_box(&a);
			AABB box_right = get_triangle_bounding_box(&b);

			return(box_left.min.e[axis] < box_right.min.e[axis]);

		});

		res->type = BVHNode_interior;
		solution->bvh.interior_count++;
		res->left = bvh_recursive_build(solution, start, mid);
		res->right = bvh_recursive_build(solution, mid, end);

		if(!res->left) {
			kh_assert(res->right);
			res->left = res->right;
		} else if(!res->right) {
			res->right = res->left;
		}
		res->box = aabb_union(res->left->box, res->right->box);
	}
	return(res);
}

static void
get_primitives_from_scene(PathSolution *solution) {
	PrimitiveList *prim_list = &solution->prim_list;
	Scene *scene = solution->scene;
	RenderManager *render = solution->render;
	Assets *assets = solution->assets;
	prim_list->count = 0;
	u32 entity_count = 0;
	kh_lu0(pool_i, scene->pool_count) {
		SceneEntity *scene_entity = scene->entity_pool + pool_i;
		if(!scene_entity->next_free) {
			Entity *entity = &scene_entity->entity;
			kh_assert(entity->mesh_renderer + sizeof(MeshRenderer) < render->commands_size);
			MeshRenderer *meshr = get_entity_meshr(render, entity);
			kh_assert(meshr->loaded);
			if(meshr->loaded) {
				Asset *trimesh_asset = assets->arr + meshr->mesh.val;
				TriangleMesh trimesh = trimesh_asset->source.type.trimesh;
				kh_lu0(tri_i, trimesh.tri_c) {
					kh_assert(prim_list->count < prim_list->max_count);
					Primitive *prim = prim_list->ptr + prim_list->count++;
					prim->pool_entity = pool_i;
					prim->first_index_offset = tri_i * 3;
				}
				entity_count++;
			}
		}	
	}
	kh_assert(prim_list->count < prim_list->max_count);
	// Primitive *prim = prim_list->ptr + prim_list->count++;
	// *prim = {};
	kh_assert(entity_count == scene->entity_count);
}

static void
init_path_solution(PathSolution *solution, Assets *assets, RenderManager *render, Scene *scene, LinearArena *arena) {
	solution->assets = assets;
	solution->render = render;
	solution->scene = scene;

	solution->out_texture = {get_or_create_empty_texture(assets, 512, 512, "pathtracer_texture")};

	Material_T1 *target = add_mat_instance(render, Shading_diffuse, VertexFormat_PNU, Material_T1);
	target->diffuse = solution->out_texture;
	AssetID plane_mesh_id  = {get_or_create_tri_mesh_plane(assets)};
	u32 plane_meshr = add_mesh_renderer(render, target, plane_mesh_id);
	mat4 tr = kh_identity_mat4();
	tr.c3 = kh_vec4(0, 3, 0, 1);
	solution->entity = new_entity(plane_meshr, tr);

	const u32 max_prim_count = 32768*2;
	kh_assert(solution->prim_list.ptr == 0);
	solution->prim_list.count = 0;
	solution->prim_list.max_count = max_prim_count;
	solution->prim_list.ptr = kh_push_array(arena, max_prim_count, Primitive);

	solution->rays_per_pixel = 8;

	solution->bvh = {};
	solution->bvh.arena = arena;

	solution->entropy = rand_seed(1234);

	stbi_set_flip_vertically_on_load(true);
	solution->skybox.pixels = stbi_loadf("sierra_madre.hdr", &solution->skybox.w, &solution->skybox.h, &solution->skybox.bpp, 0);
}

static void 
pathtracer_render(PathSolution *solution, Camera *in_cam) {
	if(!solution->bvh.root) {
		get_primitives_from_scene(solution);
		solution->bvh.root = bvh_recursive_build(solution, 0, solution->prim_list.count);
	}

	Scene *scene = solution->scene;
	RenderManager *render = solution->render;
	Assets *assets = solution->assets;

	LoadedAsset out_texture = get_loaded_asset(solution->assets, solution->out_texture, AssetType_tex2d);
	u32 w = out_texture.type->tex2d.width;
	u32 h = out_texture.type->tex2d.height;
	u32 bpp = out_texture.type->tex2d.bytes_per_pixel;
	u8 *texture_data = out_texture.data;
	kh_assert(bpp == 3);

	LensCamera lcam = ray_camera_lookat(in_cam, 1.0f, 0.0f, 10.0f);

	const f32 contrib = 1.0f / (f32)solution->rays_per_pixel;

	v3 light_pos = kh_vec3(0.707f, 0.707f, 0.0f);
	u32 pitch = w * bpp;
	u8 *row = texture_data;
	BVHNode *nodes_to_visit[64];

	const float RAY_DIR_EPSILON = 0.00001f;

	// TODO(flo): maybe use indices instead of straight up pointer for SurfaceMaterial and BVHNode

	for(u32 y = 0; y < h; ++y) {
		u8 *pixels = row;
		for(u32 x = 0; x < w; ++x) {
			v3 final_color = kh_vec3(0.0f,0.0f,0.0f);
			for(u32 i = 0; i < solution->rays_per_pixel; ++i) {
				// TODO(flo): better sampling technique
				f32 urand = path_rand(solution);
				f32 vrand = path_rand(solution); 
				f32 usampler = (f32)x + urand;
				f32 vsampler = (f32)y + vrand;
				f32 u = usampler / (f32)w;
				f32 v = vsampler / (f32)h;
				PathRay ray = get_ray_from_camera(solution, &lcam, u, v);

				SurfaceInteraction isect = {};
				BVHNode *current_node = solution->bvh.root;
				u32 to_visit_offset = 0;
				while(true) {
					const BVHNode *node = current_node;
					if(hit_AABB(&ray, &node->box)) {
						if(node->prim_count > 0) {
							for(u32 prim_i = 0; prim_i < node->prim_count; ++prim_i) {
								u32 prim_index = (node->first_prim_offset + prim_i);
								Primitive *prim = solution->prim_list.ptr + prim_index;
									// NOTE(flo): call here!
								b32 shape_hit = primitive_ray_intersect(solution, prim, &ray, &isect);
								if(shape_hit) {
									isect.prim_index = prim_index;
									isect.hit = true;
									ray.tmax = isect.t;
								}
							}
							if(to_visit_offset == 0) break;
							current_node = nodes_to_visit[--to_visit_offset];
						} else {
							kh_assert(to_visit_offset < array_count(nodes_to_visit));
							nodes_to_visit[to_visit_offset++] = node->right;
							current_node = node->left;
						}
					} else {
						if(to_visit_offset == 0) break;
						current_node = nodes_to_visit[--to_visit_offset];
					}
				}
				v3 direct_illumination = kh_vec3(0,0,0);
				v3 indirect_illumination = kh_vec3(0,0,0);

				if(isect.hit) {
					/* TODO(flo): 
							get the entity material properties from the prim_index
							calculate the barycentric coordinates (the weights) from the isect.pos
							get values we need here from info above :
								color (from uv)
								the new normal based on the normal map
								the metalness/roughness to calculate the bounce and the light attenuation
								what do we want to do with the ambient occlusion?
					*/

					Primitive *prim = solution->prim_list.ptr + isect.prim_index;
					Entity *entity = &scene->entity_pool[prim->pool_entity].entity;
					MeshRenderer *meshr = get_entity_meshr(render, entity);
					Material *mat = get_entity_material(render, entity);
					Shading *shading = get_entity_shading(render, entity);
					MaterialInfo *mat_info = render->mat_infos + shading->mat_type;

					VertexAttribute attrib = get_vertex_attribute(shading->format);

					TriangleMesh trimesh = get_trimesh(assets, meshr->mesh);
					u8 *mesh = get_datas(assets, meshr->mesh, AssetType_trimesh);
					u32 *indices = (u32 *)(mesh + trimesh.vert_c * attrib.vertex_size);
					u8 *mat_property = (u8 *)(mat + 1);

					v3 col = {};
					v3 N = isect.normal_nn / isect.normal_len;
					v3 P = isect.pos;
					if(shading->texture_count > 0) {
						AssetID texture_id = *(AssetID *)mat_property;
						Texture2D texture = get_texture(assets, texture_id);
						u8 *diffuse = get_datas(assets, texture_id, AssetType_tex2d); 

						v3 v_0 = *(v3 *)(mesh + (indices[prim->first_index_offset + 0] * attrib.vertex_size) + attrib.pos_offset);
						v3 v_1 = *(v3 *)(mesh + (indices[prim->first_index_offset + 1] * attrib.vertex_size) + attrib.pos_offset);
						v3 v_2 = *(v3 *)(mesh + (indices[prim->first_index_offset + 2] * attrib.vertex_size) + attrib.pos_offset);

						v2 uv_0 = *(v2 *)(mesh + (indices[prim->first_index_offset + 0] * attrib.vertex_size) + attrib.uv0_offset);
						v2 uv_1 = *(v2 *)(mesh + (indices[prim->first_index_offset + 1] * attrib.vertex_size) + attrib.uv0_offset);
						v2 uv_2 = *(v2 *)(mesh + (indices[prim->first_index_offset + 2] * attrib.vertex_size) + attrib.uv0_offset);

						v3 n_0 = *(v3 *)(mesh + (indices[prim->first_index_offset + 0] * attrib.vertex_size) + attrib.nor_offset);
						v3 n_1 = *(v3 *)(mesh + (indices[prim->first_index_offset + 1] * attrib.vertex_size) + attrib.nor_offset);
						v3 n_2 = *(v3 *)(mesh + (indices[prim->first_index_offset + 2] * attrib.vertex_size) + attrib.nor_offset);

						v_0 = entity->transform * v_0;
						v_1 = entity->transform * v_1;
						v_2 = entity->transform * v_2;

						v3 n = isect.normal_nn;
						f32 n_dot_n = kh_dot_v3(n,n);
						f32 inv_n_dot_n = 1.0f / n_dot_n;

						v3 e21= v_2 - v_1;
						v3 p0 = isect.pos - v_1;
						v3 w0 = kh_cross_v3(e21,p0);
						f32 tri_u = kh_dot_v3(n, w0) * inv_n_dot_n;

						v3 e02 = v_0 - v_2;
						v3 p1 = isect.pos - v_2; 
						v3 w1 = kh_cross_v3(e02,p1);
						f32 tri_v = kh_dot_v3(n, w1) * inv_n_dot_n;
						f32 tri_w = 1.0f - tri_u - tri_v;
						v2 uv = uv_0 * tri_u + uv_1 * tri_v + uv_2 * tri_w;
						v3 interp_normal = n_0 * tri_u + n_1 * tri_v + n_2 * tri_w;  

						u32 x_offset = uv.x * texture.width;
						u32 y_offset = uv.y * (texture.height - 1);
						u8 *src_0 = diffuse + x_offset * texture.bytes_per_pixel + y_offset * texture.width * texture.bytes_per_pixel;

						f32 inv_255 = 1.0f / 255.0f;
						col.x = inv_255 * (f32)src_0[2];
						col.y = inv_255 * (f32)src_0[1];
						col.z = inv_255 * (f32)src_0[0];
						N = kh_vec3(entity->transform * kh_vec4(interp_normal, 0.0f)); 
					}

					v3 wo = ray.dir;	
					v3 Lr = kh_vec3(0,0,0);
					kh_lu0(light_i, render->light_count) {
						Light *light = render->lights + light_i;
						v3 l_pos = light->pos;
						v3 to_light = l_pos - P;
						f32 dist = kh_length_v3(to_light);
						f32 inv_dist = 1.0f / dist;
						v3 L = to_light * inv_dist;
						f32 attenuation = inv_dist * inv_dist;
						v3 light_color = light->color * attenuation;
						// TODO(flo): non diffuse material!

						v3 Lrk = kh_hadamard_v3(light_color, (1.0f/(f32)PI32) * col) * kh_max_f32(kh_dot_v3(N, L), 0.0f);
						if(kh_dot_v3(Lrk, Lrk) > 0.0f) {
							PathRay shadow_ray = {};
							shadow_ray.orig = P;
							shadow_ray.dir = L;
							shadow_ray.tmin = 0.0f; 
							shadow_ray.tmax = dist;
							SurfaceInteraction shadow_isect = intersect_bvh(solution, &shadow_ray, isect.prim_index);
							if(!shadow_isect.hit) {
								Lr += Lrk;
							}
						}
					}
					direct_illumination = Lr;
					v3 attenuation = kh_vec3(1,1,1);
					kh_lu0(bounce_i, 1) {

						/* TODO(flo): compute indirect illumination with next event estimation
							use metal/rougness information from the uv if available
						*/
						PathRay bounce_ray;
						bounce_ray.orig = P;
						bounce_ray.dir = N;
						bounce_ray.tmin = RAY_DIR_EPSILON;
						bounce_ray.tmax = F32_MAX;
						SurfaceInteraction bounce_intersect = intersect_bvh(solution, &bounce_ray, isect.prim_index);
						if(!bounce_intersect.hit) {
							PathSkybox *skybox = &solution->skybox;
							if(skybox->w && skybox->h && skybox->pixels) {
								f32 one_over_pi = 0.3183f;
								f32 one_over_two_pi = 0.1591f;
								f32 phi = kh_atan2_f32(bounce_ray.dir.z, bounce_ray.dir.x);
								f32 theta = kh_asin_f32(bounce_ray.dir.y);
								v2 sky_uv = kh_vec2(phi * one_over_two_pi, theta * one_over_pi);
								sky_uv.x += 0.5f;
								sky_uv.y += 0.5f;

								u32 sky_x_offset = sky_uv.x * skybox->w;
								u32 sky_y_offset = sky_uv.y * (skybox->h - 1);
								f32 *src = skybox->pixels + sky_x_offset * skybox->bpp + sky_y_offset * skybox->w * skybox->bpp;

								v3 color = *(v3 *)src;
								indirect_illumination = INV_PI32 * color;
							}
						}
					}
				} else {
					// TODO(flo): skybox integration from the render manager
					PathSkybox *skybox = &solution->skybox;
					if(skybox->w && skybox->h && skybox->pixels) {
						v3 n = kh_normalize_v3(ray.dir);
						f32 one_over_pi = 0.3183f;
						f32 one_over_two_pi = 0.1591f;
						f32 phi = kh_atan2_f32(n.z, n.x);
						f32 theta = kh_asin_f32(n.y);
						v2 uv = kh_vec2(phi * one_over_two_pi, theta * one_over_pi);
						uv.x += 0.5f;
						uv.y += 0.5f;

						u32 x_offset = uv.x * skybox->w;
						u32 y_offset = uv.y * (skybox->h - 1);
						f32 *src = skybox->pixels + x_offset * skybox->bpp + y_offset * skybox->w * skybox->bpp;

						v3 color = *(v3 *)src;
						direct_illumination = color;
					}
				}
				final_color += direct_illumination + indirect_illumination;
			}
			final_color = final_color * contrib;
			// NOTE(flo): gamma correction approximation
			// final_color = kh_vec3(kh_sqrt_f32(final_color.x), kh_sqrt_f32(final_color.y), kh_sqrt_f32(final_color.z));
			final_color.x = kh_clamp01_f32(final_color.x);
			final_color.y = kh_clamp01_f32(final_color.y);
			final_color.z = kh_clamp01_f32(final_color.z);
			pixels[0] = (u8)(final_color.z * 255);
			pixels[1] = (u8)(final_color.y * 255);
			pixels[2] = (u8)(final_color.x * 255);
			pixels += bpp;

		}
		row += pitch;
	}
}