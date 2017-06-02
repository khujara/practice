#include "kh/kh_platform.h"
KH_GLOBAL b32 global_isrunning;
Platform g_platform;

#include <windows.h>
#include "kh\kh_win32.h"

#include <xinput.h>
#include <gl/gl.h>
#include "practice_win32.h"

#include "kh/kh_win32_oglext.h"
#include "kh/kh_opengl.h"

#include "kh_asset_file.h"
#include "kh_asset.h"
#include "kh/kh_render.h"
#include "kh_renderer_software.h"

#include "kh_rasterizer_software.cpp"
#include "kh_renderer_software.cpp"
#include "kh_renderer_opengl.cpp"
#include "kh_asset.cpp"
#include "kh_asset_init.cpp"

LRESULT WINAPI
win32_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT res = 0;

	switch(message)
	{
		case WM_CLOSE :
		{
			global_isrunning = false;
		} break;
		case WM_DESTROY :
		{
			global_isrunning = false;
		} break;
		case WM_PAINT :
		{
			PAINTSTRUCT paint;
			HDC device_ctx_hdl = BeginPaint(window, &paint);
			EndPaint(window, &paint);
		} break;

		default :
		{
			res = DefWindowProcA(window, message, w_param, l_param);
		} break;
	}

	return(res);
}

KH_INTERN SoftwareFrameBuffer
win32_intialize_back_buffer(RenderManager *render)
{
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
win32_button_event(ButtonState *state, b32 is_down)
{
	if(state->down != is_down)
	{
		state->down = is_down;
		++state->down_count;
	}
}

KH_INTERN void
win32_process_messages(win32_state *state, Input *input)
{
	MSG win32_mes;
	for(;;)
	{
		BOOL got_mes = FALSE;

		{
			got_mes = PeekMessage(&win32_mes, 0, 0, 0, PM_REMOVE);
		}
		
		if(!got_mes)
		{
			break;
		}

		switch(win32_mes.message)
		{
			case WM_QUIT :
			{
				global_isrunning = false;
			} break;

			case WM_SYSKEYDOWN :
			case WM_SYSKEYUP :
			case WM_KEYDOWN :
			case WM_KEYUP :
			{
				b32 show_menu = state->show_menu;
				u32 VKCode = (u32)win32_mes.wParam;
				b32 WasPressed = ((win32_mes.lParam & (1 << 30)) != 0);
				b32 IsPressed = ((win32_mes.lParam & (1 << 31)) == 0);

				b32 IsUp = (WasPressed && !IsPressed);
				b32 IsDown = (!WasPressed && IsPressed);

				if(IsDown)
				{
					switch(VKCode)
					{
						case 'Q' : { win32_toggle_full_screen(win32_mes.hwnd, &state->window_pos); } break;
						case 'L' : { state->show_menu = !state->show_menu; } break;
						case 'S' : { 
							// TODO(flo): we will need a gui for this
							if(state->renderer == RendererType_software) {
								state->renderer = RendererType_opengl;
							} else {
								state->renderer = RendererType_software;
							}
						} break;
						case 0x30 : {
							state->new_sample = Sample_basic_scene;
						} break;
						case 0x31 : {
							state->new_sample = Sample_basic_mesh;
						} break;
						case 0x32 : {
							state->new_sample = Sample_basic_animation;
						} break;
						case VK_ESCAPE : { global_isrunning = false; } break;
					}
				}
			} break;

			case WM_MOUSEWHEEL :
			{
				input->dt_wheel = (i32)GET_WHEEL_DELTA_WPARAM(win32_mes.wParam);
			} break;
			default :
			{
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

KH_INTERN win32_sample_library
win32_load_sample_library(int index) {
	win32_sample_library res = {};

	u32 ind = index - 1;
	PracticeSample sample = (PracticeSample)ind;

	char *dlls[] = {FOREACH_SAMPLE(GENERATE_DLL_NAMES)};
	char dll[1024];
	char *dll_name = dlls[ind];
	ProgramPath exe_path = win32_get_program_path();
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
win32_unload_sample(win32_sample_library *sample) {
	if(sample->lib) {
		FreeLibrary(sample->lib);
		sample->lib = 0;
	}
	sample->is_loaded = false;
	sample->frame_update = 0;
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

		prog->platform.init_asset_import = (InitAssetLoader *)GetProcAddress(lib, "init_asset_loader");
		kh_assert(prog->platform.init_asset_import);
		prog->platform.init_asset_import(&prog->platform);
	}

}
#endif

KH_INTERN void
win32_opengl_init(HWND window)
{
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
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB|WGL_CONTEXT_DEBUG_BIT_ARB,
		// WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		// WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0,
	};

	HGLRC modern_opengl_rc = wglCreateContextAttribsARB(wnd_dc, 0, context_attribs_45);
	make_cur = wglMakeCurrent(wnd_dc, modern_opengl_rc);

	HGLRC cur_ogl_rc = wglGetCurrentContext();
	kh_assert(cur_ogl_rc == modern_opengl_rc);

	win32_load_gl_extensions();
	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(1);
	}
	else
	{
		kh_assert(!"no swap interval");
	}

	// wglDeleteContext(opengl_rc);
	ReleaseDC(window, wnd_dc);

	// TODO(flo): use this for recent OpenGL
	// wglChoosePixelFormatARB(wnd_dc, attrib_list, NULL, 1, &pixel_format, &num_formats);
}

KH_INTERN void
win32_load_graphics_api(HWND window) {
	win32_opengl_init(window);
}

KH_INTERN void
reset_memory(ProgramMemory *mem, RenderManager *render, OglState *ogl) {
	StackAllocator p_stack_copy = mem->p_state->stack;
	if(mem->p_state) {
		kh_clear(&p_stack_copy);
	}
	mem->p_state = 0;

	StackAllocator f_stack_copy = mem->f_state->stack;
	if(mem->f_state) {
		kh_clear(&f_stack_copy);
	}
	mem->f_state = 0;

	render->light_count = 0;
	render->has_skybox = false;

	ogl_delete_skybox(ogl);
	ogl_delete_light(ogl);
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int cmd_show)
{


	/* TODO(flo): 
		- send our update to github
		- some basic IK implementation
	*/

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
		HWND window = CreateWindowExA(0, wnd_class.lpszClassName, "Practice", 
			WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
			CW_USEDEFAULT, CW_USEDEFAULT, start_w, start_h, 0, 0, instance, 0);
		if(window) {
			win32_state state = {};
			state.window_pos = { sizeof(state.window_pos) };
			state.show_menu = false;
			state.renderer = RendererType_opengl;
			// state.renderer = RendererType_software;
			state.display = DisplayType_opengl;
			state.new_sample = Sample_basic_animation;
			// state.new_sample = Sample_basic_scene;
			// state.new_sample = Sample_basic_mesh;
			state.cur_sample = Sample_none;

			win32_sample_library sample = {};

			const u32 high_thread_count = 4;
			WorkQueue high_queue = {};
			// TODO(flo): do not like this!
			win32_sched_handle high_handle = win32_create_thread_handle(high_thread_count);
			win32_create_sched_queue(&high_queue, &high_handle, high_thread_count);

			// u32 low_thread_count = 2;
			WorkQueue low_queue = {};
			// TODO(flo): do not like this!
			win32_sched_handle low_handle = win32_create_thread_handle(1);
			win32_create_sched_queue(&low_queue, &low_handle, 1);

			// win32_toggle_full_screen(window, &state.window_pos);

			u32 monitor_refresh_freq = 60;
			HDC refresh_dc = GetDC(window);
			i32 win32_refresh_rate = GetDeviceCaps(refresh_dc, VREFRESH);
			ReleaseDC(window, refresh_dc);
			if(win32_refresh_rate > 0)
			{
				monitor_refresh_freq = win32_refresh_rate;
			}
			f32 update_freq = (f32)monitor_refresh_freq;
			f32 target_spf = 1.0f / update_freq;

			win32_init_allocator_sentinel();

			StackAllocator menu_stack = {};
			StackAllocator backbuffer_stack = {};


			ProgramMemory memory = {};
			memory.high_queue = &high_queue;
			memory.low_queue = &low_queue;

			// memory.platform.get_program_path = win32_get_program_path;
			memory.platform.add_work_to_queue = win32_add_work_to_queue;
			memory.platform.complete_all_queue_works = win32_complete_all_queue_works;
			memory.platform.get_all_files_of_type = win32_get_all_files_of_type;
			memory.platform.close_file_group = win32_close_file_group;
			memory.platform.open_next_file_of_type = win32_open_next_file_of_type;
			memory.platform.open_file = win32_open_file;
			memory.platform.close_file = win32_close_file;
			memory.platform.read_bytes_of_file = win32_read_bytes_of_file;
			memory.platform.write_bytes_to_file = win32_write_bytes_to_file;
			memory.platform.get_file_size = win32_get_file_size;
			memory.platform.virtual_alloc = win32_virtual_alloc;
			memory.platform.virtual_free = win32_virtual_free;
			#ifdef KH_IN_DEVELOPMENT
			win32_load_asset_import_library(&memory);
			#endif
			g_platform = memory.platform;

			win32_load_graphics_api(window);

			global_isrunning = true;
			f32 old_mouse_x = 0;
			f32 old_mouse_y = 0;

			Input cur_frame_input_ = {};
			Input last_frame_input_ = {};
			Input *input_cur_frame = &cur_frame_input_;
			Input *input_last_frame = &last_frame_input_;

			RenderManager render = {};
			// render.width         = 800;
			// render.height        = 450;
			// render.width         = 1280;
			// render.height        = 720;
			render.width         = 1920;
			render.height        = 1080;
			render.commands_size = kilobytes(4);
			render.commands      = (u8 *)VirtualAlloc(0, render.commands_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.animators.max_count = 64;
			render.animators.count = 0;
			render.animators.data = (Animator *)VirtualAlloc(0, render.animators.max_count * sizeof(Animator), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			render.bone_tr.max_count = 1024;
			render.bone_tr.count = 0;
			render.bone_tr.data = (mat4 *)VirtualAlloc(0, render.bone_tr.max_count * sizeof(mat4), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

			// NOT_IMPLEMENTED;
			// TODO(flo): since our assets system is always loaded in the platform layer
			// i do not think we have to return a pointer here
			Assets *assets = load_assets_infos("datas.khjr", memory.high_queue, megabytes(256), 16);

			OglState ogl = {};
			ogl_start(&ogl, &render, assets);
			SoftwareFrameBuffer back_buff = win32_intialize_back_buffer(&render);

			const u32 work_count = 128;
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

			u32 render_size = render.width * render.height * sizeof(u32);
			GLuint pixel_unpack_b;
			glCreateBuffers(1, &pixel_unpack_b);
			glNamedBufferData(pixel_unpack_b, render_size, 0, GL_DYNAMIC_COPY);
			while(global_isrunning)
			{


				Win32DebugTimer frame_time("-----------------------------START FRAME----------------------- \n");

				f32 dt = target_spf;
				input_cur_frame->dt_wheel = 0;

				// u64 begin = __rdtsc();
				// OutputDebugStringA("-----------------------------START FRAME----------------------- \n");

				win32_process_messages(&state, input_cur_frame);

			    // u64 end = __rdtsc();
			    // u64 elapsed = end - begin;
			    // char text_buff[256];
			    // _snprintf_s(text_buff, sizeof(text_buff), "win32 peek message cycle : %llu \n", elapsed);
			    // OutputDebugStringA(text_buff);

			    POINT mouse_p;
			    GetCursorPos(&mouse_p);
			    ScreenToClient(window, &mouse_p);

			    win32_wnd_dim dim = win32_get_wnd_dim(window);
			    f32 mouse_p_x = (f32)mouse_p.x;
			    f32 mouse_p_y = (f32)((dim.h - 1) - mouse_p.y);

			    input_cur_frame->delta_mouse_x = mouse_p_x - input_cur_frame->mouse_x; 
			    input_cur_frame->delta_mouse_y = mouse_p_y - input_cur_frame->mouse_y; 

			    input_cur_frame->mouse_x = mouse_p_x;
			    input_cur_frame->mouse_y = mouse_p_y;

			    DWORD win_mouse_id[MouseButton_count] =
			    {
			    	VK_LBUTTON,
			    	VK_RBUTTON,
			    	VK_MBUTTON,
			    	VK_XBUTTON1,
			    	VK_XBUTTON2,
			    };

			    for(u32 b_i = 0; b_i < MouseButton_count; ++b_i)
			    {
			    	input_cur_frame->mouse_buttons[b_i] = input_last_frame->mouse_buttons[b_i];
			    	input_cur_frame->mouse_buttons[b_i].down_count = 0;
			    	win32_button_event(&input_cur_frame->mouse_buttons[b_i], GetKeyState(win_mouse_id[b_i]) & (1 << 15));
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
				    	reset_memory(&memory, &render, &ogl);
			    	}
			    	state.cur_sample = state.new_sample;
			    	state.new_sample = Sample_none;
			    }


			    if(sample.frame_update) {
			    	sample.frame_update(&memory, input_cur_frame, &render, assets, dt);
			    }

			    // frame_update(&memory, input_cur_frame, &render, assets, dt);

			    switch(state.renderer) {
			    	case RendererType_software : {
					    software_render(&back_buff, assets, &render, &high_queue, works, work_count);
						// glNamedBufferData(pixel_unpack_b, render_size, back_buff.pixels.memory, GL_STREAM_DRAW);
						glNamedBufferSubData(pixel_unpack_b, 0, render_size, back_buff.pixels.memory);
						glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixel_unpack_b);
						glTextureSubImage3D(ogl.target.name, 0, 0, 0, ogl.target.slice, render.width, render.height, 1, GL_BGRA, GL_UNSIGNED_BYTE, 0);
						glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

			    	} break;
			    	case RendererType_opengl : {
					    ogl_render(&ogl, &render, assets);
			    	} break;
			    	default : {
			    		NOT_IMPLEMENTED;
			    	} break;
			    }
			    switch(state.display) {
			    	case DisplayType_opengl : {
					    ogl_display_buffer(&ogl, dim.w, dim.h);
	    	    		HDC ctx = GetDC(window);
						SwapBuffers(ctx);
						ReleaseDC(window, ctx);
			    	} break;
			    	default : {
			    		NOT_IMPLEMENTED;
			    	} break;
			    }
			}
		}
	}
	return(0);
}