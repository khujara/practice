#ifndef KH_CAMERA_H

#define DEFAULT_CAMERA_NEAR 0.3f
#define DEFAULT_CAMERA_FAR 100.0f
#define DEFAULT_CAMERA_FOV 70.0f


// enum culling_flags {
// 	CullingMask_3D_default = 0x1,
// 	CullingMask_ui = 0x2,
// };

// TODO(flo): Update our camera to our new quaternion system !
struct Camera {
	/*TODO(flo):
		- implement the use of culling mask (when we add an entry we want to check camera that had the same culling mask and add entry to the associated frame buffer if yes)
		- allow skyboxes
		- allow viewport rect if we do not want full screen
		- allow priority (for camera on top of each other)
	*/

	Transform_SQT tr;


	v4 clear_color;
	b32 orthographic;

	union
	{
		f32 fov;
		v2 size;
	};

	// TODO(flo): viewport!
	f32 w, h;

	f32 aspect_ratio;
	f32 znear;
	f32 zfar;

	float dist;
	v3 target;
	v3 right;

	// u32 culling_mask;
	// u32 target_texture;

	mat4 view;
	mat4 projection;
};

// TODO(flo): remove targets for perspective camera ? or even all cameras since we're doing rotation with quaternion now?
// TODO(flo): do something with the target texture
KH_INLINE Camera
perspective_camera(f32 w, f32 h, v3 pos, v3 target, v4 color = COLOR_BLACK, f32 fov = DEFAULT_CAMERA_FOV, 
                   f32 znear = DEFAULT_CAMERA_NEAR, f32 zfar = DEFAULT_CAMERA_FAR) {
	Camera res = {};
	res.clear_color = color;
	res.fov = fov;
	res.w = w;
	res.h = h;
	res.aspect_ratio = w/h;
	res.znear = znear;
	res.zfar = zfar;
	res.tr.pos = pos;
	res.tr.rot = quat_identity();
	res.target = target;
	res.projection = perspective_fov_lh(fov, res.aspect_ratio, znear, zfar);
	// res.projection = perspective_fov_rh(fov, res.aspect_ratio, znear, zfar);
	return(res);
}

KH_INLINE v3 
get_cam_pos(Camera *cam) {
	v3 res = kh_get_translation_mat4(cam->view);
	return(res);
}

KH_INLINE Camera
orthographic_camera(f32 size, f32 w, f32 h, v3 pos, v3 target, v4 color = COLOR_BLACK,
                            f32 znear = DEFAULT_CAMERA_NEAR, f32 zfar = DEFAULT_CAMERA_FAR) {
	Camera res = {};

	res.dist = 1.0f;
	res.clear_color = color;
	res.orthographic = true;
	res.aspect_ratio = w / h;
	res.size = kh_vec2(size * 2 * res.aspect_ratio, size * 2);
	res.w = w;
	res.h = h;
	res.znear = znear;
	res.zfar = zfar;
	res.tr.pos = pos;
	res.tr.rot = quat_identity();
	res.target = target;
	res.projection = orthographic_lh(res.size.x, res.size.y, znear, zfar);
	return(res);
}

KH_INTERN mat4 
get_vp_matrix(const Camera *cam) {
	mat4 res = (cam->view * cam->projection);
	return(res);
}

inline v3
from_screen_space_to_vp(v2 pos, f32 w, f32 h) {
	v3 res;
	res.x = -1.0f + (pos.x / w) * 2;
	res.y = -1.0f + (pos.y / h) * 2;
	res.z = 0;
	return(res);
}

// TODO(flo): experiment with this to debug it and verify that it is correct!
inline v3
from_vp_to_world_ortho(Camera *cam, v3 pos) {
	mat4 inverse_proj = orthographic_unproj_lh(cam->size.x, cam->size.y, cam->znear, cam->zfar);
	mat4 inverse_view = kh_transpose_mat4(cam->view);

	v4 tmp = (inverse_proj * inverse_view) * kh_vec4(pos, 1);

	v3 res = kh_vec3(tmp);
	return(res);
}

inline v3
from_vp_to_world_persp(Camera *cam, mat4 view, v3 pos) {
	// TODO(flo): Inverse matrix and perspective unprojection 
	// mat4 inverse_proj = perspective_unproj_lh(cam->size_x, cam->size_y, cam->znear, cam->zfar);
	mat4 inverse_proj = kh_identity_mat4();
	mat4 inverse_view = kh_transpose_mat4(view);

	v4 tmp = (inverse_proj * inverse_view) * kh_vec4(pos, 1);

	v3 res = kh_vec3(tmp);
	return(res);
}

inline void
lookat(Camera *cam, v3 pos, v3 target, v3 up) {
	cam->tr.pos = pos;
	cam->target = target;
	cam->view = look_at_matrix_lh(cam->tr.pos, cam->target, up);
	cam->right = kh_vec3(cam->view.c0.x, cam->view.c1.x, cam->view.c2.x);
	cam->tr.rot = from_mat4_to_quat(cam->view);
	cam->dist = kh_length_v3(cam->target - cam->tr.pos); 
}

inline mat3
set_view_matrix(Camera *cam) {
	mat3 cam_wld = from_quat_to_mat3(cam->tr.rot);
	cam->right = kh_right_mat3(cam_wld);
	cam->view = look_at_matrix(cam_wld, cam->tr.pos);
	return(cam_wld);
}

#define KH_CAMERA_H
#endif
