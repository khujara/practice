inline mat4
from_euler_angle_to_object_mat4(euler_angle e)
{
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
from_euler_angle_to_world_mat4(euler_angle e)
{
	mat4 res = kh_transpose_mat4(from_euler_angle_to_object_mat4(e));
	return(res);
}
#if 0
inline euler_angle
from_mat4x4_to_euler_angle(mat4 m)
{
	euler_angle res;
	return(res);
}
#endif

inline mat4
from_quat_to_mat4(quat q)
{
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
from_quat_to_mat3(quat q)
{
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
from_mat4_to_quat(mat4 m)
{
	quat res;

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
from_euler_angle_to_quat_world(euler_angle e)
{
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
from_euler_angle_to_quat_object(euler_angle e)
{
	quat res = conjugate(from_euler_angle_to_quat_world(e));
	return(res);
}
#if 0
inline euler_angle
from_quat_to_euler_angle(quat q)
{
	euler_angle res;
	return(res);
}
#endif
