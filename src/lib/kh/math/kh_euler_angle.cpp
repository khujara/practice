inline v3
axis_angle_rotation_test(v3 a, v3 b, f32 angle) {
	v3 p = kh_orth_proj_v3(a, b);
	v3 e = a - p;
	v3 f = kh_cross_v3(b, a);
	v3 res = p + e*kh_cos_f32(angle) + f*kh_sin_f32(angle);
	return(res);
}

inline v3
axis_angle_rotation(v3 a, v3 b, f32 angle) {
	f32 cos_a = kh_cos_f32(angle);
	v3 res = a * cos_a + kh_dot_v3(a, b)*b*(1.0f - cos_a) + kh_cross_v3(b, a)*kh_sin_f32(angle);
	return(res);
}

inline v3
from_euler_angle_z(EulerAngle euler) {
	v3 res;
	res.z = kh_cos_f32(euler.yaw)*kh_cos_f32(euler.pitch);
	res.y = kh_sin_f32(euler.pitch);
	res.x = kh_sin_f32(euler.yaw)*kh_cos_f32(euler.pitch);
	return(res);
}

inline v3
from_euler_angle_x(EulerAngle euler) {
	v3 res;
	res.x = kh_cos_f32(euler.yaw)*kh_cos_f32(euler.pitch);
	res.y = kh_sin_f32(euler.pitch);
	res.z = kh_sin_f32(euler.yaw) * kh_cos_f32(euler.pitch);
	return(res);
}

inline EulerAngle
normalize_euler_angle(EulerAngle euler) {
	EulerAngle res = euler;
	if((res.pitch * TO_DEGREES) > 89) {
		res.pitch = 89 * TO_RADIANS;
	}
	if((res.pitch * TO_DEGREES) < -89) {
		res.pitch = -89 * TO_RADIANS;
	}

	while((res.yaw * TO_DEGREES) < -180) {
		res.yaw += 360 * TO_RADIANS;
	}
	while((res.yaw * TO_DEGREES) > 180)	{
		res.yaw -= 360 * TO_RADIANS;
	}
	return(res);
}

inline f32
wrap_pi(f32 a) {
	f32 res = a;
	if(kh_abs_f32(a) <= PI32) {
		i32 revolutions = kh_floor_f32_to_i32((a + PI32) * (1.0f / TAU32));
		res -= revolutions*TAU32;
	}
	return(res);
}

inline EulerAngle
euler_from_quat_norm(quat q) {
	EulerAngle res;
	f32 test = q.x*q.y + q.z*q.w;
	if(test > 0.499f) {
		res.pitch = 2.0f * kh_atan2_f32(q.x, q.w);
		res.yaw = PI32 * 0.5f;
		res.roll = 0.0f;
	} else if (test < -0.499f) {
		res.pitch = -2.0f * kh_atan2_f32(q.x, q.w);
		res.yaw = -PI32 * 0.5f;
		res.roll = 0.0f;
	} else {
		f32 sqx = q.x*q.x;
		f32 sqy = q.y*q.y;
		f32 sqz = q.z*q.z;
		res.pitch = kh_atan2_f32(2.0f*q.y*q.w - 2.0f*q.x*q.z, 1.0f - 2.0f*sqy - 2.0f*sqz);
		res.yaw = kh_asin_f32(2.0f * test);
		res.roll = kh_atan2_f32(2.0f*q.x*q.w - 2.0f*q.y*q.z, 1.0f - 2*sqx - 2.0f*sqz);
	}
	return(res);
}

inline EulerAngle
euler_from_quat(quat q) {
	EulerAngle res;
	f32 sqw = q.w*q.w;
	f32 sqx = q.x*q.x;
	f32 sqy = q.y*q.y;
	f32 sqz = q.z*q.z;
	f32 unit = sqx + sqy + sqz + sqw;
	f32 test = q.x*q.y + q.z*q.w;
	if(test > 0.499f) {
		res.pitch = 2.0f * kh_atan2_f32(q.x, q.w);
		res.yaw = PI32 * 0.5f;
		res.roll = 0.0f;
	} else if (test < -0.499f) {
		res.pitch = -2.0f * kh_atan2_f32(q.x, q.w);
		res.yaw = -PI32 * 0.5f;
		res.roll = 0.0f;
	} else {
		res.pitch = kh_atan2_f32(2.0f*q.y*q.w - 2.0f*q.x*q.z, sqx - sqy - sqz + sqw);
		res.yaw = kh_asin_f32(2.0f * (test/unit));
		res.roll = kh_atan2_f32(2.0f*q.x*q.w - 2.0f*q.y*q.z, -sqx + sqy - sqz + sqw);
	}
	return(res);
}