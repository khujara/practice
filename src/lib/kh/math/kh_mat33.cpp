inline mat3
m33(v3 col_0, v3 col_1, v3 col_2)
{
	mat3 res;
	res.c0 = col_0;
	res.c1 = col_1;
	res.c2 = col_2;
	return(res);
}

inline mat3
m33(f32 m00, f32 m01, f32 m02, f32 m10, f32 m11, f32 m12, f32 m20, f32 m21, f32 m22)
{
	mat3 res;
	res.c0 = kh_vec3(m00, m01, m02);
	res.c1 = kh_vec3(m10, m11, m12);
	res.c2 = kh_vec3(m20, m21, m22);
	return(res);
}

inline mat3
operator*(mat3 a, mat3 b)
{
	mat3 res;

	res.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20;
	res.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21;
	res.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22;

	res.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20;
	res.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21;
	res.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22;

	res.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20;
	res.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21;
	res.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22;

	return(res);
}

inline v3
operator*(mat3 a, v3 b)
{
	v3 res;
	res.x = b.x * a.m00 + b.y * a.m10 + b.z * a.m20;
	res.y = b.x * a.m01 + b.y * a.m11 + b.z * a.m21;
	res.z = b.x * a.m02 + b.y * a.m12 + b.z * a.m22;
	return(res);
}

inline mat3
kh_identity_mat3()
{
	mat3 res = {};
	res.m00 = 1;
	res.m11 = 1;
	res.m22 = 1;
	return(res);
}


inline mat3
kh_transpose_mat3(mat3 m)
{
	mat3 res;

	res.m00 = m.m00;
	res.m01 = m.m10;
	res.m02 = m.m20;

	res.m10 = m.m01;
	res.m11 = m.m11;
	res.m12 = m.m21;

	res.m20 = m.m02;
	res.m21 = m.m12;
	res.m22 = m.m22;

	return(res);
}


inline v3
kh_right_mat3(mat3 m)
{
	v3 res = kh_vec3(m.c0.x, m.c0.y, m.c0.z);
	return(res);
}

