#include "kh_sample_shared.h"

struct ProgramSample {
	ProgramState state;
};

Platform g_platform;
extern "C" kh_update *
frame_update(ProgramMemory *memory, Input *input, RenderManager *render, Assets *assets, f32 dt) {
	// khjr_assert(!"learn the math for perspective projection");
	g_platform = memory->platform;
	if(!memory->p_state) {

		StackAllocator stack = {};
		memory->p_state = (ProgramState *)kh_push_struct(&stack, ProgramSample);
		ProgramState *p_state = memory->p_state;
		p_state->stack = stack;
		ProgramSample *sample = (ProgramSample *)p_state;

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
		mat3 cam_wld = from_quat_to_mat3x3(cam->tr.rot);
		cam->right = kh_right_mat3(cam_wld);

		TextureID right = get_first_texture_2d(assets, AssetName_skybox_right);
		TextureID left = get_first_texture_2d(assets, AssetName_skybox_left);
		TextureID bottom = get_first_texture_2d(assets, AssetName_skybox_bottom);
		TextureID top = get_first_texture_2d(assets, AssetName_skybox_top);
		TextureID back = get_first_texture_2d(assets, AssetName_skybox_back);
		TextureID front = get_first_texture_2d(assets, AssetName_skybox_front);

		define_skybox(assets, render, right, left, bottom, top, back, front);
		v3 light_dir = kh_normalize_v3(kh_vec3(0.5f, -0.6f, 0.70710678118f));
		DirectionalLight light;
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.ambient_intensity = 0.15f;
		light.dir = light_dir;
		light.diffuse_intensity = 0.8f;
		define_light(render, light);
	}

	if(!memory->f_state) {
		StackAllocator stack = {};
		memory->f_state = kh_push_struct(&stack, FrameState);
		FrameState *f_state = memory->f_state;
		f_state->stack = stack;
		f_state->asset_arr = assets;
	}

	ProgramState *p_state = memory->p_state;
	FrameState *f_state = memory->f_state;
	kh_assert(p_state && f_state);

	TransientStack f_stack = kh_begin_transient(&f_state->stack);
	begin_render_frame(render);

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

	mat3 cam_wld = from_quat_to_mat3x3(cam->tr.rot); 
	cam->right = kh_right_mat3(cam_wld);
	cam->view = look_at_matrix(cam_wld, cam->tr.pos);

	TextureID texture_none = {};
	begin_vertex_format(render, VertexFormat_PosNormalUV);
	begin_material(render, Material_phong);
	begin_material_instance(render, assets, get_first_texture_2d(assets, AssetName_randy_albedo), texture_none, kh_vec4(1,1,1,1));
	MeshRenderer *phong_randy = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_randy_mesh));
	MeshRenderer *phong_ico = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_icosphere));
	end_material_instance(render);
	begin_material_instance(render, assets, get_first_texture_2d(assets, AssetName_bricks), texture_none, kh_vec4(1,1,1,1));
	MeshRenderer *phong_cube = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_cube));
	end_material_instance(render);
	end_material(render);
	end_vertex_format(render);

	begin_vertex_format(render, VertexFormat_PosNormalTangentBitangentUV);
	begin_material(render, Material_normalmap);
	begin_material_instance(render, assets, get_first_texture_2d(assets, AssetName_wall), get_first_texture_2d(assets, AssetName_wall_normal), kh_vec4(1,1,1,1));
	MeshRenderer *normal_terrain = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_terrain_test));
	end_material_instance(render);
	begin_material_instance(render, assets, get_first_texture_2d(assets, AssetName_randy_albedo), get_first_texture_2d(assets, AssetName_randy_normal), kh_vec4(1,1,1,1));
	MeshRenderer *normal_randy = push_mesh_renderer(render, assets, get_first_TriangleMeshID(assets, AssetName_randy_mesh_tangent));
	end_material_instance(render);
	end_material(render);
	end_vertex_format(render);

	mat4 vp = kh_identity_mat4();
	v4 mx = kh_vec4(1, 0, 0, 0);
	v4 my = kh_vec4(0, 1, 0, 0);
	v4 mz = kh_vec4(0, 0, 1, 0);
	push_render_entry(render, normal_randy, m44(mx, my, mz, kh_vec4(0,0,0.2f,1))); 
	push_render_entry(render, normal_randy, m44(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(2,0,0,1))); 
	push_render_entry(render, normal_randy, m44(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(0,0,2,1))); 
	push_render_entry(render, normal_randy, m44(mx, my, mz, kh_vec4(0,0,-2,1))); 
	push_render_entry(render, phong_ico, 	m44(mx, my, mz, kh_vec4(2,3,-2,1)));
	push_render_entry(render, normal_randy, m44(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-2,0,0,1))); 
	push_render_entry(render, normal_randy, m44(mx, my, mz, kh_vec4(3,0, -3,1))); 
	push_render_entry(render, normal_randy, m44(kh_vec4(0, 0, 1, 0), my, kh_vec4(-1, 0, 0, 0), kh_vec4(3,0,3,1))); 
	push_render_entry(render, normal_randy, m44(kh_vec4(-1, 0, 0, 0), my, kh_vec4(0, 0, -1, 0), kh_vec4(-3,0,3,1))); 
	push_render_entry(render, normal_randy, m44(kh_vec4(0, 0, -1, 0), my, kh_vec4(1,0,0,0), kh_vec4(-2,0,-3,1))); 

	push_render_entry(render, phong_cube, m44(mx, my, mz, kh_vec4(0,3,0,1)));

	push_render_entry(render, phong_ico, m44(mx, my, mz, kh_vec4(4,0,0,1)));
	push_render_entry(render, phong_ico, m44(mx, my, mz, kh_vec4(-4,0,0,1)));

	push_render_entry(render, normal_terrain, m44(mx, my, mz, kh_vec4(0,-1.5f,0,1)));
	push_render_entry(render, phong_randy, m44(mx, my, mz, kh_vec4(0, 0, -5, 1)));	

	push_render_entry(render, normal_randy, m44(mx, my, mz, kh_vec4(0,0,-0.3f,1)));

	kh_end_transient(&f_stack);

	return(0);
}
