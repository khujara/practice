#define IMGUI_API __declspec(dllimport)
#include "ext/imgui.h"
// #include "ext/imgui.cpp"
// #include "ext/imgui_draw.cpp"

struct DebugLine {
	v3 pos;
	v3 color;
};

struct DebugHemisphere {
	mat4 tr;
	v3 color;
	f32 scale;
};

#define create_debug_array_struct(name) struct name##Array { u32 count; u32 max_count; name *data;}
create_debug_array_struct(DebugLine);
create_debug_array_struct(DebugHemisphere);

struct DebugState {
	LinearArena debug_arena;

	b32 show;

	u32 render_w, render_h;
	u32 wnd_w, wnd_h;

	Input *input;
	Assets *assets;

	DebugLineArray lines;
	DebugHemisphereArray hemispheres;

	u32 last_entity_index;
	b32 check_entity_on_click;

	// TODO(flo): we do not need these anymore!
	u8 *imgui_font_px;
	i32 imgui_font_w;
	i32 imgui_font_h;
	ImDrawData *imgui_data;
	void *imgui_font_id;

	// ImGuiContext *imctx;
};

KH_INTERN DebugState *
debug_init(Assets *assets) {
	DebugState *res = boot_strap_push_struct(DebugState, debug_arena);

	res->render_w = res->render_h = 0;

	res->lines.max_count = 8192;
	res->lines.count = 0;
	res->lines.data = (DebugLine *)kh_push(&res->debug_arena, res->lines.max_count * sizeof(DebugLine));

	res->hemispheres.max_count = 32;
	res->hemispheres.count = 0;
	res->hemispheres.data = (DebugHemisphere *)kh_push(&res->debug_arena, res->hemispheres.max_count * sizeof(DebugHemisphere));

	res->assets = assets;
	res->input = 0;
	res->last_entity_index = 0;
	res->check_entity_on_click = false;

	return(res);
}