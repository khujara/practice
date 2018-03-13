KH_INTERN void
add_light(RenderManager *render, Light light) {
	kh_assert(render->light_count < MAX_LIGHTS);
	kh_assert(render->light_count == 0);
	render->lights[render->light_count++] = light;
}

// NOTE(flo): for now skybox and light should only be sent once, and not at each frame like other render commands
KH_INTERN void
define_skybox(RenderManager *render, Assets *assets, AssetID right, AssetID left, AssetID bottom, 
              AssetID top, AssetID back, AssetID front) {
	ask_for_asset_force_immediate(assets, right);
	ask_for_asset_force_immediate(assets, left);
	ask_for_asset_force_immediate(assets, bottom);
	ask_for_asset_force_immediate(assets, top);
	ask_for_asset_force_immediate(assets, back);
	ask_for_asset_force_immediate(assets, front);
	render->skybox.right = right;
	render->skybox.left = left;
	render->skybox.bottom = bottom;
	render->skybox.top = top;
	render->skybox.back = back;
	render->skybox.front = front;
	render->has_skybox = true;
}

KH_INLINE void
init_render(RenderManager *render) {

	kh_lu0(i, VertexFormat_count_or_none) {
		render->vertex_format_offset_hash[i] = -1;
	}
	kh_lu0(i, Shading_count) {
		render->shading_mat_type[i] = Type_Material_count;
	}
	kh_lu0(i, render->commands_at) {
		u8 *commands = render->commands + i;
		*commands = 0;
	}
	kh_lu0(i, render->mat_buffer_count) {
		MaterialHashEl *el = render->mat_buffer + i;
		el->cmd_offset = 0;
	}

	render->shading_mat_type[Shading_phong] = Type_Material_T1F3;
	render->shading_mat_type[Shading_normalmap] = Type_Material_T2F2;
	render->shading_mat_type[Shading_pbr] = Type_Material_T5;
	render->shading_mat_type[Shading_color] = Type_Material_F4;
	render->shading_mat_type[Shading_diffuse] = Type_Material_T1;

	render->commands_at = 0;
	render->first_vertex_buffer = 0;
	render->animators.count = 0;
}

KH_INLINE void
begin_render_frame(RenderManager *render, Assets *assets) {
	render->joint_tr.count = 0;
	render->entry_count = 0;
	render->batch_count = 0;
	for(VertexBuffer *buffer = render->first_vertex_buffer; buffer; buffer = buffer->next) {
		for(Shading *shading = buffer->first; shading; shading = shading->next) {
			for(Material *mat = shading->first; mat; mat = mat->next) {
				b32 mat_loaded = true;
				if(shading->texture_count > 0) {
					AssetID *textures = (AssetID *)(mat + 1);
					if(textures) {
						for(u32 i = 0; i < shading->texture_count; ++i) {
							mat_loaded &= ask_for_asset(assets, textures[i]);
						}
					}
				}
				for(MeshRenderer *meshr = mat->first; meshr; meshr = meshr->next) {
					b32 mesh_loaded = ask_for_asset(assets, meshr->mesh);
					b32 valid = (mat_loaded && mesh_loaded);
					meshr->loaded = valid;
					meshr->entry_count = 0;
					if(valid) {
						render->batch_count++;
					}
				}
			}
		}
	}
}

KH_INLINE u32
render_get_offset(RenderManager *render, void *val) {
	kh_assert((u8 *)val < (render->commands + render->commands_size));
	u32 res = (u32)((u8 *)val - render->commands);
	kh_assert(res < render->commands_at);
	return(res);
}

// TODO(flo): reenable this if a Shading_ can have multiple vertex format
// KH_INTERN u32
// add_vertex_format(RenderManager *render, VertexFormat format) {
// 	u32 res = INVALID_U32_OFFSET;
// 	if(render->vertex_format_offset_hash[format] == -1) {
// 		kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
// 		VertexBuffer *buffer = (VertexBuffer *)(render->commands + render->commands_at);
// 		buffer->first = 0; 
// 		buffer->format = format;
// 		buffer->next = render->first_vertex_buffer;
// 		render->first_vertex_buffer = buffer;
// 		res = render->commands_at;
// 		render->vertex_format_offset_hash[format] = res;
// 		render->commands_at += sizeof(VertexBuffer);
// 	} else {
// 		res = render->vertex_format_offset_hash[format];
// 	}
// 	return(res);
// }

KH_INTERN Shading *
get_shading(RenderManager *render, ShadingType type, VertexFormat format, MaterialType mat_type, u32 *out_offset) {
	u32 hash_key = (u32)format * Shading_count + type;
	u32 hash_slot = hash_key & (array_count(render->mat_used) - 1);

	MaterialHashEl **first = render->mat_used + hash_slot;
	MaterialHashEl *find = 0;

	MaterialHashEl *search = *first;
	while(search) {
		if(search->key == hash_key) {
			find = search;
			break;
		}
		if(!search->one_past_next_in_hash) break; 
		search = render->mat_buffer + (search->one_past_next_in_hash - 1);
		kh_assert(search);
	}
	// NOTE(flo): if not find handle error here, it means that we do not instantiate the correct combination
	// of shading type and vertex format (see add shading)
	kh_assert(find);
	Shading *res;
	// NOTE(flo): we assume here that we cannot be the first command in the buffer which should be the case
	if(!find->cmd_offset) {
		VertexBuffer *buffer = 0;
		if(render->vertex_format_offset_hash[format] == -1) {
			kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
			buffer = (VertexBuffer *)(render->commands + render->commands_at);
			buffer->first = 0;
			buffer->format = format;
			buffer->next = render->first_vertex_buffer;
			render->first_vertex_buffer = buffer;
			render->vertex_format_offset_hash[format] = render->commands_at;
			render->commands_at += sizeof(VertexBuffer);
		} else {
			i32 vert_buffer_off = render->vertex_format_offset_hash[format];
			buffer = (VertexBuffer *)(render->commands + vert_buffer_off);
		}
		kh_assert(buffer);
		kh_assert(render->commands_at + sizeof(Shading) <= render->commands_size);
		res = (Shading *)(render->commands + render->commands_at);
		res->first = 0;
		res->format = format;
		res->type = type;
		res->mat_type = mat_type;

		MaterialInfo *info = render->mat_infos + mat_type;
		res->size = info->size;
		res->texture_count = info->texture_count;

		res->shader_index = find->shader_index;
		res->next = buffer->first;
		buffer->first = res;

		find->cmd_offset = render->commands_at;
		kh_assert(find->cmd_offset != 0);

		*out_offset = find->cmd_offset;

		render->commands_at += sizeof(Shading);
 	} else {
 		res = (Shading *)(render->commands + find->cmd_offset);
 		*out_offset = find->cmd_offset;
 	}
	return(res);
}

KH_INTERN u8 * 
add_mat_instance_(RenderManager *render, ShadingType shade_type, VertexFormat format, u32 mat_size, MaterialType mat_type) {
	u32 shading_offset;
	Shading *shading = get_shading(render, shade_type, format, mat_type, &shading_offset);
	kh_assert(mat_type == render->shading_mat_type[shade_type]);

	kh_assert(render->commands_at + sizeof(Material) <= render->commands_size);
	if(mat_type == Type_Material_none) mat_size = 0;
	kh_assert(shading->size + (shading->texture_count * sizeof(AssetID)) == mat_size);

	Material *mat = (Material *)(render->commands + render->commands_at);
	mat->shading_off = shading_offset;
	mat->next = shading->first;
	shading->first = mat;
	u32 mat_off = render->commands_at;

	render->commands_at += sizeof(Material);
	u8 *res = render->commands + render->commands_at;
	kh_assert(shading->mat_type == mat_type);
	render->commands_at += mat_size;
	return(res);

}
#define add_mat_instance(render, shade_type, format, type) (type *)add_mat_instance_(render, shade_type, format, sizeof(type), Type_##type) 

KH_INLINE void
copy_mat_instance_(u8 *from, u8 *to, u32 size) {
	kh_lu0(i, size) {
		to[i] = from[i];
	}
}
#define copy_mat_instance(from, to, type) copy_mat_instance_(from, to, sizeof(type))

KH_INTERN u32
add_mesh_renderer(RenderManager *render, void *instance, AssetID mesh) {
	u32 res = INVALID_U32_OFFSET;
	u32 mat_offset = render_get_offset(render, (u8 *)instance - sizeof(Material));
	kh_assert(render->commands_at + sizeof(MeshRenderer) <= render->commands_size);
	Material *mat = (Material *)(render->commands + mat_offset);
	MeshRenderer *meshr = (MeshRenderer *)(render->commands + render->commands_at);
	meshr->first_entry = 0;
	meshr->entry_count = 0;
	meshr->mat_off = mat_offset;
	meshr->mesh = mesh;
	meshr->loaded = false;

	meshr->next = mat->first;
	mat->first = meshr;

	res = render->commands_at;
	render->commands_at += sizeof(MeshRenderer);
	return(res);
}

KH_INTERN void
push_render_entry(RenderManager *render, mat4 tr, u32 meshr, u32 animator = INVALID_U32_OFFSET) {
	kh_assert(meshr + sizeof(MeshRenderer) < render->commands_size);
	MeshRenderer *mesh_renderer = (MeshRenderer *)(render->commands + meshr); 
	u32 bone_offset = INVALID_U32_OFFSET;
	b32 animator_valid = true;
	if(animator != INVALID_U32_OFFSET) {
		kh_assert(animator < render->animators.count);
		Animator *anim = render->animators.data + animator;
		bone_offset = anim->transformation_offset;
		animator_valid = (bone_offset != INVALID_U32_OFFSET);
	}

	if(mesh_renderer->loaded && animator_valid) {
		kh_assert(render->entry_count < render->max_entries);
		u32 entry_id = render->entry_count++;
		RenderEntry *entry = render->render_entries + entry_id;
		entry->tr = tr;
		entry->next_in_mesh_renderer = mesh_renderer->first_entry;
		entry->bone_transform_offset = bone_offset;
		mesh_renderer->first_entry = entry_id;
		mesh_renderer->entry_count++;
	}
}

KH_INTERN void
push_render_entry(RenderManager *render, Entity *entity) {
	push_render_entry(render, entity->transform, entity->mesh_renderer, entity->animator);
}