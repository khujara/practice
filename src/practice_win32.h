#ifndef KHJR_WIN32_H

#define FOREACH_SAMPLE(SAMPLE) \
	SAMPLE(basic_mesh) \
	SAMPLE(basic_scene) \
	SAMPLE(basic_animation) \

#define GENERATE_DLL_NAMES(NAME) "kh_" #NAME "_sample.dll",
#define GENERATE_DLL_ENUM(ENUM) Sample_##ENUM,

enum PracticeSample
{
	Sample_none,
	FOREACH_SAMPLE(GENERATE_DLL_ENUM)
};

struct win32_sample_library
{
	HMODULE lib;
	b32 is_loaded;
	kh_update *frame_update;
};

// TODO(flo): remove this?
#if 0
enum main_menu_state
{
	MainMenu_start,
	MainMenu_show,
};

typedef void mainmenu_reset(prog_memory *mem);
typedef void mainmenu_show(StackAllocator *memstack, prog_memory *memory, WorkQueue *queue, kh_render_manager *rd_buff, Assets *assets, f32 dt, main_menu_state state);
struct win32_main_menu_library
{
	HMODULE lib;
	b32 is_loaded;
	mainmenu_show *show_main_menu;
	mainmenu_reset *reset_main_menu;
};
#endif
typedef DWORD WINAPI xinput_get_state(DWORD dwUserIndex, XINPUT_STATE *pState);
typedef DWORD WINAPI xinput_set_state(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

struct win32_xinput_library
{
	HMODULE lib;
	xinput_get_state *get_state;
	xinput_set_state *set_state;
};

enum RendererType {
	RendererType_software,
	RendererType_opengl,
};

enum DisplayType {
	DisplayType_opengl,
};

struct win32_state
{
	RendererType renderer;
	DisplayType display;

	PracticeSample new_sample;
	PracticeSample cur_sample;

	b32 show_menu;
	WINDOWPLACEMENT window_pos;
};

#define KHJR_WIN32_H
#endif //KHJR_WIN32_H
