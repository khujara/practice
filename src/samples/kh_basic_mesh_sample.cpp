#include "kh_sample_shared.h"

struct ProgramSample {
	ProgramState state;
	Scene scene;
	v3 base_cam_pos;
	v3 base_cam_target;
	SceneEntity *player;
	LevelEditor level_editor;
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
	 	// start_recording(&global_debug_to_file, "test.txt", 1024);
		sample->base_cam_pos = kh_vec3(0.0f,30.0f,0.0f);
		sample->base_cam_target = kh_vec3(0,0.0f,0);
		render->camera = perspective_camera(w, h, sample->base_cam_pos, sample->base_cam_target, COLOR_RED, 35.0f);
		// render->camera = orthographic_camera(5.0f, w, h, sample->base_cam_pos, sample->base_cam_target, CullingMask_3D_default);

		lookat(&render->camera, sample->base_cam_pos, sample->base_cam_target, kh_vec3(0,0,-1));
		// Camera *cam = &render->camera;
		// cam->dist = kh_length_v3(render->camera.target - render->camera.tr.pos);
		// cam->tr.rot = quat_identity();
		// cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
		// set_view_matrix(cam);

		v3 light_dir = kh_normalize_v3(kh_vec3(0.577350259f, -0.577350259f, 0.577350259f));
		// v3 light_dir = kh_vec3(0,-0.707f,0.707f);
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

		sample->scene = new_scene(&program_state->arena, 16);

		AssetID plane_id = {get_or_create_tri_mesh_plane(assets)};
		AssetID cube_id = {get_or_create_tri_mesh_cube(assets)};
		
		Material_T1F3 *phong = add_mat_instance(render, Shading_phong, VertexFormat_PNU, Material_T1F3);
		phong->diffuse = get_first_asset(assets, AssetName_randy_albedo);
		phong->spec_power = 32.0f;
		phong->spec_intensity = 0.5f;
		phong->reflect_intensity = 0.0f;

		// sample->randy = new_entity(randy_meshr, kh_identity_mat4());

		Material_T1 *diffuse = add_mat_instance(render, Shading_diffuse, VertexFormat_PNU, Material_T1);
		diffuse->diffuse = get_first_asset(assets, AssetName_randy_albedo);

		Material_F4 *color = add_mat_instance(render, Shading_color, VertexFormat_PNU, Material_F4);
		color->color = kh_vec4(1,1,1,1);

		u32 plane_mesh = add_mesh_renderer(render, color, plane_id);
		u32 cube_mesh = add_mesh_renderer(render, diffuse, cube_id);
		u32 randy_mesh = add_mesh_renderer(render, diffuse, get_first_asset(assets, AssetName_randy_mesh));

		mat4 rot = kh_identity_mat4();
		rot = look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(0,1,0), kh_vec3(0,0,-1));
		mat4 scale = kh_identity_mat4(); 
		scale.m00 = 10.0f;
		scale.m11 = 10.0f;
		mat4 test = scale * rot;

		add_entity(&sample->scene, plane_mesh, test);

		sample->player = add_entity(&sample->scene, cube_mesh, kh_identity_mat4());
		set_pos(sample->player, kh_vec3(0,1,0));

		sample->level_editor = get_level_editor(memory->debug_state, render, assets, &sample->scene);
	}

	FrameState *frame_state = memory->frame_state;
	if(!frame_state) {
		frame_state = memory->frame_state = boot_strap_push_struct(FrameState, arena);
	}

	TransientLinear f_arena = kh_begin_transient(&frame_state->arena);
	begin_render_frame(render, assets);
	debug_begin_frame(memory->debug_state, render->width, render->height, render->wnd_w, render->wnd_h, dt);

	b32 right_click = was_pressed(input->mouse_buttons[MouseButton_right]);
	if(right_click) {
		render->camera.tr.pos = sample->base_cam_pos;
		render->camera.target = sample->base_cam_target;
		lookat(&render->camera, sample->base_cam_pos, sample->base_cam_target, kh_vec3(0,0,-1));
	}
	default_camera_move(&render->camera, input, dt);

	v3 add_pos = {};
	f32 speed = 5.0f;

	if(is_down(input->buttons[Button_move_up])) {
		add_pos.z -= 1.0f;
	}
	if(is_down(input->buttons[Button_move_down])) {
		add_pos.z += 1.0f;
	}
	if(is_down(input->buttons[Button_move_right])) {
		add_pos.x -= 1.0f;
	}
	if(is_down(input->buttons[Button_move_left])) {
		add_pos.x += 1.0f;
	}

	if(kh_lensqr_v3(add_pos) > 1.0f) {
		// add_pos = kh_normalize_v3(add_pos);
		add_pos *= 0.70710678118f;
	}

	v3 pos = get_pos(sample->player);
	pos += add_pos * speed * dt;
	set_pos(sample->player, pos);

	push_scene_to_render(&sample->scene, render);

	edit_level(&sample->level_editor);
	debug_end_frame(memory->debug_state);

	kh_end_transient(&f_arena);

	return(0);

}
