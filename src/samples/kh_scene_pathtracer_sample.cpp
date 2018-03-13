#include "kh_sample_shared.h"
#include "..\kh_pathtracing.h"

#define STBTT_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

#include "..\kh_pathtracing.cpp"

struct ProgramSample {
	ProgramState state;
	Scene *cur_scene;

	Camera first_cam;

	Scene path_scene;
	Scene cornell_scene;
	PathSolution test_pathtracing;
};

KH_INTERN void
debug_box(DebugState *debug, AABB *a, v3 color) {
	v3 v_0 = a->min;
	v3 v_1 = kh_vec3(a->max.x, a->min.y, a->min.z);
	v3 v_2 = kh_vec3(a->max.x, a->max.y, a->min.z);
	v3 v_3 = kh_vec3(a->min.x, a->max.y, a->min.z);

	v3 v_4 = a->max;
	v3 v_5 = kh_vec3(a->min.x, a->max.y, a->max.z);
	v3 v_6 = kh_vec3(a->min.x, a->min.y, a->max.z);
	v3 v_7 = kh_vec3(a->max.x, a->min.y, a->max.z);

	debug_line(debug, v_0, v_1, color); 
	debug_line(debug, v_1, v_2, color);
	debug_line(debug, v_2, v_3, color);
	debug_line(debug, v_3, v_0, color);
	debug_line(debug, v_4, v_5, color);
	debug_line(debug, v_5, v_6, color); 
	debug_line(debug, v_6, v_7, color);
	debug_line(debug, v_7, v_4, color);
	debug_line(debug, v_3, v_5, color);
	debug_line(debug, v_0, v_6, color);
	debug_line(debug, v_1, v_7, color);
	debug_line(debug, v_2, v_4, color);
}

static void
debug_bvh_level(DebugState *debug, BVHNode *root, v3 color, int level) {
	if(!root) return;
	if(level == 1) {
			debug_box(debug, root->box, color);
	} else {
		debug_bvh_level(debug, root->left, color, level - 1);
		debug_bvh_level(debug, root->right, color, level - 1);
	}
}

static void
debug_bvh(DebugState *debug, PathSolution *solution, u32 depth) {
	if(depth) {
		f32 inv_depth = 1.0f / (f32)depth;
		for(u32 i = 2; i < (depth + 1); ++i) {
			v3 color = kh_mix_v3(kh_vec3(1,1,1), kh_vec3(0,0,0), (f32)i * inv_depth);
			debug_bvh_level(debug, solution->bvh.root, color, i);
		}
	}
}

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
		lookat(cam, kh_vec3(-1.5f, 1.5f, -20.0f), kh_vec3(0,0,0), kh_vec3(0,1,0));

		AssetID right = get_first_asset(assets, AssetName_skybox_right);
		AssetID left = get_first_asset(assets, AssetName_skybox_left);
		AssetID bottom = get_first_asset(assets, AssetName_skybox_bottom);
		AssetID top = get_first_asset(assets, AssetName_skybox_top);
		AssetID back = get_first_asset(assets, AssetName_skybox_back);
		AssetID front = get_first_asset(assets, AssetName_skybox_front);

		define_skybox(render, assets, right, left, bottom, top, back, front);
		v3 light_pos = kh_vec3(0, 0, -1.5f);
		// v3 light_dir = kh_normalize_v3(kh_vec3(0.5f, -0.6f, 0.70710678118f));
		Light light;
		light.type = LightType_point;
		light.color = kh_vec3(10.0f, 10.0f, 10.0f);
		light.cutoff = 0;
		light.outer_cutoff = 0;
		light.dir = kh_normalize_v3(light_pos);
		light.pos = light_pos;
		light.scale = 15.0f;
		light.z_near = 0.0f;
		light.z_far = 40.0f;
		add_light(render, light);

		AssetID cube_mesh_id   = {get_or_create_tri_mesh_cube(assets)};
		AssetID plane_mesh_id = {get_or_create_tri_mesh_plane(assets)};
		AssetID sphere_mesh_id = {get_or_create_tri_mesh_sphere(assets)};
		u32 tex_sizes = 1;
		AssetID red_texture = {get_or_create_empty_texture(assets, tex_sizes, tex_sizes, "red_texture")};
		LoadedAsset red_asset = get_loaded_asset(assets, red_texture, AssetType_tex2d);
		u32 bpp = red_asset.type->tex2d.bytes_per_pixel;
		u8 *pixels = red_asset.data;
		for(u32 pixels_i = 0; pixels_i < (tex_sizes*tex_sizes); ++pixels_i) {
			pixels[0] = 0x00;
			pixels[1] = 0x00;
			pixels[2] = 0xFF;
			pixels += bpp;
		}
		AssetID green_texture = {get_or_create_empty_texture(assets, tex_sizes, tex_sizes, "green_texture")};
		LoadedAsset green_asset = get_loaded_asset(assets, green_texture, AssetType_tex2d);
		pixels = green_asset.data;
		for(u32 pixels_i = 0; pixels_i < (tex_sizes*tex_sizes); ++pixels_i) {
			pixels[0] = 0x00;
			pixels[1] = 0xFF;
			pixels[2] = 0x00;
			pixels += bpp;
		}
		AssetID blue_texture = {get_or_create_empty_texture(assets, tex_sizes, tex_sizes, "blue_texture")};
		LoadedAsset blue_asset = get_loaded_asset(assets, blue_texture, AssetType_tex2d);
		pixels = blue_asset.data;
		for(u32 pixels_i = 0; pixels_i < (tex_sizes*tex_sizes); ++pixels_i) {
			pixels[0] = 0xFF;
			pixels[1] = 0x00;
			pixels[2] = 0x00;
			pixels += bpp;
		}
		AssetID white_texture = {get_or_create_empty_texture(assets, tex_sizes, tex_sizes, "white_texture")};
		LoadedAsset white_asset = get_loaded_asset(assets, white_texture, AssetType_tex2d);
		pixels = white_asset.data;
		for(u32 pixels_i = 0; pixels_i < (tex_sizes*tex_sizes); ++pixels_i) {
			pixels[0] = 0xFF;
			pixels[1] = 0xFF;
			pixels[2] = 0xFF;
			pixels += bpp;
		}

		sample->cornell_scene = new_scene(&program_state->arena, 32);
		Material_T1F3 *white_phong = add_mat_instance(render, Shading_phong, VertexFormat_PNU, Material_T1F3);
		white_phong->diffuse = white_texture;
		white_phong->spec_power = 32.0f;
		white_phong->spec_intensity = 0.5f;
		white_phong->reflect_intensity = 0.0f;
		u32 plane_meshr = add_mesh_renderer(render, white_phong, plane_mesh_id);
		mat4 plane_tr = kh_identity_mat4();
		plane_tr.m00 = 3;
		plane_tr.m11 = 2;
		add_entity(&sample->cornell_scene, plane_meshr, plane_tr);

		sample->path_scene = new_scene(&program_state->arena, 1024);
		Material_T1F3 *phong = add_mat_instance(render, Shading_phong, VertexFormat_PNU, Material_T1F3);
		phong->diffuse = get_first_asset(assets, AssetName_randy_albedo);
		phong->spec_power = 32.0f;
		phong->spec_intensity = 0.5f;
		phong->reflect_intensity = 0.0f;

		u32 randy_meshr = add_mesh_renderer(render, phong, get_first_asset(assets, AssetName_randy_mesh));
		mat4 randy_tr = kh_identity_mat4();
		randy_tr.c3 = kh_vec4(0, -1, 0, 1);
		add_entity(&sample->path_scene, randy_meshr, randy_tr);
		randy_tr.c3 = kh_vec4(0, -1, -2, 1);
		// add_entity(&sample->path_scene, randy_meshr, randy_tr);

		// sample->cur_scene = &sample->cornell_scene;
		sample->cur_scene = &sample->path_scene;
		init_path_solution(&sample->test_pathtracing, assets, render, sample->cur_scene, &program_state->arena);

		// char *scene_file = "test.khscene";
		// load_scene_for_game(&sample->scene, render, assets, scene_file);

		sample->first_cam = render->camera;
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

	default_camera_move(&render->camera, input, dt, 0.5f);

	push_scene_to_render(sample->cur_scene, render);

	PathSolution *solution = &sample->test_pathtracing;
	// TODO(flo): GUI and better input handling
	if(was_pressed(input->mouse_buttons[MouseButton_right])) {
		kh_assert(sample->test_pathtracing.out_texture.val != 0);
		// get_debug_scene(&sample->prim_list);
		pathtracer_render(&sample->test_pathtracing, &render->camera);
		Asset *asset = get_asset(assets, sample->test_pathtracing.out_texture, AssetType_tex2d);
		asset->header.gpu_reload = true;
		u32 w = asset->source.type.tex2d.width;
		u32 h = asset->source.type.tex2d.height;
		u8 *pixels = asset->header.data;
		write_to_bmp_rgb("test.bmp", w, h, pixels);
		int debugp = 2;
	}

	// u32 w = 64;
	// u32 h = 32;
	// LensCamera lcam = ray_camera_lookat(&sample->first_cam, 1.0f, 0.0f, 10.0f);
	// for(u32 y = 0; y < h; ++y) {
	// 	for(u32 x = 0; x < w; ++x) {

	// 		f32 u = (f32)x / w;
	// 		f32 v = (f32)y / h;
	// 		PathRay ray = get_ray_from_camera(solution, &lcam, u, v);

	// 		debug_line(debug, V3_ZERO, ray.dir, kh_vec3(1,0,0));
	// 	}
	// }

	// if(solution->prim_list.count) {
	// 	debug_bvh(debug, solution, 2);
	// }

	push_render_entry(render, &solution->entity);
	debug_end_frame(debug);
	kh_end_transient(&f_arena);

	return(0);
}