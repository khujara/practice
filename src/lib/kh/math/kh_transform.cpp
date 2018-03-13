KH_INTERN mat4
orthographic_unproj_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar) {
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
orthographic_unproj_lh(f32 width, f32 height, f32 znear, f32 zfar) {
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;
	mat4 res = orthographic_unproj_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
orthographic_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar) {
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
orthographic_lh(f32 width, f32 height, f32 znear, f32 zfar) {
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;
	mat4 res = orthographic_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

// TODO(flo): perspective unproj
KH_INTERN mat4
perspective_unproj_lh() {
	NOT_IMPLEMENTED;
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
perspective_fov_lh(f32 fov, f32 ar, f32 nearz, f32 farz) {
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
perspective_off_center_lh(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar) {
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
perspective_fov_lh2(f32 fov, f32 aspect_ratio, f32 znear, f32 zfar) {
	f32 y_scale = 1.0f / kh_tan_f32(fov * TO_RADIANS * 0.5f);
	f32 x_scale = y_scale / aspect_ratio;

	f32 half_w = znear / x_scale;
	f32 half_h = znear / y_scale;

	mat4 res = perspective_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
perspective_lh(f32 width, f32 height, f32 znear, f32 zfar) {
	f32 half_w = width * 0.5f;
	f32 half_h = height * 0.5f;

	mat4 res = perspective_off_center_lh(-half_w, half_w, -half_h, half_h, znear, zfar);
	return(res);
}

KH_INTERN mat4
look_at_matrix(v3 cam_pos, v3 target_pos, v3 up_axis) {
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

KH_INTERN mat4
look_at_matrix(mat3 w, v3 pos) {
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
look_at_matrix_lh(v3 cam_pos, v3 target_pos, v3 up_axis) {
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

KH_INTERN mat3
look_at_matrix_lh_mat3(v3 cam_pos, v3 target_pos, v3 up_axis = kh_vec3(0,1,0)) {
  mat3 res;

  v3 dir = target_pos - cam_pos;
  v3 z_axis = kh_normalize_v3(dir);
  v3 x_axis = kh_normalize_v3(kh_cross_v3(up_axis, z_axis));
  v3 y_axis = kh_cross_v3(z_axis, x_axis);

  res.m00 = x_axis.x; res.m01 = y_axis.x; res.m02 = z_axis.x;
  res.m10 = x_axis.y; res.m11 = y_axis.y; res.m12 = z_axis.y;
  res.m20 = x_axis.z; res.m21 = y_axis.z; res.m22 = z_axis.z;
  return(res);
}

inline b32
inside_view_frustum(v4 pos) {
	b32 res = ((kh_abs_f32(pos.x) <= kh_abs_f32(pos.w)) &&
		(kh_abs_f32(pos.y) <= kh_abs_f32(pos.w)) &&
		(kh_abs_f32(pos.z) <= kh_abs_f32(pos.w)));
	return(res);
}

inline v4
project_position(mat4 wld, mat4 view, mat4 proj, v3 pos) {
	v4 wld_p = wld * kh_vec4(pos, 1);
	v4 view_p = view * wld_p;
	v4 proj_p = proj * view_p;
	return(proj_p);
}

inline v4
project_position(mat4 mvp, v3 pos) {
	v4 res = mvp * kh_vec4(pos, 1);
	return(res);
}

inline v3
project_normal(mat4 wld_mat, v3 normal) {
	v3 res = kh_normalize_v3(kh_mul_mat4_dir(wld_mat, normal));
	return(res);
}

inline v3
from_mvp_to_screen_space(v4 pos, f32 half_w, f32 half_h) {
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
from_world_meters_to_screen_pixels(f32 wld_meters) {
	f32 res = 0;
	return(res);
}

inline mat4
get_modelviewproj(mat4 wld, mat4 vp) {
	mat4 res = wld * vp;
}

inline mat4
from_euler_angle_to_object_mat4(EulerAngle e) {
	mat4 res;

	f32 sh = kh_sin_f32(e.yaw);
	f32 ch = kh_cos_f32(e.yaw);
	f32 sp = kh_sin_f32(e.pitch);
	f32 cp = kh_cos_f32(e.pitch);
	f32 sb = kh_sin_f32(e.roll);
	f32 cb = kh_cos_f32(e.roll);

	res.c0 = kh_vec4(ch*cb+sh*sp*sb, sb*cp, -sh*cb+ch*sp*sb, 0);
	res.c1 = kh_vec4(-ch*sb+sh*sp*cb, cb*cp, sb*sh+ch*sp*cb, 0);
	res.c2 = kh_vec4(sh*cp, -sp, ch*cp, 0);
	res.c3 = kh_vec4(0,0,0,1);

	return(res);
}

inline mat4
from_euler_angle_to_world_mat4(EulerAngle e) {
	mat4 res = kh_transpose_mat4(from_euler_angle_to_object_mat4(e));
	return(res);
}
#if 0
inline EulerAngle
from_mat4x4_to_euler_angle(mat4 m)
{
	EulerAngle res;
	return(res);
}
#endif

inline mat4
from_quat_to_mat4(quat q) {
	mat4 res;

	f32 xx = 2.0f*q.x*q.x;
	f32 xy = 2.0f*q.x*q.y;
	f32 xz = 2.0f*q.x*q.z;
	f32 yy = 2.0f*q.y*q.y;
	f32 yz = 2.0f*q.y*q.z;
	f32 zz = 2.0f*q.z*q.z;
	f32 wx = 2.0f*q.w*q.x;
	f32 wy = 2.0f*q.w*q.y;
	f32 wz = 2.0f*q.w*q.z;

	res.c0 = kh_vec4(1.0f - yy - zz, xy + wz, xz - wy, 0);
	res.c1 = kh_vec4(xy - wz, 1.0f - xx - zz, yz + wx, 0);
	res.c2 = kh_vec4(xz + wy, yz - wx, 1.0f - xx - yy, 0);
	res.c3 = kh_vec4(0,0,0,1);

	return(res);
}

inline mat3
from_quat_to_mat3(quat q) {
	mat3 res;

	f32 xx = 2.0f*q.x*q.x;
	f32 xy = 2.0f*q.x*q.y;
	f32 xz = 2.0f*q.x*q.z;
	f32 yy = 2.0f*q.y*q.y;
	f32 yz = 2.0f*q.y*q.z;
	f32 zz = 2.0f*q.z*q.z;
	f32 wx = 2.0f*q.w*q.x;
	f32 wy = 2.0f*q.w*q.y;
	f32 wz = 2.0f*q.w*q.z;

	res.c0 = kh_vec3(1.0f - yy - zz, xy + wz, xz - wy);
	res.c1 = kh_vec3(xy - wz, 1.0f - xx - zz, yz + wx);
	res.c2 = kh_vec3(xz + wy, yz - wx, 1.0f - xx - yy);

	return(res);
}

inline quat
from_mat4_to_quat(mat4 m) {
	quat res = {};

	f32 four_wsqrt_minus_one = m.m00 + m.m11 + m.m22;
	f32 four_xsqrt_minus_one = m.m00 - m.m11 - m.m22;
	f32 four_ysqrt_minus_one = m.m11 - m.m00 - m.m22;
	f32 four_zsqrt_minus_one = m.m22 - m.m00 - m.m11;

	i32 biggest_ind = 0;
	f32 biggest_val = four_wsqrt_minus_one;
	if(four_xsqrt_minus_one > biggest_val)
	{
		biggest_val = four_xsqrt_minus_one;
		biggest_ind = 1;
	}
	if(four_ysqrt_minus_one > biggest_val)
	{
		biggest_val = four_ysqrt_minus_one;
		biggest_ind = 2;
	}
	if(four_zsqrt_minus_one > biggest_val)
	{
		biggest_val = four_zsqrt_minus_one;
		biggest_ind = 3;
	}

	biggest_val = kh_sqrt_f32(biggest_val + 1.0f) * 0.5f;
	f32 mul = 0.25f / biggest_val;

	switch(biggest_ind)
	{
		case 0 :
		{
			res.w = biggest_val;
			res.x = (m.m21 - m.m12) * mul;
			res.y = (m.m02 - m.m20) * mul;
			res.z = (m.m10 - m.m01) * mul;
		} break;
		case 1 :
		{
			res.x = biggest_val;
			res.w = (m.m21 - m.m12) * mul;
			res.y = (m.m10 + m.m01) * mul;
			res.z = (m.m02 + m.m20) * mul;
		} break;
		case 2 :
		{
			res.y = biggest_val;
			res.w = (m.m02 - m.m20) * mul;
			res.x = (m.m10 + m.m01) * mul;
			res.z = (m.m21 + m.m12) * mul;
		} break;
		case 3 :
		{
			res.z = biggest_val;
			res.w = (m.m10 - m.m01) * mul;
			res.x = (m.m02 + m.m20) * mul;
			res.y = (m.m21 + m.m12) * mul;
		} break;
	}
	return(res);
}

inline quat
from_mat3_to_quat(mat3 m) {
	quat res;

	f32 four_wsqrt_minus_one = m.m00 + m.m11 + m.m22;
	f32 four_xsqrt_minus_one = m.m00 - m.m11 - m.m22;
	f32 four_ysqrt_minus_one = m.m11 - m.m00 - m.m22;
	f32 four_zsqrt_minus_one = m.m22 - m.m00 - m.m11;

	i32 biggest_ind = 0;
	f32 biggest_val = four_wsqrt_minus_one;
	if(four_xsqrt_minus_one > biggest_val) {
		biggest_val = four_xsqrt_minus_one;
		biggest_ind = 1;
	}
	if(four_ysqrt_minus_one > biggest_val) {
		biggest_val = four_ysqrt_minus_one;
		biggest_ind = 2;
	}
	if(four_zsqrt_minus_one > biggest_val) {
		biggest_val = four_zsqrt_minus_one;
		biggest_ind = 3;
	}

	biggest_val = kh_sqrt_f32(biggest_val + 1.0f) * 0.5f;
	f32 mul = 0.25f / biggest_val;

	switch(biggest_ind) {
		case 0 : 		{
			res.w = biggest_val;
			res.x = (m.m21 - m.m12) * mul;
			res.y = (m.m02 - m.m20) * mul;
			res.z = (m.m10 - m.m01) * mul;
		} break;
		case 1 : 		{
			res.x = biggest_val;
			res.w = (m.m21 - m.m12) * mul;
			res.y = (m.m10 + m.m01) * mul;
			res.z = (m.m02 + m.m20) * mul;
		} break;
		case 2 : {
			res.y = biggest_val;
			res.w = (m.m02 - m.m20) * mul;
			res.x = (m.m10 + m.m01) * mul;
			res.z = (m.m21 + m.m12) * mul;
		} break;
		case 3 : {
			res.z = biggest_val;
			res.w = (m.m10 - m.m01) * mul;
			res.x = (m.m02 + m.m20) * mul;
			res.y = (m.m21 + m.m12) * mul;
		} break;
	}
	return(res);
}

inline quat
from_euler_angle_to_quat_world(EulerAngle e) {
	f32 half_yaw = e.yaw * 0.5f;
	f32 half_pitch = e.pitch * 0.5f;
	f32 half_roll = e.roll * 0.5f;

	f32 ch = kh_cos_f32(half_yaw);
	f32 sh = kh_sin_f32(half_yaw);
	f32 cp = kh_cos_f32(half_pitch);
	f32 sp = kh_sin_f32(half_pitch);
	f32 cb = kh_cos_f32(half_roll);
	f32 sb = kh_sin_f32(half_roll);

	f32 chcb = ch*cb;
	f32 shsb = sh*sb;
	f32 chsb = ch*sb;
	f32 shcb = sh*cb;

	quat res;
	res.w = chcb*cp + shsb*sp;
	res.x = chcb*sp + shsb*cp;
	res.y = shcb*cp - chsb*sp;
	res.z = chsb*cp - shcb*sp;

	return(res);
}

inline quat
from_euler_angle_to_quat_object(EulerAngle e) {
	quat res = conjugate(from_euler_angle_to_quat_world(e));
	return(res);
}

inline EulerAngle
from_quat_to_euler_angle(quat q) {
	EulerAngle res;

	f32 ysqr = q.y*q.y;

	// NOTE(flo): roll (x_axis rotation)
	f32 t0 = 2.0f * (q.w * q.x + q.y * q.z);
	f32 t1 = 1.0f - 2.0f * (q.x * q.x + ysqr);
	res.roll = kh_atan2_f32(t0, t1);

	// NOTE(flo): pitch (y_axis rotation)
	f32 t2 = 2.0f * (q.w * q.y - q.z * q.x);
	t2 = kh_min_f32(1.0f, t2);
	t2 = kh_max_f32(-1.0f, t2);
	res.pitch = kh_asin_f32(t2);

	// NOTE(flo): yaw (z_axis rotation)
	f32 t3 = 2.0f * (q.w * q.z + q.x * q.y);
	f32 t4 = 1.0f - 2.0f*(ysqr + q.z * q.z);
	res.yaw = kh_atan2_f32(t3, t4);


	return(res);
}

inline v3
to_polar_v3(v3 v) {
	v3 res;
	NOT_IMPLEMENTED;
	f32 radius = kh_length_v3(v);
	f32 latitude = kh_acos_f32(v.z/radius);
	f32 longitude = kh_atan2_f32(v.y, v.z);

	return(res);
}

inline v2
to_polar_v2(v2 v) {
	v2 res;

	f32 radius = kh_len_v2(v);
	f32 theta = v.x/radius;

	res.x = radius;
	res.y = theta;

	return(res);
}