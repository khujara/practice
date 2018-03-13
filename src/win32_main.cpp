#include "kh/kh_platform.h"
KH_GLOBAL b32 global_isrunning;
Platform g_platform;

#ifdef KH_IN_DEVELOPMENT
KH_GLOBAL b32 g_view_debug;
KH_GLOBAL b32 g_ogl_wireframe;
#endif

#include <windows.h>
#include "kh\kh_win32.h"

#include <xinput.h>
#include "win32_main.h"

#include "kh_asset_names.h"
#include "kh_material_names.h"
#include "kh_shading_names.h"
#include "kh_asset.h"
#include "kh_render.h"
#include "kh_debug.h"

#include "kh_asset.cpp"
#include "kh_asset_loader.cpp"

#include "kh_renderer_software.h"
#include "kh_rasterizer_software.cpp"
#include "kh_renderer_software.cpp"

#include <gl/gl.h>
#include "kh/kh_ogl_ext.h"
#include "kh_renderer_ogl.h"

#ifdef KH_IN_DEVELOPMENT
#include "kh_ogl_debug.h"
#include "kh_ogl_debug.cpp"
#endif

// #define USE_RENDERDOC
#ifdef USE_RENDERDOC
#include "app/renderdoc_app.h"
RENDERDOC_API_1_1_1 g_renderdoc;
KH_INTERN RENDERDOC_API_1_1_1
win32_load_renderdoc(char *renderdoc_dll_full_path) {
	RENDERDOC_API_1_1_1 res = {};
	HMODULE lib = LoadLibrary(renderdoc_dll_full_path);	
	if(lib) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(lib, "RENDERDOC_GetAPI");

		RENDERDOC_API_1_1_1 *rdoc_api = 0;
		int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_1, (void **)&rdoc_api);
		res = *rdoc_api;
		kh_assert(ret == 1);
		kh_assert(res.StartFrameCapture != NULL && res.EndFrameCapture != NULL);
	}
	return(res);
}
#endif

KH_INTERN void
win32_init_debug(ProgramMemory *memory, Assets *assets) {
	memory->debug_state = debug_init(assets);
	DebugState *debug = memory->debug_state;
	kh_assert(debug);
	ImGuiIO &io = ImGui::GetIO();
	io.FontGlobalScale = 1.5f;

	ImFont *font = io.Fonts->AddFontFromFileTTF("data/LiberationMono-Regular.ttf", 18.0f);
	io.Fonts->GetTexDataAsAlpha8(&debug->imgui_font_px, &debug->imgui_font_w, &debug->imgui_font_h);

	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';
}


LRESULT WINAPI
win32_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	LRESULT res = 0;
	switch(message)	{
		case WM_CLOSE : {
			global_isrunning = false;
		} break;
		case WM_DESTROY : {
			global_isrunning = false;
		} break;
		case WM_PAINT : {
			PAINTSTRUCT paint;
			HDC device_ctx_hdl = BeginPaint(window, &paint);
			EndPaint(window, &paint);
		} break;
		default : {
			res = DefWindowProcA(window, message, w_param, l_param);
		} break;
	}
	return(res);
}

KH_INTERN SoftwareFrameBuffer
win32_intialize_back_buffer(RenderManager *render) {
	SoftwareFrameBuffer buffer = {};
	buffer.pixels.w = render->width;
	buffer.pixels.h = render->height;
	buffer.pixels.pitch = KH_ALIGN16(buffer.pixels.w * FRAME_BUFFER_BYTES_PER_PIXEL);
	i32 bmp_memory_size = buffer.pixels.pitch * buffer.pixels.h;
	u32 depth_size = render->width * render->height * sizeof(f32);

	u32 size = KH_ALIGN16(bmp_memory_size + depth_size);
	u8 *mem = (u8 *)VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	buffer.zbuffer.memory = (f32 *)mem;
	buffer.pixels.memory = mem + depth_size;

	return(buffer);
}

KH_INTERN void
win32_keyboard_button_event(ButtonState *state, b32 is_down) {
	if(state->down != is_down) {
		state->down = is_down;
		++state->down_count;
	}
}

KH_INTERN void
win32_process_messages(Win32State *state, Input *input) {
	MSG win32_mes;
    BYTE keystate[256]={0};
	for(;;)	{
		BOOL got_mes = FALSE;

		{
			got_mes = PeekMessage(&win32_mes, 0, 0, 0, PM_REMOVE);
		}
		
		if(!got_mes) break;

		switch(win32_mes.message) {
			case WM_QUIT : {
				global_isrunning = false;
			} break;
			case WM_SYSKEYDOWN :
			case WM_SYSKEYUP :
			case WM_KEYDOWN :
			case WM_KEYUP : {
				u32 code = (u32)win32_mes.wParam;

				b32 was_pressed = ((win32_mes.lParam & (1 << 30)) != 0);
				b32 is_pressed = ((win32_mes.lParam & (1 << 31)) == 0);

				if(was_pressed != is_pressed) {
					u16 ascii_code;
					GetKeyboardState((PBYTE)keystate);
					b32 translated = ToAscii(win32_mes.wParam, MapVirtualKey(win32_mes.wParam, 0), keystate, &ascii_code, 0);
					if(ascii_code < array_count(input->debug_buttons) && translated == 1) {
						win32_keyboard_button_event(&input->debug_buttons[ascii_code], is_pressed);
					}

					if(code == VK_LEFT) {
						win32_keyboard_button_event(&input->buttons[Button_move_left], is_pressed);
					} else if(code == VK_RIGHT) {
						win32_keyboard_button_event(&input->buttons[Button_move_right], is_pressed);
					}
					else if(code == VK_UP) {
						win32_keyboard_button_event(&input->buttons[Button_move_up], is_pressed);
					}
					else if(code == VK_DOWN) { 
						win32_keyboard_button_event(&input->buttons[Button_move_down], is_pressed);
					}
					else if(code == 'a') { 
						win32_keyboard_button_event(&input->buttons[Button_action_left], is_pressed);
					}
					else if(code == 'd') { 
						win32_keyboard_button_event(&input->buttons[Button_action_right], is_pressed);
					}
					else if(code == 'w') { 
						win32_keyboard_button_event(&input->buttons[Button_action_up], is_pressed);
					}
					else if(code == 's') { 
						win32_keyboard_button_event(&input->buttons[Button_action_down], is_pressed);
					}
					if(is_pressed)
					{
						if(code == 'Q') { 
							win32_toggle_full_screen(win32_mes.hwnd, &state->window_pos); 
						} else if(code == 'S') { 
							// TODO(flo) we will need a gui for this
							if(state->renderer == RendererType_software) {
								state->renderer = RendererType_opengl;
							} else {
								state->renderer = RendererType_software;
							}
						}
						else if(code == VK_F1) {
							state->new_sample = Sample_basic_scene;
						} else if(code == VK_F2) {
							state->new_sample = Sample_basic_mesh;
						} else if(code == VK_F3) {
							state->new_sample = Sample_basic_animation;
						} else if(code == VK_F4) {
							state->new_sample = Sample_scene_pathtracer;
						} else if(code == VK_F5) {
							state->new_sample = Sample_radiance_transfer_view;
						} else if(code == VK_ESCAPE) { 
							global_isrunning = false; 
						}
#if KH_IN_DEVELOPMENT
						else if(code == 'D') {
							g_view_debug = !g_view_debug;
						} else if(code == 'W') {
							g_ogl_wireframe = !g_ogl_wireframe;
						} else if(code == VK_SNAPSHOT) {
#ifdef USE_RENDERDOC
							g_renderdoc.TriggerCapture();
#endif
						}
#endif
					}
				}
			} break;
			case WM_MOUSEWHEEL : {
				input->dt_wheel = (i32)GET_WHEEL_DELTA_WPARAM(win32_mes.wParam);
			} break;
			default : {
				TranslateMessage(&win32_mes);
				DispatchMessage(&win32_mes);
			} break;
		}
	}
}

KH_INTERN void
win32_load_d3d_library() {
	HMODULE lib;
	lib = LoadLibraryA("D3d12.dll");
	if(!lib) {
		lib = LoadLibraryA("D3D11.dll");
	}
	if(!lib) {
		lib = LoadLibraryA("D3D10.dll");
	}
	if(!lib) {
		lib = LoadLibraryA("D3D9.dll"); 
	}
	kh_assert(lib);
}

KH_INTERN b32
win32_load_vulkan_library() {
	b32 res = false;
	HMODULE lib;
	lib = LoadLibraryA("vulkan-1.dll");
	if(lib) res = true;
	return(res);
}

KH_INTERN Win32SampleLibrary
win32_load_sample_library(int index) {
	Win32SampleLibrary res = {};

	u32 ind = index - 1;
	PracticeSample sample = (PracticeSample)ind;

	char *dlls[] = {FOREACH_SAMPLE(GENERATE_DLL_NAMES)};
	char dll[1024];
	char *dll_name = dlls[ind];
	ProgramPath exe_path = win32_get_program_path();
	kh_assert(exe_path.path_len < 1024);
	strings_copy(exe_path.path_len, exe_path.path, dll);
	strings_concat(string_length(dll), dll, string_length(dll_name), dll_name, sizeof(dll), dll);
	res.lib = LoadLibrary(dll);
	if(res.lib)	{
		res.frame_update = (kh_update *)GetProcAddress(res.lib, "frame_update");
		if(!res.frame_update) {
	      // TODO(flo) : error handling
		}
		else {
			res.is_loaded = true;
		}
		// res.is_loaded = (res.frame_update);
	}
	if(!res.is_loaded) {
		res.frame_update = 0;
	}
	return(res);
}

KH_INTERN void
win32_unload_sample(Win32SampleLibrary *sample) {
	if(sample->lib) {
		FreeLibrary(sample->lib);
		sample->lib = 0;
	}
	sample->is_loaded = false;
	sample->frame_update = 0;
}

KH_INTERN OglAPI_Version
win32_ogl_init(HWND window) {
	OglAPI_Version res = OglAPI_Version_none;
	HDC wnd_dc = GetDC(window);

	PIXELFORMATDESCRIPTOR pf_desc = {};
	pf_desc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pf_desc.nVersion = 1;
	pf_desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pf_desc.iPixelType = PFD_TYPE_RGBA;
	pf_desc.cColorBits = 32;
	pf_desc.cAlphaBits = 8;
	pf_desc.iLayerType = PFD_MAIN_PLANE;

	int win32_suggested_pf_ind = ChoosePixelFormat(wnd_dc, &pf_desc);
	PIXELFORMATDESCRIPTOR win32_pf_desc;
	DescribePixelFormat(wnd_dc, win32_suggested_pf_ind, sizeof(win32_pf_desc), &win32_pf_desc);
	SetPixelFormat(wnd_dc, win32_suggested_pf_ind, &pf_desc);

	// TODO(flo): check if we can retrieve an opengl version that we handle if not return OglAPI_Version_none
	HGLRC opengl_rc = wglCreateContext(wnd_dc);
	b32 make_cur = wglMakeCurrent(wnd_dc, opengl_rc);

	win32_load_wgl_extensions();

	/* @NOTE(flo) : If you wish, you may delete the first context. Ultimately, though, 
	you will always end up creating at least two contexts in any new application that uses a 
	core profile context or needs debugging features.
	*/

	wglDeleteContext(opengl_rc);

	int context_attribs_45[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		// WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB|WGL_CONTEXT_DEBUG_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		// WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0,
	};


	HGLRC modern_opengl_rc = wglCreateContextAttribsARB(wnd_dc, 0, context_attribs_45);
	make_cur = wglMakeCurrent(wnd_dc, modern_opengl_rc);
	res = OglAPI_Version_4_5;

	HGLRC cur_ogl_rc = wglGetCurrentContext();
	kh_assert(cur_ogl_rc == modern_opengl_rc);

	load_gl_extensions();
	kh_assert(wglSwapIntervalEXT);
	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1);
	}
	else
	{
		kh_assert(!"no swap interval");
	}

	ReleaseDC(window, wnd_dc);

	// TODO(flo): use this for recent OpenGL
	// wglChoosePixelFormatARB(wnd_dc, attrib_list, NULL, 1, &pixel_format, &num_formats);
	return(res);
}

KH_INTERN void
win32_reset_memory(ProgramMemory *mem, RenderManager *render, OglAPI *ogl) {
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

#ifdef KH_IN_DEVELOPMENT
KH_INTERN void
win32_load_asset_import_library(ProgramMemory *prog) {
	char *dll_name = "kh_asset_import.dll";
	char dll[1024];
	ProgramPath exe_path = win32_get_program_path();
	strings_copy(exe_path.path_len, exe_path.path, dll);
	strings_concat(string_length(dll), dll, string_length(dll_name), dll_name, sizeof(dll), dll);
	HMODULE lib = LoadLibrary(dll);
	if(lib) {
		prog->platform.load_tex2d_directly = (LoadTex2dFile *)GetProcAddress(lib, "load_tex2d_file");
		kh_assert(prog->platform.load_tex2d_directly);
		prog->platform.load_font_directly = (LoadFontFile *)GetProcAddress(lib, "load_font_file");
		kh_assert(prog->platform.load_font_directly);
		prog->platform.load_trimesh_directly = (LoadTriMeshFile *)GetProcAddress(lib, "load_trimesh_file");
		kh_assert(prog->platform.load_trimesh_directly);
		prog->platform.load_skeleton_directly = (LoadSkeletonFile *)GetProcAddress(lib, "load_skeleton_file");
		kh_assert(prog->platform.load_skeleton_directly);
		prog->platform.load_skin_directly = (LoadSkinFile *)GetProcAddress(lib, "load_skin_file");
		kh_assert(prog->platform.load_skin_directly);
		prog->platform.load_animation_directly = (LoadAnimationFile *)GetProcAddress(lib, "load_animation_file");
		kh_assert(prog->platform.load_animation_directly);
		prog->platform.load_animation_for_skin_directly = (LoadAnimationForSkinFiles *)GetProcAddress(lib, "load_animation_for_skin_files");
		kh_assert(prog->platform.load_animation_for_skin_directly);
		prog->platform.init_asset_import = (InitAssetLoader *)GetProcAddress(lib, "init_asset_loader");
		kh_assert(prog->platform.init_asset_import);
		prog->platform.init_asset_import(&prog->platform);
	}
}
#endif

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show) {
	// TODO(flo): pass the dll here!
	LARGE_INTEGER perf_counter_freq;
	QueryPerformanceFrequency(&perf_counter_freq);
	f64 inv_perf_counter_freq = 1.0f / (f64)perf_counter_freq.QuadPart;

	WNDCLASS wnd_class = {};
	wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	wnd_class.lpfnWndProc = win32_window_callback;
	wnd_class.hInstance = instance;
	wnd_class.hIcon = LoadIcon(0, IDI_APPLICATION);
	wnd_class.hCursor = LoadCursor(0, IDC_ARROW);
	wnd_class.lpszMenuName =  MAKEINTRESOURCE(3); 
	wnd_class.lpszClassName = "Practice";
	const int start_w = 1280;
	const int start_h = 720;
	if(RegisterClassA(&wnd_class)) {
		HWND window = CreateWindowExA(0, wnd_class.lpszClassName, "Win32 Practice", 
		                              WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
		                              CW_USEDEFAULT, CW_USEDEFAULT, start_w, start_h, 0, 0, instance, 0);
		if(window) {
			// g_renderdoc = win32_load_renderdoc("D:\\Dropbox\\work\\tools\\renderdoc_d.dll");
			// g_renderdoc.TriggerCapture()2018,12 March : ;
			// rdoc.TriggerCapture();

			Win32State state = {};
			state.window_pos = { sizeof(state.window_pos) };
			state.renderer = RendererType_opengl;
			// state.renderer = RendererType_software;
			state.display = DisplayType_opengl;
			// state.new_sample = Sample_basic_animation;
			// state.new_sample = Sample_basic_scene;
			// state.new_sample = Sample_basic_mesh;
			state.new_sample = Sample_scene_pathtracer;
			// state.new_sample = Sample_radiance_transfer_view;
			state.cur_sample = Sample_none;

			Win32SampleLibrary sample = {};

			const u32 high_thread_count = 7;
			WorkQueue high_queue = {};
			// TODO(flo): do not like this!
			Win32SchedHandle high_handle = win32_create_thread_handle(high_thread_count);
			win32_create_sched_queue(&high_queue, &high_handle, high_thread_count);

			u32 low_thread_count = 1;
			WorkQueue low_queue = {};
			// TODO(flo): do not like this!
			Win32SchedHandle low_handle = win32_create_thread_handle(low_thread_count);
			win32_create_sched_queue(&low_queue, &low_handle, low_thread_count);

			// win32_toggle_full_screen(window, &state.window_pos);

			u32 monitor_refresh_freq = 60;
			HDC refresh_dc = GetDC(window);
			i32 win32_refresh_rate = GetDeviceCaps(refresh_dc, VREFRESH);
			ReleaseDC(window, refresh_dc);
			if(win32_refresh_rate > 0) {
				monitor_refresh_freq = win32_refresh_rate;
			}
			f32 update_freq = (f32)monitor_refresh_freq;
			f32 target_spf = 1.0f / update_freq;

			win32_init_allocator_sentinel();

			ProgramMemory memory = {};
			memory.high_queue = &high_queue;
			memory.low_queue = &low_queue;

			// memory.platform.get_program_path = win32_get_program_path;
			win32_set_platform(&memory.platform);
			RECT desk_rect;
			HWND desktop = GetDesktopWindow();
			GetWindowRect(desktop, &desk_rect);

			#ifdef KH_IN_DEVELOPMENT
			win32_load_asset_import_library(&memory);
			#endif
			g_platform = memory.platform;

			OglAPI_Version ogl_ver = win32_ogl_init(window);

			global_isrunning = true;
			f32 old_mouse_x = 0;
			f32 old_mouse_y = 0;

			Input inputs[2] = {};
			Input *input_cur_frame = &inputs[0];
			Input *input_last_frame = &inputs[1];

			RenderManager render = {};
			// render.width         = 800;
			// render.height        = 450;
			// render.width         = 1280;
			// render.height        = 720;
			render.width         = 1920;
			render.height        = 1080;
			render.mat_buffer_count = 0;
			render.mat_buffer_max_count = 128;
			render.mat_buffer = (MaterialHashEl *)VirtualAlloc(0, render.mat_buffer_max_count * sizeof(MaterialHashEl), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.commands_size = kilobytes(4);
			render.commands      = (u8 *)VirtualAlloc(0, render.commands_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.animators.max_count = 64;
			render.animators.count = 0;
			render.animators.data = (Animator *)VirtualAlloc(0, render.animators.max_count * sizeof(Animator), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.joint_tr.max_count = 1024;
			render.joint_tr.count = 0;
			render.joint_tr.data = (mat4 *)VirtualAlloc(0, render.joint_tr.max_count * sizeof(mat4), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.light_count = 0;
			render.max_entries = 256;
			render.render_entries = (RenderEntry *)VirtualAlloc(0, render.max_entries * sizeof(RenderEntry), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

			// TODO(flo): since our assets system is always loaded in the platform layer
			// i do not think we have to return a pointer here except for stack purposes
			Assets *assets = load_assets_infos("data/datas.khjr", memory.low_queue, megabytes(256), 32);
			load_materials(assets, &render, "data/materials.mat");

			OglAPI ogl = {};
			LinearArena arena = {};
			ogl.arena = &arena;
			// TODO(flo): if we retrieve a non opengl version use software renderer
			ogl_set_api_version(ogl_ver, &ogl);
			if(ogl.init) {
				ogl.init(&ogl, &render, assets);
			} else {
				state.renderer = RendererType_software;
				state.display = DisplayType_software;
			}

			SoftwareFrameBuffer back_buff = win32_intialize_back_buffer(&render);
			win32_init_debug(&memory, assets);
#if KH_IN_DEVELOPMENT
			OglDebugState ogl_debug;
			ogl_debug_init(&ogl_debug, &ogl, &render);
			g_view_debug = false;
#endif

			// TODO(flo) IMPORTANT(flo) CLEANUP(flo) : find another way to handle multithreading in our software renderer! 
			const u32 work_count = 4096;
			TriMeshRenderWork works[work_count];
			f32 min_clip_x = 0.0f;
			f32 max_clip_x = (f32)render.width;

			f32 min_clip_y = 0.0f;
			f32 max_clip_y = (f32)render.height * 0.25f;
			f32 clip_pitch = (f32)render.height * 0.25f;

			for(u32 work_i = 0; work_i < work_count; ++work_i) {
				TriMeshRenderWork *work = works + work_i;
				work->target = &back_buff;
				work->min_x = min_clip_x;
				work->max_x = max_clip_x;
				work->min_y = min_clip_y;
				work->max_y = max_clip_y;
				work->assets = assets;
				min_clip_y += clip_pitch;
				max_clip_y += clip_pitch;

				if((work_i + 1) % 4 == 0) {
					min_clip_y = 0.0f;
					max_clip_y = (f32)render.height * 0.25f;
				}
			}
			while(global_isrunning)
			{
				Win32DebugTimer frame_time("-----------------------------START FRAME----------------------- \n");

				f32 dt = target_spf;
				input_cur_frame->dt_wheel = 0;

				kh_lu0(button_i, Button_count) {
					input_cur_frame->buttons[button_i].down_count = 0;
					input_cur_frame->buttons[button_i].down = input_last_frame->buttons[button_i].down;
				}
				kh_lu0(button_i, array_count(input_cur_frame->debug_buttons)) {
					input_cur_frame->debug_buttons[button_i].down_count = 0;
					input_cur_frame->debug_buttons[button_i].down = input_last_frame->debug_buttons[button_i].down;
				}

				win32_process_messages(&state, input_cur_frame);

			    // u64 end = __rdtsc();
			    // u64 elapsed = end - begin;
			    // char text_buff[256];
			    // _snprintf_s(text_buff, sizeof(text_buff), "win32 peek message cycle : %llu \n", elapsed);
			    // OutputDebugStringA(text_buff);

				POINT mouse_p;
				GetCursorPos(&mouse_p);
				ScreenToClient(window, &mouse_p);

				Win32WndDim dim = win32_get_wnd_dim(window);
				f32 mouse_p_x = (f32)mouse_p.x;
				f32 mouse_p_y = (f32)((dim.h - 1) - mouse_p.y);

				input_cur_frame->delta_mouse_x = mouse_p_x - input_cur_frame->mouse_x; 
				input_cur_frame->delta_mouse_y = mouse_p_y - input_cur_frame->mouse_y; 

				input_cur_frame->mouse_x = mouse_p_x;
				input_cur_frame->mouse_y = mouse_p_y;

				f32 aspect_ratio = (f32)render.width / (f32)render.height;
			    u32 new_w = (u32)((f32)dim.h * aspect_ratio);
			    u32 pad_w = (dim.w - new_w) / 2;
			    u32 pad_h = 0;
			    u32 new_h = dim.h;
			    if(new_w > (u32)dim.w) {
			    	f32 inv_ar = (f32)render.height / (f32)render.width;
			    	new_w = dim.w;
			    	pad_w = 0;
			    	new_h = (u32)((f32)dim.w * inv_ar);
			    	kh_assert(new_h <= (u32)dim.h);
			    	pad_h = (dim.h - new_h) / 2;
			    }
			    i32 half_diff_w = (i32)(new_w - dim.w) / 2;
			    i32 half_diff_h = (i32)(new_h - dim.h) / 2;
				input_cur_frame->mouse_rx = kh_clamp_f32(0, new_w, mouse_p_x + half_diff_w)/(f32)new_w;
				input_cur_frame->mouse_ry = kh_clamp_f32(0, new_h, mouse_p_y + half_diff_h)/(f32)new_h;

				input_cur_frame->modifier_down[Modifier_shift] = (GetKeyState(VK_SHIFT) & (1 << 15));
				input_cur_frame->modifier_down[Modifier_alt] = (GetKeyState(VK_MENU) & (1 << 15));
				input_cur_frame->modifier_down[Modifier_ctrl] = (GetKeyState(VK_CONTROL) & (1 << 15));

				DWORD win_mouse_id[MouseButton_count] = {
					VK_LBUTTON,
					VK_RBUTTON,
					VK_MBUTTON,
					VK_XBUTTON1,
					VK_XBUTTON2,
				};

				for(u32 b_i = 0; b_i < MouseButton_count; ++b_i) 				{
					input_cur_frame->mouse_buttons[b_i] = input_last_frame->mouse_buttons[b_i];
					input_cur_frame->mouse_buttons[b_i].down_count = 0;
					win32_keyboard_button_event(&input_cur_frame->mouse_buttons[b_i], (GetKeyState(win_mouse_id[b_i]) & (1 << 15)));
				}

				if(memory.debug_state) {
					memory.debug_state->input = input_cur_frame;
				}

				if(state.new_sample > 0 && state.new_sample != state.cur_sample) {
					win32_complete_all_queue_works(&low_queue);
					win32_complete_all_queue_works(&high_queue);
					win32_unload_sample(&sample);

					for(u32 i = 0; i < 50 && !sample.is_loaded; ++i) {
						sample = win32_load_sample_library(state.new_sample);
						Sleep(100);
					}
					kh_assert(sample.is_loaded);
					if(state.cur_sample > 0) {
						win32_reset_memory(&memory, &render, &ogl);
					}
					state.cur_sample = state.new_sample;
					state.new_sample = Sample_none;
				}


				render.wnd_w = dim.w;
				render.wnd_h = dim.h; 

				if(memory.debug_state) {
					memory.debug_state->show = g_view_debug;
				}

				if(sample.frame_update) {
					sample.frame_update(&memory, input_cur_frame, &render, assets, dt);
				}

				b32 blit = false;
				switch(state.renderer) {
					case RendererType_software : {
						software_render(&back_buff, assets, &render, &high_queue, works, work_count);
					} break;
					case RendererType_opengl : {
						blit = true;
#if KH_IN_DEVELOPMENT
						ogl.wireframe = g_ogl_wireframe;
#endif
						ogl.render(&ogl, &render, assets);
					} break;
					default : {
						NOT_IMPLEMENTED;
					} break;
				}
				switch(state.display) {
					case DisplayType_opengl : {
						ogl.display(&ogl, dim.w, dim.h, back_buff.pixels.memory, blit);
#if KH_IN_DEVELOPMENT
						if(memory.debug_state && memory.debug_state->show) {
							ogl_debug_draw(&ogl_debug, memory.debug_state, &render, blit);
						}
#endif
						HDC ctx = GetDC(window);
						SwapBuffers(ctx);
						ReleaseDC(window, ctx);
					} break;
					default : {
						NOT_IMPLEMENTED;
					} break;
				}

				Input *tmp = input_cur_frame;
				input_cur_frame = input_last_frame;
				input_last_frame = tmp;

			}

		}
	}
	return(0);
}