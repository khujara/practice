KH_INTERN mat4
orthographic_unproj_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar)
{
	mat4 res = {};

	res.m00 = (right - left) * 0.5f;
	res.m11 = (top - bottom) * 0.5f;
	res.m22 = -(zfar - znear) * 0.5f;

	res.m03 = -(left + right) * 0.5f;
	res.m13 = -(top + bottom) * 0.5f;
	res.m23 = -(zfar + right) * 0.5f;

	res.m33 = 1;

	return(res);

}

KH_INTERN mat4
orthographic_unproj_lh(f32 width, f32 height, f32 znear, f32 zfar)
{
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;

	mat4 res = orthographic_unproj_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
orthographic_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar)
{
	f32 range = 1.0f / (zfar - znear);

	mat4 res = {};

	res.m00 = 2.0f / (right - left);
	res.m11 = 2.0f / (top - bottom);
	res.m22 = range;

	res.m03 = ((left + right) / (left - right)); // -(left + right) / (right - left);
	res.m13 = ((top + bottom) / (bottom - top));
	res.m23 = -znear * range;

	res.m33 = 1;
	return(res);

}

KH_INTERN mat4
orthographic_lh(f32 width, f32 height, f32 znear, f32 zfar)
{
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;

	mat4 res = orthographic_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

// TODO(flo): perspective unproj
KH_INTERN mat4
perspective_unproj_lh()
{
}


/* TODO(flo): 

	Perspective fov rh

		2n/r-l	0		r+l/r-l		0
		0		2n/t-b	t+b/t-b		0
		0		0		-f+n/f-n	-2fn/f-n
		0		0		-1			0

	Orthographic rh

		2/r-l	0		0		r+l/r-l
		0		2/t-b	0		t+b/t-b
		0		0		-2/f-n	f+n/f-n
		0		0		0		1
		

*/
KH_INTERN mat4 
perspective_fov_lh(f32 fov, f32 ar, f32 nearz, f32 farz)
{
	mat4 res = {};

	f32 focal_length = 1.0f / kh_tan_f32(fov * 0.5f * TO_RADIANS);
	f32 dz = farz / (farz - nearz);

	res.m00 = focal_length / ar;
	res.m11 = focal_length;
	res.m22 = dz;
	res.m23 = 1.0f;
	res.m32 = -dz * nearz;
	// res.m33 = 1.0f;

	return(res);
}

KH_INTERN mat4 
perspective_fov_rh(f32 fov, f32 ar, f32 nearz, f32 farz) {
	mat4 res = {};

	f32 dtan = 1.0f / kh_tan_f32(fov * 0.5f * TO_RADIANS);
	f32 dz = 1.0f / (farz - nearz);

	res.m00 = dtan / ar;
	res.m11 = dtan;
	res.m22 = farz + nearz / dz;
	res.m23 = 1.0f;
	res.m32 = 2.0f * nearz * farz * dz;

	return(res);
}

KH_INTERN mat4
perspective_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar)
{
	f32 range = zfar / (zfar - znear);

	mat4 res = {};
	res.m00 = (2.0f * znear) / (right - left);
	res.m11 = (2.0f * znear) / (top - bottom);

	res.m02 = (left + right) / (left-right);
	res.m21 = (top + bottom) / (bottom - top);

	res.m22 = range;
	//NOTE(Flo) : Z move forward
	res.m23 = 1.0f;
	res.m32 = -znear * range;

	return(res);
}

KH_INTERN mat4
perspective_fov_lh2(f32 fov, f32 aspect_ratio, f32 znear, f32 zfar)
{
	f32 y_scale = 1.0f / kh_tan_f32(fov * TO_RADIANS * 0.5f);
	f32 x_scale = y_scale / aspect_ratio;

	f32 half_w = znear / x_scale;
	f32 half_h = znear / y_scale;

	mat4 res = perspective_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
perspective_lh(f32 width, f32 height, f32 znear, f32 zfar)
{
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;

	mat4 res = perspective_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
look_at_matrix(v3 cam_pos, v3 target_pos, v3 up_axis)
{
  mat4 res;

  v3 dir = cam_pos - target_pos;

	// NOTE(flo) : Normalization we need to keep length of dir
	// f32 DirLength = Length(Dir);
	// f32 InverseLength = SafeRatio0(1.0f, DirLength);
	// v3 zAxis = InverseLength * Dir;

  v3 z_axis = kh_normalize_v3(dir);
  v3 x_axis = kh_cross_v3(z_axis, up_axis);
  v3 y_axis = kh_cross_v3(x_axis, z_axis);

  res.m00 = x_axis.x; res.m01 = y_axis.x; res.m02 = z_axis.x; res.m03 = 0;
  res.m10 = x_axis.y; res.m11 = y_axis.y; res.m12 = z_axis.y; res.m13 = 0;
  res.m20 = x_axis.z; res.m21 = y_axis.z; res.m22 = z_axis.z; res.m23 = 0;
  res.m30 = -kh_dot_v3(x_axis, cam_pos);
  res.m31 = -kh_dot_v3(y_axis, cam_pos);
  res.m32 = -kh_dot_v3(z_axis, cam_pos);
	// res.m32 = -DirLength;

  res.m33 = 1;

  return(res);
}

// NOTE(flo): build view matrix from a quaternion matrix
// TODO(flo): repair this check implementation on unity
KH_INTERN mat4
look_at_matrix(mat3 w, v3 pos)
{
	mat4 res;

	v3 x_axis = kh_vec3(w.m00, w.m01, w.m02);
	v3 y_axis = kh_vec3(w.m10, w.m11, w.m12);
	v3 z_axis = kh_vec3(w.m20, w.m21, w.m22);

	res.m00 = x_axis.x;
	res.m01 = y_axis.x;
	res.m02 = z_axis.x;
	res.m03 = 0;

	res.m10 = x_axis.y;
	res.m11 = y_axis.y;
	res.m12 = z_axis.y;
	res.m13 = 0;

	res.m20 = x_axis.z;
	res.m21 = y_axis.z;
	res.m22 = z_axis.z;
	res.m23 = 0;

	res.m30 = -kh_dot_v3(x_axis, pos);
	res.m31 = -kh_dot_v3(y_axis, pos);
	res.m32 = -kh_dot_v3(z_axis, pos);
	res.m33 = 1;

	return(res);
}


KH_INTERN mat4
look_at_matrix_lh(v3 cam_pos, v3 target_pos, v3 up_axis)
{
  mat4 res;

  v3 dir = target_pos - cam_pos;

	// NOTE(flo) : Normalization we need to keep length of dir
	// f32 DirLength = Length(Dir);
	// f32 InverseLength = SafeRatio0(1.0f, DirLength);
	// v3 zAxis = InverseLength * Dir;

  v3 z_axis = kh_normalize_v3(dir);
  // up_axis = (z_axis.y == -1.0f) ? V3(0, 0, 1) : up_axis;
  // up_axis = (z_axis.y == 1.0f) ? V3(0, 0, -1) : up_axis;
  v3 x_axis = kh_normalize_v3(kh_cross_v3(up_axis, z_axis));
  v3 y_axis = kh_cross_v3(z_axis, x_axis);

  res.m00 = x_axis.x; res.m01 = y_axis.x; res.m02 = z_axis.x; res.m03 = 0;
  res.m10 = x_axis.y; res.m11 = y_axis.y; res.m12 = z_axis.y; res.m13 = 0;
  res.m20 = x_axis.z; res.m21 = y_axis.z; res.m22 = z_axis.z; res.m23 = 0;
  res.m30 = -kh_dot_v3(x_axis, cam_pos);
  res.m31 = -kh_dot_v3(y_axis, cam_pos);
  res.m32 = -kh_dot_v3(z_axis, cam_pos);
	// res.m32 = -DirLength;
  res.m33 = 1.0f;
  return(res);
}

inline b32
inside_view_frustum(v4 pos)
{
	b32 res = ((kh_abs_f32(pos.x) <= kh_abs_f32(pos.w)) &&
		(kh_abs_f32(pos.y) <= kh_abs_f32(pos.w)) &&
		(kh_abs_f32(pos.z) <= kh_abs_f32(pos.w)));
	return(res);
}

inline v4
project_position(mat4 wld, mat4 view, mat4 proj, v3 pos)
{
	v4 wld_p = wld * kh_vec4(pos, 1);
	v4 view_p = view * wld_p;
	v4 proj_p = proj * view_p;
	return(proj_p);
}

inline v4
project_position(mat4 mvp, v3 pos)
{
	v4 res = mvp * kh_vec4(pos, 1);
	return(res);
}

inline v3
project_normal(mat4 wld_mat, v3 normal)
{
	v3 res = kh_normalize_v3(kh_mul_mat4_dir(wld_mat, normal));
	return(res);
}

inline v3
from_mvp_to_screen_space(v4 pos, f32 half_w, f32 half_h)
{
	v3 res;

	// pos.xyz = pos.xyz / pos.w;
	res.x = pos.x * half_w + pos.w * half_w;
	res.y = pos.y * half_h + pos.w * half_h;
	res.z = pos.w;

	f32 inv_z = kh_safe_ratio1_f32(1.0f, res.z);

	res.x = res.x * inv_z;
	res.y = res.y * inv_z;

	return(res);
}

inline f32
from_world_meters_to_screen_pixels(f32 wld_meters)
{
	f32 res = 0;
	return(res);
}

inline mat4
get_modelviewproj(mat4 wld, mat4 vp)
{
	mat4 res = wld * vp;
}