#define MAX_RENDER_ENTRIES 64
#define FRAME_BUFFER_BYTES_PER_PIXEL 4
#define MAX_DATA_HANDLES_IN_HASH 512
// TODO(flo): support multiple lights and different light types (point, spot)
#define MAX_LIGHTS 1

// TODO(flo): component based?
// TODO(flo): this has nothing to do here (in the render.h)
struct Entity {
	u32 animator;
	u32 mesh_renderer;

	// TODO(flo): we should replace this by Transform_SQT
	mat4 transform;
};

struct Animator {
	AssetID skeleton;
	AssetID skin;
	AssetID animation_clip;

	f64 cur_time;
	f64 utime;
	f32 playback_rate;
	b32 loop;

	u32 transformation_offset;

	b32 DEBUG_DELETE_LERP = false;
	b32 DEBUG_USE_NLERP = false;
};

struct DirectionalLight {
	v3 color;
	f32 ambient_intensity;
	v3 dir;
	f32 diffuse_intensity;
};

struct Skybox {
	AssetID right;
	AssetID left;
	AssetID bottom;
	AssetID top;
	AssetID back;
	AssetID front;
};

struct RenderEntry {
	u32 next_in_mesh_renderer;

	u32 bone_transform_offset;

	// TODO(flo): Transform_SQT
	mat4 tr;
};

struct MeshRenderer {
	AssetID mesh;
	u32 entry_count;
	u32 first_entry;
	b32 valid;
	MeshRenderer *next;
};

struct MaterialInstance {
	AssetID diffuse;
	AssetID normal;
	v4 color;
	MaterialInstance *next;
	MeshRenderer *first;
};

struct Material {
	MaterialType type;
	Material *next;
	MaterialInstance *first;
};

struct VertexBuffer {
	VertexFormat format;
	VertexBuffer *next;
	Material *first;
};

struct BoneTransform {
	u32 count;
	u32 max_count;
	mat4 *data;
};

struct AnimatorArray {
	u32 count;
	u32 max_count;
	Animator *data;
};

struct RenderManager {

	Camera camera;
	u32 width, height;

	// -----------------------------------------------
	// NOTE(flo): we need to clear this each frame
	u32 entry_count;
	u32 batch_count;
	// --------------------------------------------------------

	VertexBuffer *first_vertex_buffer;
	
	u32 commands_at;
	u32 commands_size;
	u8 *commands;
	RenderEntry render_entries[MAX_RENDER_ENTRIES]; 

	// TODO(flo): it seems we only need bone_transformations here
	// TODO(flo): Pool allocator (free list)
	AnimatorArray animators;

	// TODO(flo): Pool allocator (free list)
	BoneTransform bone_tr;

	b32 has_skybox;
	Skybox skybox;
	u32 light_count;
	DirectionalLight lights[MAX_LIGHTS];
};

// NOTE(flo): for now we assume that the curve between sample is linear
// TODO(flo): support non linear curves
KH_INLINE void 
animator_advance(Animator *animator, f64 duration, f64 dt) {
	f64 old_time = animator->cur_time;
	animator->cur_time += dt * animator->playback_rate;

	if((animator->cur_time != old_time) && (animator->cur_time >= duration)) {
		if(!animator->loop) {	
			animator->cur_time = duration;
		} else {
			animator->cur_time -= duration;
		}
	}
	animator->utime = animator->cur_time / duration; 
}

KH_INTERN u32
add_animator(AnimatorArray *animators, BoneTransform *bone_tr, Assets *assets, AssetID skeleton, AssetID skin, AssetID clip) {
	u32 res = animators->count++;
	Animator *animator = animators->data + res;
	animator->animation_clip = clip;
	animator->skeleton = skeleton;
	animator->skin = skin;
	animator->utime = 0.0;
	animator->cur_time = 0.0;
	animator->loop = true;
	animator->playback_rate = 1.0f;
	return(res);
}

KH_INTERN void
set_playback_rate(AnimatorArray *animators, u32 index, f32 playback_rate) {
	kh_assert(index < animators->count);
	Animator *animator = animators->data + index;
	animator->playback_rate = playback_rate;
}

// TODO(flo): implement this and then blending between AnimationID
KH_INTERN void
change_clip(Animator *animator, Assets *assets, AssetID clip) {
	NOT_IMPLEMENTED;
}

// TODO(flo): use IDs instead of pointers and count values here!
KH_INTERN void
update_animator(Animator *animator, BoneTransform *bone_tr,  Assets *assets, f64 dt) {

	// TODO(flo): if animator clip == 0 

	animator->transformation_offset = INVALID_U32_OFFSET;

	b32 skeleton_loaded = asset_loaded(assets, animator->skeleton);
	b32 animation_loaded = asset_loaded(assets, animator->animation_clip);
	b32 skin_loaded = asset_loaded(assets, animator->skin);

	if(skeleton_loaded && skin_loaded) {

		Skeleton skeleton = get_skeleton(assets, animator->skeleton);
		u32 joint_count = skeleton.joint_count;

		if(bone_tr->count + joint_count < bone_tr->max_count) {

			animator->transformation_offset = bone_tr->count;
			bone_tr->count += joint_count;

			Joint *joints = (Joint *)get_datas(assets, animator->skeleton, AssetType_skeleton);
			mat4 *inverse_bind_poses = (mat4 *)get_datas(assets, animator->skin, AssetType_meshskin);
			mat4 *final_transformations = bone_tr->data + animator->transformation_offset;

			if(animation_loaded) {

				LoadedAsset animation = get_loaded_asset(assets, animator->animation_clip, AssetType_animation);
				AnimationClip clip = animation.type->animation;

				Transform_SQT *samples = (Transform_SQT *)animation.data;

				// animator->playback_rate = 1.0f;

				f64 duration = clip.duration;
				u32 sample_count = clip.sample_count;

				animator_advance(animator, duration, dt);

				i32 index = (i32)(animator->utime * (sample_count - 1));
				kh_clamp_i32(0, sample_count - 1, index);

				f64 inv = 1.0 / (f64)(sample_count - 1);
				f64 time_per_sample = duration * inv;
				f64 time_base       = duration * inv * index;
				f64 fraction = (animator->cur_time - time_base) / time_per_sample;
				kh_clamp01_f64(fraction);

				kh_assert((index + 1) < (i32)sample_count);

				Transform_SQT *sample0 = samples + ((index + 0) * joint_count);
				Transform_SQT *sample1 = samples + ((index + 1) * joint_count);


				for(i32 i = 0; i < (i32)joint_count; ++i) {
					Transform_SQT tr;
					if(!animator->DEBUG_DELETE_LERP) {
						Transform_SQT *sqt0 = sample0 + i;
						Transform_SQT *sqt1 = sample1 + i;
						tr.pos = kh_mix_v3(sqt0->pos, sqt1->pos, (f32)fraction);
						//NOTE(flo) : if dot product of two interpolating quaternions is < 0, it won't take the shortest arc around the sphere. that's why we negate one of the quaternions.
						if(animator->DEBUG_USE_NLERP) {
							if(dot(sqt0->rot, sqt1->rot) < 0) {
								tr.rot = kh_nlerp_quat(sqt0->rot, negate(sqt1->rot), fraction);
							} else {
								tr.rot = kh_nlerp_quat(sqt0->rot, sqt1->rot, fraction);
							}
						} else {
							if(dot(sqt0->rot, sqt1->rot) < 0) {
								tr.rot = kh_slerp_quat(sqt0->rot, negate(sqt1->rot), fraction);
							} else {
								tr.rot = kh_slerp_quat(sqt0->rot, sqt1->rot, fraction);
							}
						}
						tr.scale = kh_mix_v3(sqt0->scale, sqt1->scale, (f32)fraction);
					}
					else {
						tr = sample0[i];
					}
					mat4 local_pose = kh_get_mat4_from_sqt(tr.pos, tr.rot, tr.scale);

					Joint *joint = joints + i;

					i32 parent_index = joint->parent_id;
					if(parent_index >= 0) {
						kh_assert(parent_index < i);
						final_transformations[i] = local_pose * final_transformations[parent_index];
					} else {
						final_transformations[i] = local_pose * inverse_bind_poses[joint_count];
					}
				}
			} else {
				Transform_SQT *local_poses = (Transform_SQT *)(inverse_bind_poses + joint_count + 1);
				for(i32 i = 0; i < (i32)joint_count; ++i) {
					Transform_SQT *tr = local_poses + i;
					mat4 local_pose = kh_get_mat4_from_sqt(tr->pos, tr->rot, tr->scale);

					Joint *joint = joints + i;
					i32 parent_index = joint->parent_id;
					if(parent_index >= 0) {
						kh_assert(parent_index < i);
						final_transformations[i] = local_pose * final_transformations[parent_index];
					} else {
						final_transformations[i] = local_pose * inverse_bind_poses[joint_count];
					}

				}
			}
			for(u32 i = 0; i < joint_count; ++i) {
				final_transformations[i] = inverse_bind_poses[i] * final_transformations[i];
			}
		} else {
			kh_assert(!"break the bone tr count limitation");
		}
	}
}


KH_INTERN void
define_light(RenderManager *render, DirectionalLight light) {
	kh_assert(render->light_count < MAX_LIGHTS);
	render->lights[render->light_count++] = light;
}

// NOTE(flo): for now skybox and light should only be sent once, and not at each frame like other render commands
KH_INTERN void
define_skybox(RenderManager *render, Assets *assets, AssetID right, AssetID left, AssetID bottom, 
              AssetID top, AssetID back, AssetID front) {
	asset_load_force_immediate(assets, right);
	asset_load_force_immediate(assets, left);
	asset_load_force_immediate(assets, bottom);
	asset_load_force_immediate(assets, top);
	asset_load_force_immediate(assets, back);
	asset_load_force_immediate(assets, front);
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
	render->commands_at = 0;
	render->first_vertex_buffer = 0;
	render->animators.count = 0;
}

KH_INLINE void
begin_render_frame(RenderManager *render, Assets *assets) {
	render->bone_tr.count = 0;
	render->batch_count = 0;
	for(VertexBuffer *buffer = render->first_vertex_buffer; buffer; buffer = buffer->next) {
		for(Material *mat = buffer->first; mat; mat = mat->next) {
			for(MaterialInstance *instance = mat->first; instance; instance = instance->next) {
				b32 diffuse_loaded = asset_loaded(assets, instance->diffuse);
				b32 normal_loaded = asset_loaded(assets, instance->normal);
				b32 instance_loaded = (diffuse_loaded && normal_loaded);
				for(MeshRenderer *meshr = instance->first; meshr; meshr = meshr->next) {
					b32 mesh_loaded = asset_loaded(assets, meshr->mesh);
					b32 valid = (instance_loaded && mesh_loaded);
					meshr->valid = valid;
					meshr->entry_count = 0;
					if(valid) {
						render->batch_count++;
					}
				}
			}
		}
	}
	render->entry_count = 0;
}

KH_INTERN u32
add_vertex_format(RenderManager *render, VertexFormat format) {
	u32 res = INVALID_U32_OFFSET;
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);

	VertexBuffer *buffer = (VertexBuffer *)(render->commands + render->commands_at);
	buffer->first = 0; 
	buffer->format = format;
	buffer->next = render->first_vertex_buffer;
	render->first_vertex_buffer = buffer;

	res = render->commands_at;
	render->commands_at += sizeof(VertexBuffer);

	return(res);
}

KH_INTERN u32
add_material(RenderManager *render, u32 buffer_off, MaterialType type) {
	u32 res = INVALID_U32_OFFSET;
	kh_assert(buffer_off != INVALID_U32_OFFSET);
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
	VertexBuffer *buffer = (VertexBuffer *)(render->commands + buffer_off);
	Material *mat = (Material *)(render->commands + render->commands_at);

	mat->first = 0;
	mat->type = type;

	mat->next = buffer->first;
	buffer->first = mat;

	res = render->commands_at;
	render->commands_at += sizeof(Material);

	return(res);
}

KH_INTERN u32
add_material_instance(RenderManager *render, u32 mat_off, Assets *assets, AssetID diffuse, AssetID normal, v4 color) {
	u32 res = INVALID_U32_OFFSET;
	kh_assert(mat_off != INVALID_U32_OFFSET);
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
	asset_loaded(assets, diffuse);
	asset_loaded(assets, normal);

	Material *mat = (Material *)(render->commands + mat_off);
	MaterialInstance *instance = (MaterialInstance *)(render->commands + render->commands_at);
	instance->first = 0;
	instance->diffuse = diffuse;
	instance->normal = normal;
	instance->color = color;

	instance->next = mat->first;
	mat->first = instance;

	res = render->commands_at;
	render->commands_at += sizeof(MaterialInstance);

	return(res);
}

KH_INTERN u32
add_mesh_renderer(RenderManager *render, u32 instance_off, Assets *assets, AssetID mesh) {
	u32 res = INVALID_U32_OFFSET;
	kh_assert(instance_off != INVALID_U32_OFFSET);
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
	asset_loaded(assets, mesh);
	MaterialInstance *instance = (MaterialInstance *)(render->commands + instance_off);
	MeshRenderer *meshr = (MeshRenderer *)(render->commands + render->commands_at);
	meshr->first_entry = 0;
	meshr->entry_count = 0;
	meshr->mesh = mesh;
	meshr->valid = false;

	meshr->next = instance->first;
	instance->first = meshr;

	res = render->commands_at;
	render->commands_at += sizeof(MeshRenderer);

	return(res);
}

KH_INTERN void
push_render_entry(RenderManager *render, Entity *entity) {

	kh_assert(entity->mesh_renderer + sizeof(MeshRenderer) < render->commands_size);
	MeshRenderer *mesh_renderer = (MeshRenderer *)(render->commands + entity->mesh_renderer); 
	u32 bone_offset = INVALID_U32_OFFSET;
	b32 animator_valid = true;
	if(entity->animator != INVALID_U32_OFFSET) {
		kh_assert(entity->animator < render->animators.count);
		Animator *animator = render->animators.data + entity->animator;
		bone_offset = animator->transformation_offset;
		animator_valid = (bone_offset != INVALID_U32_OFFSET);
	}

	if(mesh_renderer->valid && animator_valid) {
		u32 entry_id = render->entry_count++;
		RenderEntry *entry = render->render_entries + entry_id;
		entry->tr = entity->transform;
		entry->next_in_mesh_renderer = mesh_renderer->first_entry;
		entry->bone_transform_offset = bone_offset;
		mesh_renderer->first_entry = entry_id;
		mesh_renderer->entry_count++;
	}
}