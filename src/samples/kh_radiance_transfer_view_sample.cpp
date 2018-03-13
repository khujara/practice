#include "kh_sample_shared.h"
#include "kh\kh_pbr.h"

#define STBTT_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"
#include <string>

// TODO(flo): add a recompiler for the shaders to change #define in vertex/fragment programs to change
// brdf function

// TODO(flo): implement real world values!
struct Emitter {
	u32 plane_r;
	u32 sphere_r;

	mat4 tr;
	f32 scale;

	Light light;

	f32 dist;
	f32 angle_from_wld_origin;
};

struct Receiver {
	v3 albedo;
	f32 metallic;
	f32 roughness;
	f32 ao;
};

typedef void BRDFFunc(Vertex_PNUC *vertices, u32 vertex_count, mat4 tr, Light *light, Receiver *r, v3 view_pos);

struct ProgramSample {
	ProgramState state;

	Emitter emitter;
	Receiver receiver;

	BRDFFunc *brdf_func;
	b32 show_axis;

	u32 entity_to_use;
	Entity sphere;
	Entity plane;
	Entity model;

	v3 light_rgb_color;
	f32 light_rgb_intensity;

	LevelEditor level_editor;
};

enum BRDFType {
	BRDFType_CookTorrance,
	BRDFType_Phong,
	BRDFType_PhongModified,
	BRDFType_BlinnPhong,
};

struct RangeFloat {
	f32 min;
	f32 max;
	f32 val;
	char *name;
};

struct BRDFCookTorrance {
	RangeFloat roughness;
	RangeFloat metallic;
};

struct BRDF {
	BRDFType cur_type;
	BRDFFunc *cur_brdf;
	BRDFCookTorrance cook_torrance;
};

KH_INTERN void
cook_torrance(Vertex_PNUC *vertices, u32 vertex_count, mat4 tr, Light *light, Receiver *r, v3 view_pos) {
	v3 albedo = r->albedo;
	f32 metallic = r->metallic;
	f32 roughness = r->roughness;
	for(u32 i = 0; i < vertex_count; ++i) {
		Vertex_PNUC *vert = vertices + i;
		v3 pos = kh_vec3(tr * kh_vec4(vert->pos, 1.0f));
		v3 normal = kh_vec3(tr * kh_vec4(vert->normal, 0.0f));
		v3 Lo = cook_torrance_brdf(light, pos, normal, albedo, view_pos, roughness, metallic); 

		v3 ambient = r->albedo * 0.03f * r->ao;
		v3 color = ambient + Lo;

		color = kh_divide_v3(color, color + kh_vec3(1.0f));
		color = kh_sqrt_v3(color);
		vert->color = color; 
	}
}

KH_INTERN void
blinn_phong(Vertex_PNUC *vertices, u32 vertex_count, mat4 tr, Light *light, Receiver *r, v3 view_pos) {
	v3 albedo = r->albedo;
	f32 roughness = r->roughness;
	for(u32 i = 0; i < vertex_count; ++i) {
		Vertex_PNUC *vert = vertices + i;
		v3 pos = kh_vec3(tr * kh_vec4(vert->pos, 1.0f));
		v3 normal = kh_vec3(tr * kh_vec4(vert->normal, 0.0f));
		v3 color = blinn_phong_brdf(light, pos, normal, albedo, view_pos, roughness);
		color = kh_divide_v3(color, color + kh_vec3(1.0f));
		color = kh_sqrt_v3(color);
		vert->color = color; 
	}
}

KH_INTERN void
phong(Vertex_PNUC *vertices, u32 vertex_count, mat4 tr, Light *light, Receiver *r, v3 view_pos) {
	v3 albedo = r->albedo;
	f32 roughness = r->roughness;
	for(u32 i = 0; i < vertex_count; ++i) {
		Vertex_PNUC *vert = vertices + i;
		v3 pos = kh_vec3(tr * kh_vec4(vert->pos, 1.0f));
		v3 normal = kh_vec3(tr * kh_vec4(vert->normal, 0.0f));
		v3 color = phong_brdf(light, pos, normal, albedo, view_pos, roughness);
		color = kh_divide_v3(color, color + kh_vec3(1.0f));
		color = kh_sqrt_v3(color);
		vert->color = color; 
	}
}

KH_INTERN mat4
get_mat4_from_angle_at_dist(f32 angle, f32 dist, v3 receiver_pos) {

	f32 s = -kh_sin_f32(angle);
	f32 c = kh_cos_f32(angle);

	v3 pos;
	pos.x = s * dist;
	pos.y = c * dist;
	pos.z = 0.0f;

	pos += receiver_pos;

	v3 y_axis = kh_vec3(s, c, 0);
	v3 x_axis = kh_cross_v3(y_axis, kh_vec3(0,0,1));
	v3 z_axis = kh_cross_v3(x_axis, y_axis);

	mat4 res = kh_mat4(kh_mat3(x_axis, -z_axis, y_axis), pos);
	return(res);
}

/*TODO(flo):  Features we want
	- Multiple lights!
	- Switch to OpenGL with randy model with the currently selected BRDF
	- Show curves of function D, G and F
	
	- Each BRDF has its own parameters (ie cooktorrance has roughness/metalness/ao)
	- Set if we want to show only specular and only specular+diffuse
	- In the Cook Torrance BRDF, we can specify D/F/G functions
	- BSSDF BTDF etc...
	- Anisotropic NDFs
	
	- CookTorrance parameters
		- metalness
		- roughness
		- D
		- F
		- G
	- Phong parameters
		- power
	- BlinnPhong parameters
		- power

 	- A scene with precomputed lighting (pathtracing)!

*/


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
		lookat(cam, kh_vec3(-1.5f, 1.5f, -5.0f), kh_vec3(0,0,0), kh_vec3(0,1,0));

		sample->brdf_func = cook_torrance;

		AssetID right = get_first_asset(assets, AssetName_skybox_right);
		AssetID left = get_first_asset(assets, AssetName_skybox_left);
		AssetID bottom = get_first_asset(assets, AssetName_skybox_bottom);
		AssetID top = get_first_asset(assets, AssetName_skybox_top);
		AssetID back = get_first_asset(assets, AssetName_skybox_back);
		AssetID front = get_first_asset(assets, AssetName_skybox_front);

		define_skybox(render, assets, right, left, bottom, top, back, front);

		{
			AssetID sphere_mesh_id = {get_or_create_tri_mesh_sphere(assets)};
			AssetID dplane_mesh_id = {get_or_create_tri_mesh_double_sided_plane(assets)};
			AssetID cube_mesh_id = {get_or_create_tri_mesh_cube(assets)};
			Material_F4 *mat_color = add_mat_instance(render, Shading_color, VertexFormat_PNU, Material_F4);
			mat_color->color = kh_vec4(1,1,1,1);
			sample->emitter.dist = 5.0f;
			sample->emitter.angle_from_wld_origin = -45.0f;
			sample->emitter.scale = 0.2f;
			AssetID emitter_tex = {get_or_create_empty_texture(assets, 1, 1, "white_texture")};
			set_texture_pixel_color(assets, emitter_tex, 0xFF, 0xFF, 0xFF);
			sample->emitter.sphere_r = add_mesh_renderer(render, mat_color, sphere_mesh_id);
			sample->emitter.plane_r = add_mesh_renderer(render, mat_color, dplane_mesh_id);
			// v3 emitter_pos = render->camera.target + kh_vec3(0, sample->emitter.dist, 0);
			// mat3 rot = kh_mat3(kh_vec3(1,0,0), kh_vec3(0,0,-1), kh_vec3(0,1,0));
			// sample->emitter.tr = kh_mat4(rot, emitter_pos);
			sample->emitter.tr = get_mat4_from_angle_at_dist(sample->emitter.angle_from_wld_origin, sample->emitter.dist, kh_vec3(0,0,0));
			sample->emitter.light.type = LightType_directional;
			sample->emitter.light.scale = 15.0f;
			sample->emitter.light.z_near = 0.0f;
			sample->emitter.light.z_far = 40.0f;
			sample->emitter.light.dir = kh_normalize_v3(kh_vec3(sample->emitter.tr.c2));
			sample->emitter.light.pos = kh_vec3(sample->emitter.tr.c3);
			// sample->light_rgb_color = kh_vec3(1.0f, 0.576f, 0.161f);
			sample->light_rgb_color = kh_vec3(0.788f, 0.886f, 1.0f);
			sample->light_rgb_intensity = 6.0f;
			sample->emitter.light.color = sample->light_rgb_color * sample->light_rgb_intensity;
			kh_scale_mat4(&sample->emitter.tr, sample->emitter.scale);

			add_light(render, sample->emitter.light);

#if 1
			AssetID texture_alb       = get_first_asset(assets, AssetName_randy_albedo);
			AssetID texture_norm      = get_first_asset(assets, AssetName_randy_normal);
			AssetID texture_metal     = get_first_asset(assets, AssetName_randy_metalness);
			AssetID texture_roughness = get_first_asset(assets, AssetName_randy_gloss);
			AssetID texture_ao        = get_first_asset(assets, AssetName_randy_ao);
#else
			AssetID texture_alb       = get_first_asset(assets, AssetName_gold_albedo);
			AssetID texture_norm      = get_first_asset(assets, AssetName_gold_normal);
			AssetID texture_metal     = get_first_asset(assets, AssetName_gold_metallic);
			AssetID texture_roughness = get_first_asset(assets, AssetName_gold_roughness);
			AssetID texture_ao        = get_first_asset(assets, AssetName_gold_ao);
#endif
			Material_T5 *instance = add_mat_instance(render, Shading_pbr, VertexFormat_PNU, Material_T5);
			instance->albedo = texture_alb;
			instance->normal = texture_norm;
			instance->roughness = texture_roughness;
			instance->metalness = texture_metal;
			instance->ao = texture_ao;

			// Material_F4 *plane_color = add_mat_instance(render, Shading_color, Material_F4);
			// plane_color->color = kh_vec4(0,0,0,1);
			sample->plane.transform = kh_mat4(kh_vec3(1,0,0), kh_vec3(0,0,1), kh_vec3(0,-1,0), kh_vec3(0,0,0));
			sample->plane.animator = INVALID_U32_OFFSET;
			sample->plane.mesh_renderer = add_mesh_renderer(render, instance, cube_mesh_id);
			// brdf_plane(sample, render, assets);
		}
		{
			sample->receiver.albedo = kh_vec3(1.0f,0.765557f,0.336057f);
			sample->receiver.roughness = 0.5f;
			sample->receiver.metallic = 1.0f;
			sample->receiver.ao = 0.5f;
		}
		{

			AssetID texture_alb       = get_first_asset(assets, AssetName_gold_albedo);
			AssetID texture_norm      = get_first_asset(assets, AssetName_gold_normal);
			AssetID texture_metal     = get_first_asset(assets, AssetName_gold_metallic);
			AssetID texture_roughness = get_first_asset(assets, AssetName_gold_roughness);
			AssetID texture_ao        = get_first_asset(assets, AssetName_gold_ao);

			Material_T5 *instance = add_mat_instance(render, Shading_pbr, VertexFormat_PNU, Material_T5);
			instance->albedo = texture_alb;
			instance->normal = texture_norm;
			instance->roughness = texture_roughness;
			instance->metalness = texture_metal;
			instance->ao = texture_ao;

			mat4 tr = kh_mat4(kh_identity_mat3(), render->camera.target);
			AssetID mesh_id = {get_or_create_tri_mesh_sphere(assets)};
			// AssetID mesh_id = {get_or_create_tri_mesh_sphere_pnuc(assets, "brdf_sphere", 24)};
			sample->sphere.transform = tr;
			sample->sphere.animator = INVALID_U32_OFFSET;
			sample->sphere.mesh_renderer = add_mesh_renderer(render, instance, mesh_id);
			// brdf_sphere(sample, render, assets);
		}
		{
#if 1
			AssetID texture_alb       = get_first_asset(assets, AssetName_randy_albedo);
			AssetID texture_norm      = get_first_asset(assets, AssetName_randy_normal);
			AssetID texture_metal     = get_first_asset(assets, AssetName_randy_metalness);
			AssetID texture_roughness = get_first_asset(assets, AssetName_randy_gloss);
			AssetID texture_ao        = get_first_asset(assets, AssetName_randy_ao);
#else
			AssetID texture_alb       = get_first_asset(assets, AssetName_gold_albedo);
			AssetID texture_norm      = get_first_asset(assets, AssetName_gold_normal);
			AssetID texture_metal     = get_first_asset(assets, AssetName_gold_metallic);
			AssetID texture_roughness = get_first_asset(assets, AssetName_gold_roughness);
			AssetID texture_ao        = get_first_asset(assets, AssetName_gold_ao);
#endif
			AssetID randy_id          = get_first_asset(assets, AssetName_randy_mesh_skinned);
			AssetID skeleton          = get_first_asset(assets, AssetName_randy_skeleton);
			AssetID skin              = get_first_asset(assets, AssetName_randy_skin);
			AssetID anim              = get_first_asset(assets, AssetName_randy_idle_skin);

			// u32 mat = add_shading(render, vert_format, Shading_normalmapskinned, Material_normal_map);
			// Material_NormalMap pbr = { texture_alb, texture_norm, 16.0f, 1.0f};
			Material_T5 *pbr = add_mat_instance(render, Shading_pbr, VertexFormat_PNUTBS, Material_T5);
			pbr->albedo = texture_alb;
			pbr->normal = texture_norm;
			pbr->roughness = texture_roughness;
			pbr->metalness = texture_metal;
			pbr->ao = texture_ao;
			u32 meshr = add_mesh_renderer(render, pbr, randy_id);
			sample->model = new_entity(meshr, kh_mat4(kh_identity_mat3(), kh_vec3(0,-1,0)), add_animator(&render->animators, skeleton, skin, anim));
			
		}
		sample->show_axis = false;
		sample->entity_to_use = 2;
		sample->level_editor = get_level_editor(memory->debug_state, render, assets, 0);
	}
	FrameState *frame_state = memory->frame_state;
	if(!frame_state) {
		frame_state = memory->frame_state = boot_strap_push_struct(FrameState, arena);
	}

	DebugState *debug = memory->debug_state;
	TransientLinear f_arena = kh_begin_transient(&frame_state->arena);
	begin_render_frame(render, assets);
	debug_begin_frame(debug, render->width, render->height, render->wnd_w, render->wnd_h, dt);
	Camera *cam = &render->camera;
	default_camera_move(cam, input, dt, 0.1f);

	Emitter *emit = &sample->emitter;
	Receiver *receiver = &sample->receiver;

	v3 receiver_pos = kh_vec3(0,0,0);
	if(is_down(input->buttons[Button_move_left])) {
		emit->angle_from_wld_origin += 90.0f * TO_RADIANS * dt;
		emit->tr = get_mat4_from_angle_at_dist(emit->angle_from_wld_origin, emit->dist, receiver_pos);
		emit->light.dir = kh_normalize_v3(kh_vec3(emit->tr.c2));
		emit->light.pos = kh_vec3(emit->tr.c3);
		kh_scale_mat4(&emit->tr, emit->scale);
	} else if(is_down(input->buttons[Button_move_right])) {
		emit->angle_from_wld_origin -= 90.0f * TO_RADIANS * dt;
		emit->tr = get_mat4_from_angle_at_dist(emit->angle_from_wld_origin, emit->dist, receiver_pos);
		emit->light.dir = kh_normalize_v3(kh_vec3(emit->tr.c2));
		emit->light.pos = kh_vec3(emit->tr.c3);
		kh_scale_mat4(&emit->tr, emit->scale);
	} else if(is_down(input->buttons[Button_move_up])) {
		emit->dist += 5.0f * dt;
		emit->tr = get_mat4_from_angle_at_dist(emit->angle_from_wld_origin, emit->dist, receiver_pos);
		emit->light.dir = kh_normalize_v3(kh_vec3(emit->tr.c2));
		emit->light.pos = kh_vec3(emit->tr.c3);
		kh_scale_mat4(&emit->tr, emit->scale);
	} else if(is_down(input->buttons[Button_move_down])) {
		emit->dist -= 5.0f * dt;
		emit->tr = get_mat4_from_angle_at_dist(emit->angle_from_wld_origin, emit->dist, receiver_pos);
		emit->light.dir = kh_vec3(emit->tr.c2);
		emit->light.pos = kh_vec3(emit->tr.c3);
		kh_scale_mat4(&emit->tr, emit->scale);
	}

	// NOTE(flo): BRDF
	static b32 brdf_wnd = true;
	debug_begin_wnd(debug, "BRDF parameters", &brdf_wnd);
	static u32 brdf_use = 0;
	u32 old_brdf_use = brdf_use; 
	if(debug_collapse(debug, "BRDF")) {
		debug_radio_button(debug, "Cook BRDF", 0, &brdf_use);
		debug_radio_button(debug, "Blinn-Phong BRDF", 1, &brdf_use);
		debug_radio_button(debug, "Phong BRDF", 2, &brdf_use);
		if(brdf_use != old_brdf_use) {
			if(brdf_use == 0) {
				sample->brdf_func = cook_torrance;
			} else if(brdf_use == 1) {
				sample->brdf_func = blinn_phong;
			} else if(brdf_use == 2) {
				sample->brdf_func = phong;
			}
			old_brdf_use = brdf_use;
		}
	}
	static u32 ndf_use = 0;
	u32 old_ndf_use = ndf_use; 
	// NOTE(flo): Normal Distribution Function
	if(debug_collapse(debug, "NDF")) {
		debug_radio_button(debug, "NDF GGX", 0, &ndf_use);
		debug_radio_button(debug, "NDF Beckmann", 1, &ndf_use);
		debug_radio_button(debug, "NDF BlinnPhong", 2, &ndf_use);
		debug_radio_button(debug, "NDF Cosine", 3, &ndf_use);
		if(ndf_use != old_ndf_use) {
			if(ndf_use == 0) {
				g_brdf_d = normal_distribution_GGX;
			} else if(ndf_use == 1) {
				g_brdf_d = normal_distribution_beckmann; 
			} else if(ndf_use == 2) {
				g_brdf_d = normal_distribution_blinn_phong;
			} else if(ndf_use == 3) {
				g_brdf_d = cosine_distribution;
			}
			old_ndf_use = ndf_use;
		}
	}
	// NOTE(flo): Fresnel
	static b32 fresnel_wnd = true;
	static u32 fresnel_use = 0;
	u32 old_fresnel_use = fresnel_use; 
	if(debug_collapse(debug, "Fresnel")) {
		debug_radio_button(debug, "Fresnel Schlick", 0, &fresnel_use);
		debug_radio_button(debug, "Fresnel Gaussian", 1, &fresnel_use);
		debug_radio_button(debug, "Fresnel None", 2, &fresnel_use);
		if(fresnel_use != old_fresnel_use) {
			if(fresnel_use == 0) {
				g_brdf_f = fresnel_schlick_v3;
			} else if(fresnel_use == 1) {
				g_brdf_f = fresnel_spherical_gaussian_v3; 
			} else if(fresnel_use == 2) {
				g_brdf_f = fresnel_none_v3; 
			}
			old_fresnel_use = fresnel_use;
		}
	}
	// NOTE(flo): Geometry
	static b32 geo_wnd = true;
	static u32 geo_use = 0;
	u32 old_geo_use = geo_use; 
	if(debug_collapse(debug, "Geometry")) {
		debug_radio_button(debug, "Geometry Smith-GGX", 0, &geo_use);
		if(geo_use != old_geo_use) {
			if(geo_use == 0) {
				g_brdf_g = geometry_smith_ggx;
			}
			old_geo_use = geo_use;
		}
	}
	debug_end_wnd(debug);

	char debug_txt[128];
	static b32 light_wnd = true;
	debug_begin_wnd(debug, "Light", &light_wnd);
	static u32 light_use = (u32)emit->light.type;
	u32 old_light_use = light_use; 
	debug_radio_button(debug, "Directional", 0, &light_use);
	debug_radio_button(debug, "Point", 1, &light_use);
	kh_assert(light_use < LightType_count);
	emit->light.type = (LightType)light_use;
	kh_assert(emit->light.type != LightType_spot);
	// debug_color_hdr(debug, "color", &emit->light.color);
	debug_color(debug, "RGB color", &sample->light_rgb_color);
	debug_slider_f32(debug, "HDR intensity", 0.0f, 100.0f, &sample->light_rgb_intensity);
	emit->light.color = sample->light_rgb_color * sample->light_rgb_intensity;

	Material_F4 *light_mat_0 = get_meshr_material(render, emit->sphere_r, Material_F4);
	f32 max = kh_max_f32(kh_max_f32(emit->light.color.x, emit->light.color.y), emit->light.color.z);
	f32 r = emit->light.color.x / max;
	f32 g = emit->light.color.y / max;
	f32 b = emit->light.color.z / max;
	light_mat_0->color = kh_vec4(r, g, b, 1.0f);
	v3 L = (emit->light.type == LightType_directional) ? kh_normalize_v3(emit->light.dir) : kh_normalize_v3(emit->light.pos - receiver_pos);
	v3 V = kh_normalize_v3(render->camera.tr.pos - receiver_pos);
	kh_printf(debug_txt, "L : {%f,%f,%f}", L.x, L.y, L.z);
	debug_text(debug, debug_txt);
	kh_printf(debug_txt, "V : {%f,%f,%f}", V.x, V.y, V.z);
	debug_text(debug, debug_txt);
	debug_check_box(debug, "Show Axis", &sample->show_axis);
	debug_end_wnd(debug);

	static b32 show_main_wnd = true;
	debug_begin_wnd(debug, "Surface Parameters", &show_main_wnd);
	debug_slider_f32(debug, "roughness", 0.0f, 1.0f, &receiver->roughness);
	debug_slider_f32(debug, "metallic", 0.0f, 1.0f, &receiver->metallic);
	debug_slider_f32(debug, "ao", 0.0f, 1.0f, &receiver->ao);
	debug_color(debug, "albedo", &receiver->albedo);

	debug_radio_button(debug, "Sphere", 0, &sample->entity_to_use);
	debug_same_line(debug);
	debug_radio_button(debug, "Plane", 1, &sample->entity_to_use);
	debug_same_line(debug);
	debug_radio_button(debug, "Model", 2, &sample->entity_to_use);
	debug_end_wnd(debug);

	if(sample->entity_to_use == 0) {
		// brdf_sphere(sample, render, assets);
		push_render_entry(render, &sample->sphere);
	} else if(sample->entity_to_use == 1) {
		// brdf_plane(sample, render, assets);
		push_render_entry(render, &sample->plane);
	} else if(sample->entity_to_use == 2) {
		for(u32 i = 0; i < render->animators.count; ++i) {
			Animator *animator = render->animators.data + i;
			global_pose_generation(animator, &render->joint_tr, assets, dt);
		}
		for(u32 i = 0; i < render->animators.count; ++i) {
			Animator *animator = render->animators.data + i;
			matrix_palette_generation(animator, &render->joint_tr, assets);
		}
		push_render_entry(render, &sample->model);
	}
	if(emit->light.type == LightType_point) {
		push_render_entry(render, emit->tr, emit->sphere_r);
	} else if(emit->light.type == LightType_directional) {
		push_render_entry(render, emit->tr, emit->plane_r);
		v3 pos = kh_vec3(emit->tr.c3);
		v3 nor = -kh_vec3(emit->tr.c2);
		debug_line(debug, pos, pos + nor, kh_vec3(0,0,1));
	}
	render->lights[0] = emit->light;

	if(sample->show_axis) {
		debug_line(debug, kh_vec3(0,0,0), kh_vec3(1,0,0), kh_vec3(1,0,0));
		debug_line(debug, kh_vec3(0,0,0), kh_vec3(0,1,0), kh_vec3(0,1,0));
		debug_line(debug, kh_vec3(0,0,0), kh_vec3(0,0,1), kh_vec3(0,0,1));
	}

	f32 dx = 0.1f;
	f32 min_x = 0.0f;
	f32 max_x = PI32 * 0.5f;
	// debug_begin_2d_grid(debug, 0, 0, 100, 100, min_x, max_x, dx);
	// f32 old_y = normal_distribution_GGX(kh_cos_f32(min_x), receiver->roughness);
	// f32 old_x = min_x;
	// for(f32 x = (min_x + dx); x < max_x; ++x) {
	// 	f32 y = normal_distribution_GGX(kh_cos_f32(x), receiver->roughness);
	// 	debug_line(debug, kh_vec3(old_x, old_y, 0), kh_vec3(x, y, 0), kh_vec3(1,1,1));
	// 	old_y = y;
	// 	old_x = x;
	// 	x += dx;
	// }
	// debug_end_2d_grid(debug, min_y, max_y);

	edit_level(&sample->level_editor);

	debug_end_frame(debug);
	kh_end_transient(&f_arena);

	return(0);
}