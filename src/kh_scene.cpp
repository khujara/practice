KH_INLINE MeshRenderer *
get_entity_meshr(RenderManager *render, Entity *entity) {
	MeshRenderer *res = (MeshRenderer *)(render->commands + entity->mesh_renderer);
	return(res);
}

#define get_entity_material_properties(render, entity, type) (type *)get_entity_material_properties_(render, entity);
KH_INLINE u8 *
get_entity_material_properties_(RenderManager *render, Entity *entity) {
	MeshRenderer *meshr = get_entity_meshr(render, entity);
	u8 *res = render->commands + meshr->mat_off + sizeof(Material);
	return(res);
}

KH_INLINE Material *
get_entity_material(RenderManager *render, Entity *entity) {
	MeshRenderer *meshr = get_entity_meshr(render, entity);
	Material *mat = (Material *)(render->commands + meshr->mat_off);
	return(mat);
}

KH_INLINE Shading *
get_shading_for_material(RenderManager *render, u32 mat_off) {
	Material *mat = (Material *)(render->commands + mat_off);
	Shading *res = (Shading *)(render->commands + mat->shading_off);
	return(res);
}

KH_INLINE Shading *
get_shading_for_material(RenderManager *render, Material *mat) {
	Shading *res = (Shading *)(render->commands + mat->shading_off);
	return(res);
}

KH_INLINE Shading *
get_entity_shading(RenderManager *render, Entity *entity) {
	Material *mat = get_entity_material(render, entity); 
	Shading *res = (Shading *)(render->commands + mat->shading_off);
	return(res);
}

#define get_meshr_material(render, meshr, type) (type *)get_meshr_material_(render, meshr);
KH_INLINE u8 *
get_meshr_material_(RenderManager *render, u32 meshr_off) {
	MeshRenderer *meshr = (MeshRenderer *)(render->commands + meshr_off);
	u8 *res = render->commands + meshr->mat_off + sizeof(Material);
	return(res);
}

#define get_material_properties(render, mat, shading, type) (type *)get_material_properties_(render, mat, shading, (u32)Type_##type)
KH_INLINE u8 *
get_material_properties_(RenderManager *render, Material *mat, Shading *shading, u32 type) {
	kh_assert((u32)shading->mat_type == type);
	u8 *res = (u8 *)(mat + 1);
	return(res);
}

KH_INLINE void
set_pos(SceneEntity *entity, v3 pos) {
	entity->entity.transform.c3 = kh_vec4(pos.x, pos.y, pos.z, 1.0f);
}

KH_INLINE v3
get_pos(SceneEntity *entity) {
	v3 res = kh_vec3(entity->entity.transform.c3);
	return(res);
}

KH_INLINE Entity 
new_entity(u32 mesh_renderer, mat4 transform, u32 animator = INVALID_U32_OFFSET) {
	Entity res;
	res.animator = animator;
	res.mesh_renderer = mesh_renderer;
	res.transform = transform;
	return(res);
}

KH_INTERN Scene
new_scene(LinearArena *arena, u32 max_pool_count) {
	Scene res;
	res.arena = arena;
	res.max_pool_count = max_pool_count;
	res.pool_count = 0;
	res.entity_pool = kh_push_array(arena, max_pool_count, SceneEntity);
	res.free_sentinel = {};
	res.first_free_entity = &res.free_sentinel;
	res.entity_count = 0;
	return(res);
}

KH_INLINE SceneEntity *
add_entity(Scene *scene, u32 mesh_renderer, mat4 transform, u32 animator = INVALID_U32_OFFSET) {
	SceneEntity *dst = 0;
	if(scene->first_free_entity != &scene->free_sentinel) {
		kh_assert(scene->pool_count < scene->max_pool_count);
		dst = scene->entity_pool + scene->pool_count++;
		dst->index = scene->pool_count - 1;
		dst->entity.mesh_renderer = mesh_renderer;
		dst->entity.transform = transform;
		dst->entity.animator = animator;
		dst->next_free = 0;
	} else {
		dst = scene->first_free_entity;
		dst->entity.mesh_renderer = mesh_renderer;
		dst->entity.transform = transform;
		dst->entity.animator = animator;
		scene->first_free_entity = dst->next_free;
		dst->next_free = 0;
	}
	scene->entity_count++;
	return(dst);
}

KH_INLINE SceneEntity *
add_entity(Scene *scene, Entity *entity) {
	SceneEntity *res = add_entity(scene, entity->mesh_renderer, entity->transform, entity->animator);
	return(res);
}

KH_INLINE Entity *
get_entity(Scene *scene, u32 pool_ind) {
	SceneEntity *scene_entity = scene->entity_pool + pool_ind;
	Entity *res = &scene_entity->entity;
	return(res);
}

KH_INLINE void
remove_entity(Scene *scene, SceneEntity *to_free) {
	kh_assert(to_free->next_free == 0);
	to_free->next_free = scene->first_free_entity;
	scene->first_free_entity = to_free;
	scene->entity_count--;
}

KH_INLINE void
remove_entity(Scene *scene, u32 index) {
	SceneEntity *to_free = scene->entity_pool + index;	
	remove_entity(scene, to_free);
}

KH_INLINE void
push_scene_to_render(Scene *scene, RenderManager *render) {
	kh_lu0(entity_i, scene->pool_count) {
		SceneEntity *scene_entity = scene->entity_pool + entity_i;
		// TODO(flo): this should mean that the entity is not on the free list
		if(!scene_entity->next_free) {
			Entity entity = scene_entity->entity;
			kh_assert(entity.mesh_renderer + sizeof(MeshRenderer) < render->commands_size);
			MeshRenderer *mesh_renderer = (MeshRenderer *)(render->commands + entity.mesh_renderer); 
			u32 bone_offset = INVALID_U32_OFFSET;
			b32 animator_valid = true;
			if(entity.animator != INVALID_U32_OFFSET) {
				kh_assert(entity.animator < render->animators.count);
				Animator *animator = render->animators.data + entity.animator;
				bone_offset = animator->transformation_offset;
				animator_valid = (bone_offset != INVALID_U32_OFFSET);
			}
			if(mesh_renderer->loaded && animator_valid) {
				kh_assert(render->entry_count < render->max_entries);
				u32 entry_id = render->entry_count++;
				RenderEntry *entry = render->render_entries + entry_id;
				entry->scene_index = entity_i + 1;
				entry->tr = entity.transform;
				entry->next_in_mesh_renderer = mesh_renderer->first_entry;
				entry->bone_transform_offset = bone_offset;
				mesh_renderer->first_entry = entry_id;
				mesh_renderer->entry_count++;
			}
		}
	}	
}
