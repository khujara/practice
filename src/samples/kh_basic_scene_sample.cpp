#include "kh_sample_shared.h"

struct ProgramSample {
	ProgramState state;
	Scene scene;
	LevelEditor level_editor;
};

Platform g_platform;
extern "C" kh_update *
frame_update(ProgramMemory *memory, Input *input, RenderManager *render, Assets *assets, f32 dt) {
	// khjr_assert(!"learn the math for perspective projection");
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
		mat4 proj = perspective_fov_lh(60.0f, w/h, DEFAULT_CAMERA_NEAR, DEFAULT_CAMERA_FAR);
		render->camera = perspective_camera(w, h, kh_vec3(0.0f, 5.0f, -4.0f), kh_vec3(0,0,0), COLOR_RED);
		f32 angle_x = 20.0f * TO_RADIANS * 0.5f;
		quat qx = { kh_sin_f32(angle_x), 0.0f, 0.0f, kh_cos_f32(angle_x)};

		f32 angle_y = 0.0f;//PI32 * 0.5f;
		quat qy = { 0.0f, kh_sin_f32(angle_y), 0.0f, kh_cos_f32(angle_y) };

		Camera *cam = &render->camera;
		cam->target = kh_vec3(0, 0, 0);
		cam->dist = 10.0f;
		cam->tr.rot = (qy * qx);
		cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
		set_view_matrix(cam);

		AssetID right = get_first_asset(assets, AssetName_skybox_right);
		AssetID left = get_first_asset(assets, AssetName_skybox_left);
		AssetID bottom = get_first_asset(assets, AssetName_skybox_bottom);
		AssetID top = get_first_asset(assets, AssetName_skybox_top);
		AssetID back = get_first_asset(assets, AssetName_skybox_back);
		AssetID front = get_first_asset(assets, AssetName_skybox_front);

		define_skybox(render, assets, right, left, bottom, top, back, front);
		v3 light_dir = kh_normalize_v3(kh_vec3(-0.9f, -0.6f, -0.70710678118f));
		Light light;
		light.type = LightType_directional;
		// light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.cutoff = 0;
		light.outer_cutoff = 0;
		light.pos = {};
		light.dir = -light_dir;
		light.scale = 15.0f;
		light.z_near = 0.0f;
		light.z_far = 40.0f;
		add_light(render, light);

		Material_T2F2 *wall_mat = add_mat_instance(render, Shading_normalmap, VertexFormat_PNUTB, Material_T2F2);
		wall_mat->diffuse = get_first_asset(assets, AssetName_wall);
		wall_mat->normal = get_first_asset(assets, AssetName_wall_normal);
		wall_mat->spec_power = 128.0f;
		wall_mat->spec_intensity = 1.0f;
		u32 normal_terrain = add_mesh_renderer(render, wall_mat, get_first_asset(assets, AssetName_terrain_test));
		u32 normal_randy_wall = add_mesh_renderer(render, wall_mat, get_first_asset(assets, AssetName_randy_mesh_tangent));

		Material_T2F2 *randy_mat = add_mat_instance(render, Shading_normalmap, VertexFormat_PNUTB, Material_T2F2);
		randy_mat->diffuse = get_first_asset(assets, AssetName_randy_albedo);
		randy_mat->normal = get_first_asset(assets, AssetName_randy_normal);
		randy_mat->spec_power = 32.0f;
		randy_mat->spec_intensity = 0.5f;
		u32 normal_randy = add_mesh_renderer(render, randy_mat, get_first_asset(assets, AssetName_randy_mesh_tangent));

		Material_T5 *randy_pbr_mat = add_mat_instance(render, Shading_pbr, VertexFormat_PNUTBS, Material_T5);
		randy_pbr_mat->albedo = get_first_asset(assets, AssetName_randy_albedo);
		randy_pbr_mat->normal = get_first_asset(assets, AssetName_randy_normal);
		randy_pbr_mat->roughness = get_first_asset(assets, AssetName_randy_gloss);
		randy_pbr_mat->metalness = get_first_asset(assets, AssetName_randy_metalness);
		randy_pbr_mat->ao = get_first_asset(assets, AssetName_randy_ao);
		u32 pbr_skinned_randy = add_mesh_renderer(render, randy_pbr_mat, get_first_asset(assets, AssetName_randy_mesh_skinned));

		Material_T1F3 *randy_phong = add_mat_instance(render, Shading_phong, VertexFormat_PNU, Material_T1F3);
		randy_phong->diffuse = get_first_asset(assets, AssetName_randy_albedo);
		randy_phong->spec_power = 128.0f;
		randy_phong->spec_intensity = 1.0f;
		randy_phong->reflect_intensity = 0.1f;
		u32 phong_randy = add_mesh_renderer(render, randy_phong, get_first_asset(assets, AssetName_randy_mesh));

		Material_T1F3 *bricks_mat = add_mat_instance(render, Shading_phong, VertexFormat_PNU, Material_T1F3);
		bricks_mat->diffuse = get_first_asset(assets, AssetName_bricks);
		bricks_mat->spec_power = 128.0f;
		bricks_mat->spec_intensity = 1.0f;
		bricks_mat->reflect_intensity = 1.0f;
		u32 phong_cube = add_mesh_renderer(render, bricks_mat, get_first_asset(assets, AssetName_cube));

		Material_T1F3 *lulu_mat = add_mat_instance(render, Shading_phong, VertexFormat_PNUS, Material_T1F3);
		lulu_mat->diffuse = get_first_asset(assets, AssetName_lulu_color_chart);
		lulu_mat->spec_power = 32.0f;
		lulu_mat->spec_intensity = 0.3f;
		lulu_mat->reflect_intensity = 0.1f;
		u32 phong_lulu = add_mesh_renderer(render, lulu_mat, get_first_asset(assets, AssetName_lulu));

		mat4 vp = kh_identity_mat4();
		v4 mx = kh_vec4(1, 0, 0, 0);
		v4 my = kh_vec4(0, 1, 0, 0);
		v4 mz = kh_vec4(0, 0, 1, 0);

		AssetID skeleton = get_first_asset(assets, AssetName_randy_skeleton);
		AssetID skin = get_first_asset(assets, AssetName_randy_skin);
		AssetID anim = get_first_asset(assets, AssetName_randy_idle_skin);

		AssetID lulu_skel = get_first_asset(assets, AssetName_lulu_skeleton);
		AssetID lulu_skin = get_first_asset(assets, AssetName_lulu_skin);
		AssetID lulu_idle = get_first_asset(assets, AssetName_lulu_idle_skin);
		AssetID lulu_run  = get_first_asset(assets, AssetName_lulu_run_skin);

		sample->scene = new_scene(&program_state->arena, 16);

		u32 animator_0 = add_animator(&render->animators, skeleton, skin, anim);
		u32 animator_1 = add_animator(&render->animators, lulu_skel, lulu_skin, {0}); 
		u32 animator_2 = add_animator(&render->animators, skeleton, skin, anim);
		u32 animator_3 = add_animator(&render->animators, skeleton, skin, anim);
		u32 animator_4 = add_animator(&render->animators, lulu_skel, lulu_skin, lulu_idle);
		u32 animator_5 = add_animator(&render->animators, lulu_skel, lulu_skin, lulu_run);
		u32 animator_6 = add_animator(&render->animators, skeleton, skin, anim);

		add_entity(&sample->scene, pbr_skinned_randy, kh_mat4(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-5,0,0,1)), animator_2);
		add_entity(&sample->scene, pbr_skinned_randy, kh_mat4(mx, my, mz, kh_vec4(-5,0,-2,1)), animator_0);
		add_entity(&sample->scene, pbr_skinned_randy, kh_mat4(mx, my, mz, kh_vec4(-5, 0, -5, 1)), animator_6);
		add_entity(&sample->scene, pbr_skinned_randy, kh_mat4(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-5,0,-3,1)), animator_3);

		add_entity(&sample->scene, phong_lulu, kh_mat4(mx, my, mz, kh_vec4(5,0,0,1)), animator_4);
		add_entity(&sample->scene, phong_lulu, kh_mat4(mx, my, mz, kh_vec4(5,0,-2,1)), animator_5);
		add_entity(&sample->scene, phong_lulu, kh_mat4(mx, my, mz, kh_vec4(5,3,-2,1)), animator_1);
		add_entity(&sample->scene, phong_cube, kh_mat4(mx, my, mz, kh_vec4(5,3,0,1)));
		add_entity(&sample->scene, phong_randy, kh_mat4(mx, my, mz, kh_vec4(5,0,-0.3f,1)));

		add_entity(&sample->scene, normal_randy, kh_mat4(mx, my, mz, kh_vec4(0,0,0.2f,1)));
		add_entity(&sample->scene, normal_randy, kh_mat4(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(2,0,0,1)));
		add_entity(&sample->scene, normal_randy, kh_mat4(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(0,0,2,1)));
		add_entity(&sample->scene, normal_randy, kh_mat4(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(3,0,3,1)));
		add_entity(&sample->scene, normal_randy, kh_mat4(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(-3,0,3,1)));
		add_entity(&sample->scene, normal_terrain, kh_mat4(mx, my, mz, kh_vec4(0,-1.5f,0,1)));
		add_entity(&sample->scene, normal_randy_wall, kh_mat4(mx, my, mz, kh_vec4(0,0, -3,1))); 

		set_playback_rate(&render->animators, animator_0, 3.0f);
		set_playback_rate(&render->animators, animator_2, 0.5f);
		set_playback_rate(&render->animators, animator_3, 0.5f);
		set_playback_rate(&render->animators, animator_6, 0.5f);

		sample->level_editor = get_level_editor(memory->debug_state, render, assets, &sample->scene);
	}

	FrameState *frame_state = memory->frame_state;
	if(!frame_state) {
		frame_state = memory->frame_state = boot_strap_push_struct(FrameState, arena);
	}

	DebugState *debug = memory->debug_state;
	kh_assert(debug);
	debug->check_entity_on_click = true;

	TransientLinear f_arena = kh_begin_transient(&frame_state->arena);
	begin_render_frame(render, assets);
	debug_begin_frame(debug, render->width, render->height, render->wnd_w, render->wnd_h, dt);

	default_camera_move(&render->camera, input, dt);

	for(u32 i = 0; i < render->animators.count; ++i) {
		Animator *animator = render->animators.data + i;
		global_pose_generation(animator, &render->joint_tr, assets, dt);
		matrix_palette_generation(animator, &render->joint_tr, assets);
	}

	push_scene_to_render(&sample->scene, render);

	edit_level(&sample->level_editor);
	debug_end_frame(debug);
	kh_end_transient(&f_arena);

	return(0);
}
