#include "kh_sample_shared.h"

struct ProgramSample
{
	ProgramState state;
};

Platform g_platform;
extern "C" kh_update *
frame_update(ProgramMemory *memory, Input *input, RenderManager *render, Assets *assets, f32 dt)
{
	g_platform = memory->platform;
	ProgramState *p_state = memory->p_state;
	ProgramSample *sample = (ProgramSample *)p_state;
	if(!p_state)
	{
		StackAllocator stack = {};
		p_state = memory->p_state = (ProgramState *)kh_push_struct(&stack, ProgramSample);
		p_state->stack = stack;
		sample = (ProgramSample *)p_state;

		f32 w = (f32)render->width;
		f32 h = (f32)render->height;
	 	// start_recording(&global_debug_to_file, "test.txt", 1024);
		render->camera = default_perspective_camera(w, h, kh_vec3(0.0f,5.0f,4.0f), kh_vec3(0,1.0f,0), CullingMask_3D_default, COLOR_RED);

		Camera *cam = &render->camera;

		f32 ax = 10.0f * TO_RADIANS * 0.5f;
		quat qx;
		qx.x = 1.0f * kh_sin_f32(ax);
		qx.y = 0.0f;
		qx.z = 0.0f;
		qx.w = kh_cos_f32(ax);

		f32 ay = 0.0f;
		quat qy;
		qy.x = 0.0f;
		qy.y = 1.0f * kh_sin_f32(ay);
		qy.z = 0.0f;
		qy.w = kh_cos_f32(ay);

		cam->dist = 2.0f;

		cam->tr.rot = (qy * qx);
		cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
		mat3 cam_wld = from_quat_to_mat3x3(cam->tr.rot);
		cam->right = kh_right_mat3(cam_wld);

		v3 light_dir = kh_normalize_v3(kh_vec3(0.577350259f, 0.577350259f, 0.577350259f));
		DirectionalLight light;
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.ambient_intensity = 0.1f;
		light.dir = light_dir;
		light.diffuse_intensity = 0.9f;
		define_light(render, light);


	}

	FrameState *f_state = memory->f_state;
	if(!f_state)
	{
		StackAllocator stack = {};
		f_state = memory->f_state = kh_push_struct(&stack, FrameState);
		f_state->stack = stack;
		f_state->asset_arr = assets;//alloc_assets_and_load_infos(&memory->assets, memory->high_queue, megabytes(64));
	}

	TransientStack f_stack = kh_begin_transient(&f_state->stack);
	begin_render_frame(render);


	v2 dt_mouse = kh_vec2(input->delta_mouse_x, input->delta_mouse_y);
	b32 middle_down = input->mouse_buttons[MouseButton_middle].down;
	f32 wheel = (f32)input->dt_wheel;

	// cam->pos += kh_normalize_v3(cam->target - cam->pos) * 0.2f * wheel * dt;

	Camera *cam = &render->camera;
	cam->dist -= wheel * 0.1f * dt;

	if(middle_down)
	{
		f32 ax = dt_mouse.x * dt * 0.125f;
		f32 ay = -dt_mouse.y * dt * 0.125f;


		float cx = kh_cos_f32(ax);
		float sx = kh_sin_f32(ax);
		float cy = kh_cos_f32(ay);
		float sy = kh_sin_f32(ay);

		quat qx;
		qx.x = 0.0f;
		qx.y = 1.0f * sx;
		qx.z = 0.0f;
		qx.w = cx;

		v3 right = rotate(cam->right, qx);
		quat qy;
		qy.x = right.x * sy;
		qy.y = right.y * sy;
		qy.z = right.z * sy;
		qy.w = cy;

		quat rot = (qy * qx) * cam->tr.rot;
		cam->tr.rot = normalize(rot);
	}

	v3 cam_pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
	cam->tr.pos = cam_pos;

	mat3 cam_wld = from_quat_to_mat3x3(cam->tr.rot); 
	cam->right = kh_right_mat3(cam_wld);
	cam->view = look_at_matrix(cam_wld, cam->tr.pos);

	begin_render_frame(render);

	TextureID texture_none = {};
	begin_vertex_format(render, VertexFormat_PosNormalUV);
	begin_material(render, Material_phong);
	begin_material_instance(render, assets, get_first_texture_2d(assets, AssetName_randy_albedo), texture_none, kh_vec4(1,1,1,1));
	MeshRenderer *randy = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_randy_mesh));
	end_material_instance(render);
	end_material(render);
	end_vertex_format(render);

	mat4 mat = kh_identity_mat4();
	push_render_entry(render, randy, mat);

	kh_end_transient(&f_stack);

	return(0);

}

