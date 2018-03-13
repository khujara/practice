#include "kh/kh_platform.h"
#include "kh/kh_sdl.h"
KH_GLOBAL b32 g_isrunning = false;
Platform g_platform;

#ifdef KH_IN_DEVELOPMENT
KH_GLOBAL b32 g_view_debug;
KH_GLOBAL b32 g_ogl_wireframe;
#endif

#ifdef WIN32
// NOTE(flo): for opengl stuff
#include <windows.h>
#endif

#include "sdl_main.h"

#include "kh_asset_names.h"
#include "kh_material_names.h"
#include "kh_shading_names.h"
#include "kh_asset.h"
#include "kh_render.h"
#include "kh_debug.h"

#include "kh_asset.cpp"
#include "kh_asset_loader.cpp"

#include <gl/gl.h>
#include "kh/kh_ogl_ext.h"
#include "kh_renderer_ogl.h"

#include "kh_ogl_debug.h"
#include "kh_ogl_debug.cpp"

KH_INTERN void
sdl_reset_memory(ProgramMemory *mem, RenderManager *render, OglAPI *ogl) {
	if(mem->program_state) {
		LinearArena arena_copy = mem->program_state->arena;
		kh_clear(&arena_copy);
	}
	mem->program_state = 0;

	if(mem->frame_state) {
		LinearArena arena_copy = mem->frame_state->arena;
		kh_clear(&arena_copy);
	}
	mem->frame_state = 0;

	render->light_count = 0;
	render->has_skybox = false;
	ogl->reset(ogl);
}


KH_INTERN SampleLibrary
sdl_load_sample_library(int index) {
	SampleLibrary res = {};

	u32 ind = index - 1;
	PracticeSample sample = (PracticeSample)ind;

	char *dlls[] = {FOREACH_SAMPLE(GENERATE_DLL_NAMES)};
	char dll[1024];
	char *dll_name = dlls[ind];
	char *path = SDL_GetBasePath();
	u32 path_len = string_length(path);
	kh_assert(path_len < 1024);
	strings_copy(path_len, path, dll);
	strings_concat(string_length(dll), dll, string_length(dll_name), dll_name, sizeof(dll), dll);
	res.lib = SDL_LoadObject(dll);
	if(res.lib) {
		res.frame_update = (kh_update *)SDL_LoadFunction(res.lib, "frame_update");
		if(!res.frame_update) {
			// TODO(flo): error handling
		} else {
			res.is_loaded = true;
		}
	}	
	if(!res.is_loaded) {
		res.frame_update = 0;
	}
	return(res);
}

KH_INTERN void
sdl_unload_sample(SampleLibrary *sample) {
	if(sample->lib) {
		SDL_UnloadObject(sample->lib);
		sample->lib = 0;
	}
	sample->is_loaded = false;
	sample->frame_update = 0;
}

KH_INTERN void
sdl_init_debug(ProgramMemory *memory, Assets *assets) {
	memory->debug_state = debug_init(assets);
	DebugState *debug = memory->debug_state;
	kh_assert(debug);
	ImGuiIO &io = ImGui::GetIO();
	// debug->imctx = ImGui::GetCurrentContext(); 
	io.FontGlobalScale = 1.5f;

	// ImFont *font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\LiberationMono-Regular.ttf", 18.0f);
	ImFont *font = io.Fonts->AddFontFromFileTTF("data/LiberationMono-Regular.ttf", 18.0f);
	io.Fonts->GetTexDataAsAlpha8(&debug->imgui_font_px, &debug->imgui_font_w, &debug->imgui_font_h);

	u32 test = SDLK_PAGEDOWN;
	io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
	io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = SDLK_a;
	io.KeyMap[ImGuiKey_C] = SDLK_c;
	io.KeyMap[ImGuiKey_V] = SDLK_v;
	io.KeyMap[ImGuiKey_X] = SDLK_x;
	io.KeyMap[ImGuiKey_Y] = SDLK_y;
	io.KeyMap[ImGuiKey_Z] = SDLK_z;
}

KH_INTERN OglAPI_Version
sdl_opengl_init(SDL_Window *wnd) {
	OglAPI_Version res = OglAPI_Version_none;

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,  SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

	SDL_GLContext opengl_rc = SDL_GL_CreateContext(wnd);

	if(opengl_rc) {


		int min_ver,maj_ver;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj_ver);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min_ver);

		res = OglAPI_Version_4_5;

		load_gl_extensions();	
		SDL_GL_MakeCurrent(wnd, opengl_rc);

		SDL_GL_SetSwapInterval(1);
	}
	return(res);
}

KH_INTERN void
sdl_keyboard_button_event(ButtonState *state, b32 is_down) {
	if(state->down != is_down) {
		state->down = is_down;
		++state->down_count;
	}
}

KH_INTERN void
sdl_process_messages(SDLState *state, Input *input) {
	SDL_Event event;
	for(;;) {
		i32 pending_events = 0;

		pending_events = SDL_PollEvent(&event);

		if(!pending_events) break;

		switch(event.type) {
			case SDL_QUIT : {
				g_isrunning = false;
			} break;
			case SDL_KEYDOWN :
			case SDL_KEYUP : {
				SDL_Keycode code = event.key.keysym.sym;

				// input->modifier_down[Modifier_shift] = (event.key.keysim.mod & KMOD_SHIFT);
				// input->modifier_down[Modifier_alt] = (event.key.keysim.mod & KMOD_ALT);
				// input->modifier_down[Modifier_ctrl] = (event.key.keysim.mod & KMOD_CTRL);

				b32 is_pressed = (event.key.state == SDL_PRESSED);

				if(event.key.repeat == 0) {
					if((int)code < array_count(input->debug_buttons)) {
						sdl_keyboard_button_event(&input->debug_buttons[code], is_pressed);
					}
					if(code == SDLK_LEFT) {
						sdl_keyboard_button_event(&input->buttons[Button_move_left], is_pressed);
					} else if(code == SDLK_RIGHT) {
						sdl_keyboard_button_event(&input->buttons[Button_move_right], is_pressed);
					}else if(code == SDLK_UP) {
						sdl_keyboard_button_event(&input->buttons[Button_move_up], is_pressed);
					} else if(code == SDLK_DOWN) { 
						sdl_keyboard_button_event(&input->buttons[Button_move_down], is_pressed);
					} else if(code == SDLK_a) { 
						sdl_keyboard_button_event(&input->buttons[Button_action_left], is_pressed);
					} else if(code == SDLK_d) { 
						sdl_keyboard_button_event(&input->buttons[Button_action_right], is_pressed);
					} else if(code == SDLK_w) { 
						sdl_keyboard_button_event(&input->buttons[Button_action_up], is_pressed);
					} else if(code == SDLK_s) { 
						sdl_keyboard_button_event(&input->buttons[Button_action_down], is_pressed);
					}
					if(is_pressed) {

						if(code == SDLK_q) {
							state->fullscreen_flags = state->fullscreen_flags ^ SDL_WINDOW_FULLSCREEN_DESKTOP;
							SDL_SetWindowFullscreen(state->window, state->fullscreen_flags);
							// TODO(flo):fullscreen
						} else if(code == SDLK_s) {
							// TODO(flo): switch software/hardware renderer
						}else if(code == SDLK_F1) {
							state->new_sample = Sample_basic_scene;
						} else if(code == SDLK_F2) {
							state->new_sample = Sample_basic_mesh;
						} else if(code == SDLK_F3) {
							state->new_sample = Sample_basic_animation;
						} else if(code == SDLK_F4) {
							state->new_sample = Sample_scene_pathtracer;
						} else if(code == SDLK_F5) {
							state->new_sample = Sample_radiance_transfer_view;
						} else if(code == SDLK_ESCAPE) {
							g_isrunning = false;
						} else if(code == SDLK_d) {
							g_view_debug = !g_view_debug;
						} else if(code == SDLK_w) {
							g_ogl_wireframe = !g_ogl_wireframe;
						}
					}
				}
			} break;
			case SDL_MOUSEWHEEL : {
				input->dt_wheel = event.wheel.y * 120;
			} break;
		}
	}
}

int main(int arg_c, char **args) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
	// SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *wnd = SDL_CreateWindow("SDL Practice",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          720,
										  SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);	
    if(wnd) {

    	SDLState state = {};
    	state.window = wnd;
    	state.fullscreen_flags = 0;
    	state.renderer = RendererType_opengl;
    	state.new_sample = Sample_basic_scene;
    	state.cur_sample = Sample_none;
		SampleLibrary sample = {};

		const u32 high_thread_count = 7;
		WorkQueue high_queue = {};
		// TODO(flo): do not like this!
		SDLSchedHandle high_handle = sdl_create_thread_handle(high_thread_count);
		sdl_create_sched_queue(&high_queue, &high_handle, high_thread_count);

		u32 low_thread_count = 1;
		WorkQueue low_queue = {};
		// TODO(flo): do not like this!
		SDLSchedHandle low_handle = sdl_create_thread_handle(low_thread_count);
		sdl_create_sched_queue(&low_queue, &low_handle, low_thread_count);

    	ProgramMemory memory = {};
    	memory.high_queue = &high_queue;
    	memory.low_queue = &low_queue;
    	sdl_set_platform(&memory.platform);
    	g_platform = memory.platform;

    	// generate_material_header("materials.mat", "kh_material_names.h");


    	Input inputs[2] = {};
    	Input *input_cur_frame = &inputs[0];
    	Input *input_last_frame = &inputs[1];

		RenderManager render = {};
		// render.height        = 720;
		render.width         = 1920;
		render.height        = 1080;
		render.mat_buffer_count = 0;
		render.mat_buffer_max_count = 128;
		render.mat_buffer = (MaterialHashEl *)calloc(1, render.mat_buffer_max_count * sizeof(MaterialHashEl));
		render.commands_size = kilobytes(4);
		render.commands      = (u8 *)calloc(1, render.commands_size);
		render.animators.max_count = 64;
		render.animators.count = 0;
		render.animators.data = (Animator *)calloc(1, render.animators.max_count * sizeof(Animator));
		render.joint_tr.max_count = 1024;
		render.joint_tr.count = 0;
		render.joint_tr.data = (mat4 *)calloc(1, render.joint_tr.max_count * sizeof(mat4));
		render.light_count = 0;
		render.max_entries = 256;
		render.render_entries = (RenderEntry *)calloc(1, render.max_entries * sizeof(RenderEntry));
		Assets *assets = load_assets_infos("data/datas.khjr", memory.low_queue, megabytes(256), 32);
		load_materials(assets, &render, "data/materials.mat");

    	OglAPI_Version ogl_ver = sdl_opengl_init(wnd);
		OglAPI ogl = {};
		LinearArena ogl_arena = {};
		ogl.arena = &ogl_arena;
		ogl_set_api_version(ogl_ver, &ogl);
		ogl.init(&ogl, &render, assets);
		sdl_init_debug(&memory, assets);

		OglDebugState ogl_debug;
		ogl_debug_init(&ogl_debug, &ogl, &render);
		g_view_debug = false;


    	g_isrunning = true;
    	float dt = 1.0f / 60.0f;
		while(g_isrunning)
		{
			input_cur_frame->dt_wheel = 0;

			kh_lu0(button_i, Button_count) {
				input_cur_frame->buttons[button_i].down_count = 0;
				input_cur_frame->buttons[button_i].down = input_last_frame->buttons[button_i].down;
			}
			kh_lu0(button_i, array_count(input_cur_frame->debug_buttons)) {
				input_cur_frame->debug_buttons[button_i].down_count = 0;
				input_cur_frame->debug_buttons[button_i].down = input_last_frame->debug_buttons[button_i].down;
			}

			sdl_process_messages(&state, input_cur_frame);

			i32 dim_w, dim_h;
			SDL_GetWindowSize(wnd, &dim_w, &dim_h);

			SDL_Point mouse_p;
			u32 mouse_state = SDL_GetMouseState(&mouse_p.x, &mouse_p.y);
			f32 mouse_p_x = (f32)mouse_p.x;
			f32 mouse_p_y = (f32)((dim_h - 1) - mouse_p.y);

			input_cur_frame->delta_mouse_x = mouse_p_x - input_cur_frame->mouse_x; 
			input_cur_frame->delta_mouse_y = mouse_p_y - input_cur_frame->mouse_y; 

			input_cur_frame->mouse_x = mouse_p_x;
			input_cur_frame->mouse_y = mouse_p_y;

			f32 aspect_ratio = (f32)render.width / (f32)render.height;
			u32 new_w = (u32)((f32)dim_h * aspect_ratio);
			u32 pad_w = (dim_w - new_w) / 2;
			u32 pad_h = 0;
			u32 new_h = dim_h;
			if(new_w > (u32)dim_w) {
				f32 inv_ar = (f32)render.height / (f32)render.width;
				new_w = dim_w;
				pad_w = 0;
				new_h = (u32)((f32)dim_w * inv_ar);
				kh_assert(new_h <= (u32)dim_h);
				pad_h = (dim_h - new_h) / 2;
			}
			i32 half_diff_w = (i32)(new_w - dim_w) / 2;
			i32 half_diff_h = (i32)(new_h - dim_h) / 2;
			input_cur_frame->mouse_rx = kh_clamp_f32(0, new_w, mouse_p_x + half_diff_w)/(f32)new_w;
			input_cur_frame->mouse_ry = kh_clamp_f32(0, new_h, mouse_p_y + half_diff_h)/(f32)new_h;

			SDL_Keymod key_mod = SDL_GetModState();
			input_cur_frame->modifier_down[Modifier_shift] = (key_mod & KMOD_SHIFT);
			input_cur_frame->modifier_down[Modifier_alt] = (key_mod & KMOD_ALT);
			input_cur_frame->modifier_down[Modifier_ctrl] = (key_mod & KMOD_CTRL);

			u32 sdl_mouse_id[MouseButton_count] = {
				SDL_BUTTON_LMASK,
				SDL_BUTTON_RMASK,
				SDL_BUTTON_MMASK,
				SDL_BUTTON_X1MASK,
				SDL_BUTTON_X2MASK,
			};

			for(u32 b_i = 0; b_i < MouseButton_count; ++b_i) 				{
				input_cur_frame->mouse_buttons[b_i] = input_last_frame->mouse_buttons[b_i];
				input_cur_frame->mouse_buttons[b_i].down_count = 0;
				sdl_keyboard_button_event(&input_cur_frame->mouse_buttons[b_i], mouse_state & sdl_mouse_id[b_i]);
			}

			if(memory.debug_state) {
				memory.debug_state->input = input_cur_frame;
			}

			if(state.new_sample > 0 && state.new_sample != state.cur_sample) {
				sdl_complete_all_queue_works(&low_queue);
				sdl_complete_all_queue_works(&high_queue);
				sdl_unload_sample(&sample);
				for(u32 i = 0; i < 50 && !sample.is_loaded; ++i) {
					sample = sdl_load_sample_library(state.new_sample);
					SDL_Delay(100);
				}
				kh_assert(sample.is_loaded);
				if(state.cur_sample > 0) {
					sdl_reset_memory(&memory, &render, &ogl);
				}
				state.cur_sample = state.new_sample;
				state.new_sample = Sample_none;
			}

			render.wnd_w = dim_w;
			render.wnd_h = dim_h; 

			if(memory.debug_state) {
				memory.debug_state->show = g_view_debug;
			}

			if(sample.frame_update) {
				sample.frame_update(&memory, input_cur_frame, &render, assets, dt);
			}

			ogl.wireframe = g_ogl_wireframe;
			ogl.render(&ogl, &render, assets);
			ogl.display(&ogl, dim_w, dim_h, 0, true);
			if(memory.debug_state && memory.debug_state->show) {
				ogl_debug_draw(&ogl_debug, memory.debug_state, &render, true);
			}
			SDL_GL_SwapWindow(wnd);

			Input *tmp = input_cur_frame;
			input_cur_frame = input_last_frame;
			input_last_frame = tmp;

		}
    }
    SDL_DestroyWindow(wnd);
    SDL_Quit();
    return(0);
}