#ifndef KH_SAMPLE_SHARED_H

#include "kh\kh_platform.h"
#include "..\kh_material_names.h"
#include "..\kh_shading_names.h"
#include "..\kh_asset_names.h"
#include "..\kh_asset.h"
#include "..\kh_render.h"
#include "..\kh_scene.h"
#include "..\kh_debug.h"
// #include "..\kh_asset_names.h"
#include "..\kh_asset.cpp"
#include "..\kh_render.cpp"
#include "..\kh_debug.cpp"
#include "..\kh_scene.cpp"
#include "..\kh_level_editor.cpp"
#include "kh\kh_animation.cpp"
#include "..\kh_asset_primitives.cpp"

KH_INTERN void
default_camera_move(Camera *cam, Input *input, f32 dt, f32 zoom_speed = 0.05f) {
	v2 dt_mouse = kh_vec2(input->delta_mouse_x, input->delta_mouse_y);
	b32 middle_down = input->mouse_buttons[MouseButton_middle].down;
	f32 wheel = (f32)input->dt_wheel;

	cam->dist -= wheel * zoom_speed * dt;
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

	cam->tr.pos = cam->target - rotate(kh_vec3(0,0,1), cam->tr.rot) * cam->dist;
	set_view_matrix(cam);
}

#define KH_SAMPLE_SHARED_H
#endif // KH_SAMPLE_SHARED_H