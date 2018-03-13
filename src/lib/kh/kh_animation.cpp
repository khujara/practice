KH_INTERN u32
add_animator(AnimatorArray *animators, AssetID skeleton, AssetID skin, AssetID clip) {
	u32 res = animators->count++;
	Animator *animator = animators->data + res;
	animator->animation_clip = clip;
	animator->skeleton = skeleton;
	animator->skin = skin;
	animator->joint_count = 0;
	animator->utime = 0.0;
	animator->cur_time = 0.0;
	animator->loop = true;
	animator->playback_rate = 1.0f;
	return(res);
}

// NOTE(flo): for now we assume that the curve between sample is linear
// TODO(flo): support non linear curves
/* TODO(flO)
	ANIMATION PIPELINE :
		1- Clip decompression (if needed) and pose extraction
		2- Pose blending	
		3- Global Pose Generation
		4- Post Processing (IK/Ragdoll/Procedural)
		5- Recalculation of global poses
		6- Inverse Bind Pose multiplcation (Matrix palette generation)

	Animation code has nothing to do here...
	we need to have a clear understanding of how we should process
	all of this (for now 1,2 and 3 are in the same function do we need to split?)

*/
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
global_pose_generation(Animator *animator, JointTransform *joint_tr,  Assets *assets, f64 dt) {

	animator->transformation_offset = INVALID_U32_OFFSET;

	b32 skeleton_loaded = ask_for_asset(assets, animator->skeleton);
	b32 skin_loaded = ask_for_asset(assets, animator->skin);

	b32 animskin_loaded = ask_for_asset(assets, animator->animation_clip);
	AssetID animation_clip = {0};
	b32 animation_loaded = false;
	if(animator->animation_clip.val != 0) {
		Asset *anim_skin_asset = get_unloaded_asset(assets, animator->animation_clip, AssetType_animationskin);
		animation_clip = {anim_skin_asset->source.type.animskin.animation_id};
		animation_loaded = ask_for_asset(assets, animation_clip) && animskin_loaded;
	}

	if(skeleton_loaded && skin_loaded) {

		Skeleton skeleton = get_skeleton(assets, animator->skeleton);
		u32 joint_count = skeleton.joint_count;

		if(joint_tr->count + joint_count < joint_tr->max_count) {

			animator->transformation_offset = joint_tr->count;
			joint_tr->count += joint_count;
			animator->joint_count = joint_count;

			Joint *joints = (Joint *)get_datas(assets, animator->skeleton, AssetType_skeleton);
			mat4 *inverse_bind_poses = (mat4 *)get_datas(assets, animator->skin, AssetType_meshskin);
			mat4 *global_transormations = joint_tr->data + animator->transformation_offset;

			// animation_loaded = false;
			if(animation_loaded) {

				kh_assert(animation_clip.val != 0);

				LoadedAsset anim_skin = get_loaded_asset(assets, animator->animation_clip, AssetType_animationskin);
				LoadedAsset animation = get_loaded_asset(assets, animation_clip, AssetType_animation);
				AnimationClip clip = animation.type->animation;

				i32 *index_map = (i32 *)anim_skin.data;

				Transform_SQT *samples = (Transform_SQT *)animation.data;

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


				// index = 0;
				kh_assert((index + 1) < (i32)sample_count);

				// animator->DEBUG_DELETE_LERP = true;

				u32 anim_joint_count = clip.joint_count;
				Transform_SQT *sample0 = samples + ((index + 0) * anim_joint_count);
				Transform_SQT *sample1 = samples + ((index + 1) * anim_joint_count);

				for(i32 joint_index = 0; joint_index < (i32)joint_count; ++joint_index) {
					Joint *joint = joints + joint_index;

					Transform_SQT tr;
					i32 anim_joint_index = index_map[joint_index];
					mat4 local_pose = kh_identity_mat4(); 

					if(anim_joint_index != -1) {
						kh_assert(anim_joint_index < (i32)anim_joint_count);

						if(!animator->DEBUG_DELETE_LERP) {
							Transform_SQT *sqt0 = sample0 + anim_joint_index;
							Transform_SQT *sqt1 = sample1 + anim_joint_index;

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
							tr = sample0[anim_joint_index];
						}

						local_pose = kh_get_mat4_from_sqt(tr.pos, tr.rot, tr.scale);

					} 

					i32 parent_index = joint->parent_id;
					if(parent_index >= 0) {
						i32 anim_parent_index = index_map[parent_index];
						kh_assert(parent_index < joint_index);
						kh_assert(anim_parent_index < anim_joint_index);
						global_transormations[joint_index] = local_pose * global_transormations[parent_index];
					} else {
						global_transormations[joint_index] = local_pose * inverse_bind_poses[joint_count];
					}
				}
			} else {
				Transform_SQT *local_poses = (Transform_SQT *)(inverse_bind_poses + joint_count + 1);
				for(i32 joint_index = 0; joint_index < (i32)joint_count; ++joint_index) {
					Transform_SQT *tr = local_poses + joint_index;
					mat4 local_pose = kh_get_mat4_from_sqt(tr->pos, tr->rot, tr->scale);

					Joint *joint = joints + joint_index;
					i32 parent_index = joint->parent_id;
					if(parent_index >= 0) {
						kh_assert(parent_index < joint_index);
						global_transormations[joint_index] = local_pose * global_transormations[parent_index];
					} else {
						global_transormations[joint_index] = local_pose * inverse_bind_poses[joint_count];
					}

				}
			}

			// NOTE(flo): NOW everything is in the global pose related to his parents;
			// TODO(flo): children traversal
			// if(test_joint != -1) {
			// 	u32 num_children = 0;
			// 	for(u32 i = test_joint + 1; i < joint_count; ++i) {
			// 		Joint *joint = joints + i;
			// 		if(joint->parent_id < test_joint) {
			// 			break;
			// 		}
			// 		++num_children;
			// 	}
			// 	int debugp = 2;
			// }
			
			// if(test_joint != 1) {
			// 	global_transormations[test_joint]

			// }

		} else {
			kh_assert(!"break the bone tr count limitation");
		}
	}
}

KH_INLINE void
matrix_palette_generation(Animator *animator, JointTransform *joint_tr, Assets *assets) {
	if(animator->transformation_offset != INVALID_U32_OFFSET) {
		mat4 *inverse_bind_poses = (mat4 *)get_datas(assets, animator->skin, AssetType_meshskin);
		mat4 *global_transormations = joint_tr->data + animator->transformation_offset;
		for(u32 i = 0; i < animator->joint_count; ++i) {
			global_transormations[i] = inverse_bind_poses[i] * global_transormations[i];
		}
	}
}

KH_INTERN void
global_pose_generation(AnimatorArray *animators, JointTransform *joint_tr, Assets *assets, f64 dt) {
	for(u32 i = 0; i < animators->count; ++i) {
		Animator *animator = animators->data + i;
		global_pose_generation(animator, joint_tr, assets, dt);
	}
}

KH_INTERN void
matrix_palette_generation(AnimatorArray *animators, JointTransform *joint_tr, Assets *assets) {
	for(u32 i = 0; i < animators->count; ++i) {
		Animator *animator = animators->data + i;
		matrix_palette_generation(animator, joint_tr, assets);
	}
}