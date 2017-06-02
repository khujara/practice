#include "kh_sample_shared.h"

struct ProgramSample {
	ProgramState state;
	Entity entities[16];
};

Platform g_platform;
extern "C" kh_update *
frame_update(ProgramMemory *memory, Input *input, RenderManager *render, Assets *assets, f32 dt) {
	// khjr_assert(!"learn the math for perspective projection");
	g_platform = memory->platform;
	ProgramState *p_state = memory->p_state;
	ProgramSample *sample = (ProgramSample *)p_state;

	if(!p_state) {

		StackAllocator stack = {};
		p_state = memory->p_state = (ProgramState *)kh_push_struct(&stack, ProgramSample);
		p_state->stack = stack;
		sample = (ProgramSample *)p_state;

		init_render(render);

		f32 w = (f32)render->width;
		f32 h = (f32)render->height;
		mat4 proj = perspective_fov_lh(60.0f, w/h, DEFAULT_CAMERA_NEAR, DEFAULT_CAMERA_FAR);
		render->camera = default_perspective_camera(w, h, kh_vec3(0.0f, 5.0f, -4.0f), kh_vec3(0,0,0), 
		                                                CullingMask_3D_default, COLOR_RED);
		f32 angle_x = 20.0f * TO_RADIANS * 0.5f;
		quat qx = { kh_sin_f32(angle_x), 0.0f, 0.0f, kh_cos_f32(angle_x)};

		f32 angle_y = 0.0f;//PI32 * 0.5f;
		quat qy = { 0.0f, kh_sin_f32(angle_y), 0.0f, kh_cos_f32(angle_y) };

		Camera *cam = &render->camera;
		cam->target = kh_vec3(0, 0, 0);
		cam->dist = 10.0f;

		cam->tr.rot = (qy * qx);
		cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
		mat3 cam_wld = from_quat_to_mat3(cam->tr.rot);
		cam->right = kh_right_mat3(cam_wld);

		AssetID right = get_first_asset(assets, AssetName_skybox_right);
		AssetID left = get_first_asset(assets, AssetName_skybox_left);
		AssetID bottom = get_first_asset(assets, AssetName_skybox_bottom);
		AssetID top = get_first_asset(assets, AssetName_skybox_top);
		AssetID back = get_first_asset(assets, AssetName_skybox_back);
		AssetID front = get_first_asset(assets, AssetName_skybox_front);

		define_skybox(render, assets, right, left, bottom, top, back, front);
		v3 light_dir = kh_normalize_v3(kh_vec3(0.5f, -0.6f, 0.70710678118f));
		DirectionalLight light;
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.ambient_intensity = 0.15f;
		light.dir = light_dir;
		light.diffuse_intensity = 0.8f;
		define_light(render, light);


		AssetID texture_none = {};
		u32 vert_format = add_vertex_format(render, VertexFormat_PosNormalUV);
		u32 mat = add_material(render, vert_format, Material_phong);
		u32 instance = add_material_instance(render, mat, assets, get_first_asset(assets, AssetName_randy_albedo), texture_none, kh_vec4(1,1,1,1));
		u32 phong_randy = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_randy_mesh));
		u32 phong_ico = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_icosphere));
		instance = add_material_instance(render, mat, assets, get_first_asset(assets, AssetName_bricks), texture_none, kh_vec4(1,1,1,1));
		u32 phong_cube = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_cube));

		vert_format = add_vertex_format(render, VertexFormat_PosNormalTangentBitangentUV);
		mat = add_material(render, vert_format, Material_normalmap);
		instance = add_material_instance(render, mat, assets, get_first_asset(assets, AssetName_wall), get_first_asset(assets, AssetName_wall_normal), kh_vec4(1,1,1,1));
		u32 normal_terrain = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_terrain_test));
		instance = add_material_instance(render, mat, assets, get_first_asset(assets, AssetName_randy_albedo), get_first_asset(assets, AssetName_randy_normal), kh_vec4(1,1,1,1));
		u32 normal_randy = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_randy_mesh_tangent));

		vert_format = add_vertex_format(render, VertexFormat_PosNormalTangentBitangentUVSkinned);
		mat = add_material(render, vert_format, Material_normalmapskinned);
		instance = add_material_instance(render, mat, assets, get_first_asset(assets, AssetName_randy_albedo), get_first_asset(assets, AssetName_randy_normal), kh_vec4(1,1,1,1));
		u32 skinned_randy = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_randy_mesh_skinned));

		for(u32 i = 0; i < array_count(sample->entities); ++i) {
			sample->entities[i].animator = INVALID_U32_OFFSET;
		}
		mat4 vp = kh_identity_mat4();
		v4 mx = kh_vec4(1, 0, 0, 0);
		v4 my = kh_vec4(0, 1, 0, 0);
		v4 mz = kh_vec4(0, 0, 1, 0);

		sample->entities[0].mesh_renderer = normal_randy;
		sample->entities[0].transform = m44(mx, my, mz, kh_vec4(0,0,0.2f,1));
		sample->entities[1].mesh_renderer = normal_randy;
		sample->entities[1].transform = m44(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(2,0,0,1));
		sample->entities[2].mesh_renderer = normal_randy;
		sample->entities[2].transform = m44(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(0,0,2,1));
		sample->entities[3].mesh_renderer = skinned_randy;
		sample->entities[3].transform = m44(mx, my, mz, kh_vec4(0,0,-2,1)); 
		sample->entities[4].mesh_renderer = phong_ico;
		sample->entities[4].transform = m44(mx, my, mz, kh_vec4(2,3,-2,1)); 
		sample->entities[5].mesh_renderer = skinned_randy;
		sample->entities[5].transform = m44(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-2,0,0,1)); 
		sample->entities[6].mesh_renderer = normal_randy;
		sample->entities[6].transform = m44(mx, my, mz, kh_vec4(3,0, -3,1)); 
		sample->entities[7].mesh_renderer = normal_randy;
		sample->entities[7].transform = m44(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(3,0,3,1));  
		sample->entities[8].mesh_renderer = normal_randy;
		sample->entities[8].transform = m44(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(-3,0,3,1));  
		sample->entities[9].mesh_renderer = skinned_randy;
		sample->entities[9].transform = m44(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-2,0,-3,1));  
		sample->entities[10].mesh_renderer = phong_cube;
		sample->entities[10].transform = m44(mx, my, mz, kh_vec4(0,3,0,1)); 
		sample->entities[11].mesh_renderer = phong_ico;
		sample->entities[11].transform = m44(mx, my, mz, kh_vec4(4,0,0,1)); 
		sample->entities[12].mesh_renderer = phong_ico;
		sample->entities[12].transform = m44(mx, my, mz, kh_vec4(-4,0,0,1)); 
		sample->entities[13].mesh_renderer = normal_terrain;
		sample->entities[13].transform = m44(mx, my, mz, kh_vec4(0,-1.5f,0,1)); 
		sample->entities[14].mesh_renderer = phong_randy;
		sample->entities[14].transform = m44(mx, my, mz, kh_vec4(0, 0, -5, 1));	 
		sample->entities[15].mesh_renderer = normal_randy;
		sample->entities[15].transform = m44(mx, my, mz, kh_vec4(0,0,-0.3f,1)); 

		AssetID skeleton = get_first_asset(assets, AssetName_randy_skeleton);
		AssetID skin = get_first_asset(assets, AssetName_randy_skin);
		AssetID anim = get_first_asset(assets, AssetName_randy_idle);

		sample->entities[3].animator = add_animator(&render->animators, &render->bone_tr, assets, skeleton, skin, anim);
		sample->entities[5].animator = add_animator(&render->animators, &render->bone_tr, assets, skeleton, skin, anim);
		sample->entities[9].animator = add_animator(&render->animators, &render->bone_tr, assets, skeleton, skin, anim);
		set_playback_rate(&render->animators, sample->entities[3].animator, 0.5f);
		set_playback_rate(&render->animators, sample->entities[5].animator, 0.5f);
		set_playback_rate(&render->animators, sample->entities[9].animator, 0.5f);

	}

	FrameState *f_state = memory->f_state;
	if(!memory->f_state) {
		StackAllocator stack = {};
		f_state = memory->f_state = kh_push_struct(&stack, FrameState);
		f_state->stack = stack;
		f_state->asset_arr = assets;//alloc_assets_and_load_infos(&memory->assets, memory->high_queue, megabytes(64));
	}

	TransientStack f_stack = kh_begin_transient(&f_state->stack);
	begin_render_frame(render, assets);

	v2 dt_mouse = kh_vec2(input->delta_mouse_x, input->delta_mouse_y);
	b32 click_mid = input->mouse_buttons[MouseButton_middle].down;
	f32 mouse_wheel = (f32)input->dt_wheel;

	Camera *cam = &render->camera;
	#if 1
	cam->dist -= mouse_wheel * 0.1f * dt;

	if(click_mid) {

		f32 angle_x = dt_mouse.x * dt * 0.125f;
		f32 angle_y = -dt_mouse.y * dt * 0.125f;

		float cx = kh_cos_f32(angle_x);
		float cy = kh_cos_f32(angle_y);
		float sx = kh_sin_f32(angle_x);
		float sy = kh_sin_f32(angle_y);

		quat qx = {0.0f, sx, 0.0f, cx };
		v3 right = rotate(cam->right, qx);

		quat qy = {right.x * sy, right.y * sy, right.z * sy, cy };

		quat rot = (qy * qx) * cam->tr.rot;	
		cam->tr.rot = normalize(rot);
	}

	v3 cam_pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;

	cam->tr.pos = cam_pos;
	#endif

	mat3 cam_wld = from_quat_to_mat3(cam->tr.rot); 
	cam->right = kh_right_mat3(cam_wld);
	cam->view = look_at_matrix(cam_wld, cam->tr.pos);

	for(u32 i = 0; i < render->animators.count; ++i) {
		Animator *animator = render->animators.data + i;
		update_animator(animator, &render->bone_tr, assets, dt);
	}


	for(u32 i = 0; i < array_count(sample->entities); ++i) {
		Entity *entity = sample->entities + i;
		push_render_entry(render, entity);
	}

	kh_end_transient(&f_stack);

	return(0);
}
