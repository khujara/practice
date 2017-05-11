#ifndef KH_CAMERA_H

#define DEFAULT_CAMERA_NEAR 0.3f
#define DEFAULT_CAMERA_FAR 100.0f
#define DEFAULT_CAMERA_FOV 70.0f


enum culling_flags
{
	CullingMask_3D_default = 0x1,
	CullingMask_ui = 0x2,
};

// TODO(flo): Update our camera to our new quaternion system !
struct Camera
{
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

	u32 culling_mask;
	u32 target_texture;

	mat4 view;
	mat4 projection;
};

// TODO(flo): remove targets for perspective camera ? or even all cameras since we're doing rotation with quaternion now?
// TODO(flo): do something with the target texture
inline Camera
default_perspective_camera(f32 w, f32 h, v3 pos, v3 target, u32 culling_mask, v4 color = COLOR_BLACK, 
                           u32 target_texture = 0, f32 fov = DEFAULT_CAMERA_FOV, f32 znear = DEFAULT_CAMERA_NEAR, 
                           f32 zfar = DEFAULT_CAMERA_FAR)
{
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
	res.culling_mask = culling_mask;
	res.target_texture = target_texture;
	res.projection = perspective_fov_lh(fov, res.aspect_ratio, znear, zfar);
	// res.projection = perspective_fov_rh(fov, res.aspect_ratio, znear, zfar);
	return(res);
}

inline v3 
get_cam_pos(Camera *cam) {
	v3 res = kh_get_translation_mat4(cam->view);
	return(res);
}

inline Camera
default_orthographic_camera(f32 size, f32 w, f32 h, v3 pos, v3 target, u32 culling_mask, v4 color = COLOR_BLACK, 
                            u32 target_texture = 0, f32 znear = DEFAULT_CAMERA_NEAR, f32 zfar = DEFAULT_CAMERA_FAR)
{
	Camera res = {};

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
	res.culling_mask = culling_mask;
	res.target_texture = target_texture;
	res.projection = orthographic_lh(res.size.x, res.size.y, znear, zfar);
	return(res);
}

KH_INTERN mat4 
get_vp_matrix(const Camera *cam)
{

	mat4 res = (cam->view * cam->projection);
	return(res);
}

inline v3
from_screen_space_to_vp(v2 pos, f32 w, f32 h)
{
	v3 res;

	res.x = -1.0f + (pos.x / w) * 2;
	res.y = -1.0f + (pos.y / h) * 2;
	res.z = 0;

	return(res);
}


// TODO(flo): experiment with this to debug it and verify that it is correct!
inline v3
from_vp_to_world_ortho(Camera *cam, v3 pos)
{
	mat4 inverse_proj = orthographic_unproj_lh(cam->size.x, cam->size.y, cam->znear, cam->zfar);
	mat4 inverse_view = kh_transpose_mat4(cam->view);

	v4 tmp = (inverse_proj * inverse_view) * kh_vec4(pos, 1);

	v3 res = tmp.xyz;
	return(res);
}

inline v3
from_vp_to_world_persp(Camera *cam, mat4 view, v3 pos)
{
	// TODO(flo): Inverse matrix and perspective unprojection 
	// mat4 inverse_proj = perspective_unproj_lh(cam->size_x, cam->size_y, cam->znear, cam->zfar);
	mat4 inverse_proj = kh_identity_mat4();
	mat4 inverse_view = kh_transpose_mat4(view);

	v4 tmp = (inverse_proj * inverse_view) * kh_vec4(pos, 1);

	v3 res = tmp.xyz;
	return(res);
}


#define KH_CAMERA_H
#endif
