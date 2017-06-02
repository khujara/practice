#include "kh_sample_shared.h"

enum MeshRendererType {
	MeshRenderer_randy,
	MeshRenderer_count,	
};

struct ProgramSample
{
	ProgramState state;

	Entity randy;
	Entity randy_2;

	StackAllocator animation_mem;
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

		init_render(render);

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

		cam->dist = 4.0f;

		cam->tr.rot = (qy * qx);
		cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
		mat3 cam_wld = from_quat_to_mat3(cam->tr.rot);
		cam->right = kh_right_mat3(cam_wld);

		v3 light_dir = kh_normalize_v3(kh_vec3(0.577350259f, 0.577350259f, 0.577350259f));
		DirectionalLight light;
		light.color = kh_vec3(1.0f, 1.0f, 1.0f);
		light.ambient_intensity = 0.1f;
		light.dir = light_dir;
		light.diffuse_intensity = 0.9f;
		define_light(render, light);

		// char *filename      = "models/randy.dae";
		// char *skeleton_str  = "skeleton";
		// char *skin_str      = "skin";
		// char *animation_str = "animation0";
		// AssetID skeleton = {get_or_create_asset_id_from_name(assets, skeleton_str)};
		// MeshSkinID skin = {get_or_create_asset_id_from_name(assets, skin_str)};
		// AnimationID animation = {get_or_create_asset_id_from_name(assets, animation_str)};
		// u32 randy_id = get_or_create_asset_id_from_name(assets, filename);
		// TriangleMeshID randy_mesh_id = {randy_id};
		// DEBUG_load_trimesh_directly(assets, filename, VertexFormat_PosNormalTangentBitangentUVSkinned, 
		//                             skin_str, skeleton_str, animation_str);

		// char *filename = "models/boblampclean.md5mesh";
		u32 buffer = add_vertex_format(render, VertexFormat_PosNormalTangentBitangentUVSkinned);
		u32 mat = add_material(render, buffer, Material_normalmapskinned);
		u32 instance = add_material_instance(render, mat, assets, 
		                                     get_first_asset(assets, AssetName_randy_albedo),
		                                     get_first_asset(assets, AssetName_randy_normal), kh_vec4(1,1,1,1));
		u32 randy = add_mesh_renderer(render, instance, assets, get_first_asset(assets, AssetName_randy_mesh_skinned));

		AssetID skeleton = get_first_asset(assets, AssetName_randy_skeleton);
		AssetID skin = get_first_asset(assets, AssetName_randy_skin);
		AssetID anim = get_first_asset(assets, AssetName_randy_idle);

		sample->randy.mesh_renderer = randy;
		sample->randy.transform = kh_identity_mat4();
		sample->randy.animator = add_animator(&render->animators, &render->bone_tr, assets, skeleton, skin, anim);

		sample->randy_2.mesh_renderer = randy;
		mat4 tr = kh_identity_mat4();
		kh_set_translation_mat4(&tr, kh_vec3(3,0,0));
		sample->randy_2.transform = tr;
		sample->randy_2.animator = add_animator(&render->animators, &render->bone_tr, assets, skeleton, skin, anim);

		set_playback_rate(&render->animators, sample->randy.animator, 0.5f);
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
	begin_render_frame(render, assets);

	v2 dt_mouse = kh_vec2(input->delta_mouse_x, input->delta_mouse_y);
	b32 middle_down = input->mouse_buttons[MouseButton_middle].down;
	f32 wheel = (f32)input->dt_wheel;

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

	mat3 cam_wld = from_quat_to_mat3(cam->tr.rot); 
	cam->right = kh_right_mat3(cam_wld);
	cam->view = look_at_matrix(cam_wld, cam->tr.pos);

	// TODO(flo): what should we do with this ?
	for(u32 i = 0; i < render->animators.count; ++i) {
		Animator *animator = render->animators.data + i;
		update_animator(animator, &render->bone_tr, assets, dt);
	}
	push_render_entry(render, &sample->randy);
	push_render_entry(render, &sample->randy_2);

	kh_end_transient(&f_stack);

	return(0);

}