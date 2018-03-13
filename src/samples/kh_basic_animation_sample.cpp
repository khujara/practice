#include "kh_sample_shared.h"

enum MeshRendererType {
	MeshRenderer_randy,
	MeshRenderer_count,	
};

struct ProgramSample {
	ProgramState state;

	Entity randy;
	Entity randy_2;

	Entity lulu;
	Entity lulu2;
};

Platform g_platform;
extern "C" kh_update *
frame_update(ProgramMemory *memory, Input *input, RenderManager *render, Assets *assets, f32 dt) {
	g_platform = memory->platform;
	ProgramState *program_state = memory->program_state;
	ProgramSample *sample = (ProgramSample *)program_state;
	if(!program_state) {
		program_state = memory->program_state = boot_strap_push_struct(ProgramState, arena);
		kh_push(&program_state->arena, sizeof(ProgramSample) - sizeof(ProgramState));
		sample = (ProgramSample *)program_state;

		init_render(render);

		f32 w = (f32)render->width;
		f32 h = (f32)render->height;
		render->camera = perspective_camera(w, h, kh_vec3(0.0f,5.0f,4.0f), kh_vec3(0,1.0f,0), COLOR_RED);

		Camera *cam = &render->camera;
		lookat(cam, kh_vec3(-1.5f, 1.5f, -2.5f), kh_vec3(0,1,0), kh_vec3(0,1,0));
		
		v3 light_dir = kh_normalize_v3(kh_vec3(0.577350259f, 0.577350259f, 0.577350259f));
		Light light;
		light.type = LightType_directional;
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.cutoff = 0;
		light.outer_cutoff = 0;
		light.pos = {};
		light.dir = -light_dir;
		light.scale = 15.0f;
		light.z_near = 0.0f;
		light.z_far = 40.0f;
		add_light(render, light);


		Material_T2F2 *randy_mat = add_mat_instance(render, Shading_normalmap, VertexFormat_PNUTBS, Material_T2F2);
		randy_mat->diffuse = get_first_asset(assets, AssetName_randy_albedo);
		randy_mat->normal = get_first_asset(assets, AssetName_randy_normal);
		randy_mat->spec_power = 32.0f;
		randy_mat->spec_intensity = 0.3f;
		u32 randy = add_mesh_renderer(render, randy_mat, get_first_asset(assets, AssetName_randy_mesh_skinned));

		Material_T1F3 *lulu_mat = add_mat_instance(render, Shading_phong, VertexFormat_PNUS, Material_T1F3);
		lulu_mat->diffuse = get_first_asset(assets, AssetName_lulu_color_chart);
		lulu_mat->spec_power = 32.0f;
		lulu_mat->spec_intensity = 0.3f;
		lulu_mat->reflect_intensity = 0.0f;
		u32 lulu = add_mesh_renderer(render, lulu_mat, get_first_asset(assets, AssetName_lulu));

		AssetID skeleton = get_first_asset(assets, AssetName_randy_skeleton);
		AssetID skin = get_first_asset(assets, AssetName_randy_skin);
		AssetID anim = get_first_asset(assets, AssetName_randy_idle_skin);

		sample->randy = new_entity(randy, kh_identity_mat4(), add_animator(&render->animators, skeleton, skin, anim));
		kh_set_translation_mat4(&sample->randy.transform, kh_vec3(-0.5f,0,0));

		sample->randy_2 = new_entity(randy, kh_identity_mat4(), add_animator(&render->animators, skeleton, skin, anim));
		kh_set_translation_mat4(&sample->randy_2.transform, kh_vec3(0.5f,0,0));

		set_playback_rate(&render->animators, sample->randy.animator, 0.5f);

		AssetID lulu_skeleton = get_first_asset(assets, AssetName_lulu_skeleton);
		AssetID lulu_skin = get_first_asset(assets, AssetName_lulu_skin);
		AssetID lulu_anim = get_first_asset(assets, AssetName_lulu_run_skin);

		sample->lulu = new_entity(lulu, kh_identity_mat4(), add_animator(&render->animators, lulu_skeleton, lulu_skin, lulu_anim));
		kh_set_translation_mat4(&sample->lulu.transform, kh_vec3(-1.5f,0,0));

		AssetID lulu_idle = get_first_asset(assets, AssetName_lulu_idle_skin);
		sample->lulu2 = new_entity(lulu, kh_identity_mat4(),add_animator(&render->animators, lulu_skeleton, lulu_skin, lulu_idle));
		kh_set_translation_mat4(&sample->lulu2.transform, kh_vec3(1.5f,0,0));
	}

	FrameState *frame_state = memory->frame_state;
	if(!frame_state) {
		frame_state = memory->frame_state = boot_strap_push_struct(FrameState, arena);
	}

	DebugState *debug = memory->debug_state;
	kh_assert(debug);

	TransientLinear f_arena = kh_begin_transient(&frame_state->arena);
	begin_render_frame(render, assets);
	debug_begin_frame(debug, render->width, render->height, render->wnd_w, render->wnd_h, dt);
	debug_line(debug, kh_vec3(0,0,0), kh_vec3(1,0,0), kh_vec3(1,0,0));
	debug_line(debug, kh_vec3(0,0,0), kh_vec3(0,1,0), kh_vec3(0,1,0));
	debug_line(debug, kh_vec3(0,0,0), kh_vec3(0,0,1), kh_vec3(0,0,1));

	default_camera_move(&render->camera, input, dt);

	// TODO(flo): what should we do with this ?
	for(u32 i = 0; i < render->animators.count; ++i) {
		Animator *animator = render->animators.data + i;
		global_pose_generation(animator, &render->joint_tr, assets, dt);
	}

	// char *test_name = "B_hand_L";
	Animator *lulu_anim = render->animators.data + sample->lulu.animator; 
	if(ask_for_asset(assets, lulu_anim->skeleton)) {
		Joint *joints = (Joint *)get_datas(assets, lulu_anim->skeleton, AssetType_skeleton);
		mat4 *global_transormations = render->joint_tr.data + lulu_anim->transformation_offset;
		// TODO(flo): we need a hash in the animator to get back the skeleton joint by name
		// TODO(flo): there are billion ways to optimize this but this for debugging now
		for(u32 i = 0; i < lulu_anim->joint_count; ++i) {
			Joint *joint = joints + i;
			mat4 global_transorm = global_transormations[i] * sample->lulu.transform;
			v3 pos = kh_vec3(global_transorm.c3);
			if(joint->parent_id != -1) {
				Joint *parent = joints + joint->parent_id;
				mat4 parent_global_transform = global_transormations[joint->parent_id] * sample->lulu.transform;
				v3 parent_pos = kh_vec3(parent_global_transform.c3);
				debug_line(debug, pos, parent_pos, kh_vec3(1,1,0));
			}
		}
	}

	Animator *randy_anim = render->animators.data + sample->randy.animator; 
	if(ask_for_asset(assets, randy_anim->skeleton)) {
		Joint *joints = (Joint *)get_datas(assets, randy_anim->skeleton, AssetType_skeleton);
		mat4 *global_transormations = render->joint_tr.data + randy_anim->transformation_offset;
		// TODO(flo): we need a hash in the animator to get back the skeleton joint by name
		// TODO(flo): there are billion ways to optimize this but this for debugging now
		for(u32 i = 0; i < randy_anim->joint_count; ++i) {
			Joint *joint = joints + i;
			mat4 global_transorm = global_transormations[i] * sample->randy.transform;
			v3 pos = kh_vec3(global_transorm.c3);
			if(joint->parent_id != -1) {
				Joint *parent = joints + joint->parent_id;
				mat4 parent_global_transform = global_transormations[joint->parent_id] * sample->randy.transform;
				v3 parent_pos = kh_vec3(parent_global_transform.c3);
				debug_line(debug, pos, parent_pos, kh_vec3(1,1,0));
			}
		}
	}

	for(u32 i = 0; i < render->animators.count; ++i) {
		Animator *animator = render->animators.data + i;
		matrix_palette_generation(animator, &render->joint_tr, assets);
	}

	push_render_entry(render, &sample->randy);
	push_render_entry(render, &sample->randy_2);
	push_render_entry(render, &sample->lulu);
	push_render_entry(render, &sample->lulu2);

	kh_end_transient(&f_arena);

	return(0);

}