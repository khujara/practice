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
	ProgramState *program_state = memory->program_state;
	ProgramSample *sample = (ProgramSample *)program_state;
	if(!program_state)
	{
		program_state = memory->program_state = boot_strap_push_struct(ProgramState, arena);
		sample = (ProgramSample *)program_state;

		init_render(render);

		f32 w = (f32)render->width;
		f32 h = (f32)render->height;
		render->camera = perspective_camera(w, h, kh_vec3(0.0f,5.0f,4.0f), kh_vec3(0,1.0f,0), CullingMask_3D_default, COLOR_RED);

		Camera *cam = &render->camera;
		lookat(cam, kh_vec3(-1.5f, 1.5f, -2.5f), kh_vec3(0,1,0), kh_vec3(0,1,0));
	}

	FrameState *frame_state = memory->frame_state;
	if(!frame_state)
	{
		frame_state = memory->frame_state = boot_strap_push_struct(FrameState, arena);
		frame_state->asset_arr = assets;//alloc_assets_and_load_infos(&memory->assets, memory->high_queue, megabytes(64));
	}

	DebugState *debug_state = memory->debug_state;
	if(!debug_state) {
		debug_state = memory->debug_state = debug_init(assets);
	}

	TransientStack f_stack = kh_begin_transient(&frame_state->allocator);
	begin_render_frame(render, assets);
	debug_begin_frame(debug_state);

	default_camera_move(&render->camera, input, dt);

	set_view_matrix(cam);

	kh_end_transient(&f_stack);

	return(0);

}