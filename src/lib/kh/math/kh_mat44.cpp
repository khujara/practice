inline mat4
m44(v4 col_0, v4 col_1, v4 col_2, v4 col_3)
{
	mat4 res;
	res.c0 = col_0;
	res.c1 = col_1;
	res.c2 = col_2;
	res.c3 = col_3;
	return(res);
}

inline mat4
m44(mat3 basis, v3 tr)
{

	mat4 res;
	res.c0 = kh_vec4(basis.c0, 0.0f);
	res.c1 = kh_vec4(basis.c1, 0.0f);
	res.c2 = kh_vec4(basis.c2, 0.0f);
	res.c3 = kh_vec4(tr, 1.0f);
	return(res);

}

inline mat4
m44(f32 m00, f32 m01, f32 m02, f32 m03,
	f32 m10, f32 m11, f32 m12, f32 m13,
	f32 m20, f32 m21, f32 m22, f32 m23,
	f32 m30, f32 m31, f32 m32, f32 m33)
{
	mat4 res;
	res.c0 = kh_vec4(m00, m01, m02, m03);
	res.c1 = kh_vec4(m10, m11, m12, m13);
	res.c2 = kh_vec4(m20, m21, m22, m23);
	res.c3 = kh_vec4(m30, m31, m32, m33);
	return(res);
}

inline mat4
m44(v3 right, v3 up, v3 forward, v3 tr)
{
	mat4 res;
	res.c0 = kh_vec4(right, 0.0f);
	res.c1 = kh_vec4(up, 0.0f);
	res.c2 = kh_vec4(forward, 0.0f);
	res.c3 = kh_vec4(tr, 1.0f);
	return(res);
}

inline mat4
operator*(mat4 a, mat4 b)
{
	mat4 res;

	res.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20  + a.m03 * b.m30;
	res.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21  + a.m03 * b.m31;
	res.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22  + a.m03 * b.m32;
	res.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23  + a.m03 * b.m33;

	res.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20  + a.m13 * b.m30;
	res.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21  + a.m13 * b.m31;
	res.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22  + a.m13 * b.m32;
	res.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23  + a.m13 * b.m33;

	res.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20  + a.m23 * b.m30;
	res.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21  + a.m23 * b.m31;
	res.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22  + a.m23 * b.m32;
	res.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23  + a.m23 * b.m33;

	res.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20  + a.m33 * b.m30;
	res.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21  + a.m33 * b.m31;
	res.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22  + a.m33 * b.m32;
	res.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23  + a.m33 * b.m33;

	return(res);
}

inline mat4
operator*(mat4 a, f32 b)
{
	mat4 res;
	res.c0 = a.c0 * b;
	res.c1 = a.c1 * b;
	res.c2 = a.c2 * b;
	res.c3 = a.c3 * b;
	return(res);
}

inline v3
operator*(mat4 a, v3 b)
{
	// NOTE(flo) : Assume w == 1 so b is a vector not a dir
	v3 res;
	res.x = b.x * a.m00 + b.y * a.m10 + b.z * a.m20 + a.m30;
	res.y = b.x * a.m01 + b.y * a.m11 + b.z * a.m21 + a.m31;
	res.z = b.x * a.m02 + b.y * a.m12 + b.z * a.m22 + a.m32;

	return(res);
}

inline v3
kh_mul_mat4_dir(mat4 a, v3 b)
{
	v3 res;

	res.x = b.x * a.m00 + b.y * a.m10 + b.z * a.m20;
	res.y = b.x * a.m01 + b.y * a.m11 + b.z * a.m21;
	res.z = b.x * a.m02 + b.y * a.m12 + b.z * a.m22;

	return(res);
}

inline v4
operator*(mat4 a, v4 b)
{
	v4 res;
	res.x = b.x * a.m00 + b.y * a.m10 + b.z * a.m20 + b.w * a.m30;
	res.y = b.x * a.m01 + b.y * a.m11 + b.z * a.m21 + b.w * a.m31;
	res.z = b.x * a.m02 + b.y * a.m12 + b.z * a.m22 + b.w * a.m32;
	res.w = b.x * a.m03 + b.y * a.m13 + b.z * a.m23 + b.w * a.m33;

	return(res);
}

inline mat4
operator+(mat4 a, mat4 b) {
	mat4 res;
	res.m00 = a.m00 + b.m00; res.m01 = a.m01 + b.m01; res.m02 = a.m02 + b.m02; res.m03 = a.m03 + b.m03;
	res.m10 = a.m10 + b.m10; res.m11 = a.m11 + b.m11; res.m12 = a.m12 + b.m12; res.m13 = a.m13 + b.m13;
	res.m20 = a.m20 + b.m20; res.m21 = a.m21 + b.m21; res.m22 = a.m22 + b.m22; res.m23 = a.m23 + b.m23;
	res.m30 = a.m30 + b.m30; res.m31 = a.m31 + b.m31; res.m32 = a.m32 + b.m32; res.m33 = a.m33 + b.m33;
	return(res);
}

inline mat4 &
operator+=(mat4 &a, mat4 b) {
	a = a + b;
	return(a);
}

inline mat4
kh_identity_mat4()
{
	mat4 res = {};
	res.m00 = 1;
	res.m11 = 1;
	res.m22 = 1;
	res.m33 = 1;
	return(res);
}

inline mat4
kh_transpose_mat4(mat4 m)
{
	mat4 res;
	res.m00 = m.m00;
	res.m01 = m.m10;
	res.m02 = m.m20;
	res.m03 = m.m30;

	res.m10 = m.m01;
	res.m11 = m.m11;
	res.m12 = m.m21;
	res.m13 = m.m31;

	res.m20 = m.m02;
	res.m21 = m.m12;
	res.m22 = m.m22;
	res.m23 = m.m32;

	res.m30 = m.m03;
	res.m31 = m.m13;
	res.m32 = m.m23;
	res.m33 = m.m33;

	return(res);
}

inline v3
kh_forward_mat4(mat4 m)
{
	v3 res = kh_vec3(m.c2.x, m.c2.y, m.c2.z);
	return(res);
}

inline v3
kh_up_mat4(mat4 m)
{
	v3 res = kh_vec3(m.c1.x, m.c1.y, m.c1.z);
	return(res);
}

inline v3
kh_right_mat4(mat4 m)
{
	v3 res = kh_vec3(m.c0.x, m.c0.y, m.c0.z);
	return(res);
}

inline v3
kh_get_translation_mat4(mat4 m)
{
	v3 res;
	res = kh_vec3(m.c3);
	return(res);
}

inline mat3
kh_get_rot_mat4(mat4 m)
{
	mat3 res;
	res.c0 = kh_vec3(m.c0);
	res.c1 = kh_vec3(m.c1);
	res.c2 = kh_vec3(m.c2);
	return(res);
}

inline void
kh_set_translation_mat4(mat4 *m, v3 tr)
{
	m->c3.x = tr.x;
	m->c3.y = tr.y;
	m->c3.z = tr.z;
}

inline mat4
kh_inverse_orthogonal_mat4(mat4 m)
{
	mat4 res;

	mat3 rot = kh_transpose_mat3(kh_get_rot_mat4(m));

	res.c0 = kh_vec4(rot.c0, 0);
	res.c1 = kh_vec4(rot.c1, 0);
	res.c2 = kh_vec4(rot.c2, 0);

	v3 tr = kh_get_translation_mat4(m);

	res.c3 = kh_vec4(-tr, 1);

	return(res);
}

inline void
kh_cofactors_mat4(mat4 m) {
	NOT_IMPLEMENTED;
}

inline void
kh_minors_mat4(mat4 m) {
	NOT_IMPLEMENTED;
}

inline f32
kh_determinant_mat4(mat4 m) {
	float res;
	NOT_IMPLEMENTED;
	return(res);
}

inline mat4
kh_inverse_mat4(mat4 m) {
	mat4 res = {};
	NOT_IMPLEMENTED;
	return(res);
}

inline mat4
kh_rotate_around_mat4(f32 a, v3 v)
{
	mat4 res;

	f32 c = kh_cos_f32(a);
	f32 s = kh_sin_f32(a);
	f32 t = 1.0f - c;
	f32 xx = v.x * v.x;
	f32 xy = v.x * v.y;
	f32 xz = v.x * v.z;
	f32 sx = v.x*s;
	f32 yy = v.y * v.y;
	f32 yz = v.y * v.z;
	f32 sy = v.y*s;
	f32 zz = v.z * v.z;
	f32 sz = v.z*s;

	res.c0 = kh_vec4(c + t*xx,  t*xy - sz, t*xz + sy, 0);
	res.c1 = kh_vec4(t*xy + sz, c + t*yy,  t*yz - sx, 0);
	res.c2 = kh_vec4(t*xz - sy, t*yz + sx, c + t*zz,  0);
	res.c3 = kh_vec4(0,0,0,1);

	return (res);
}

inline mat4
kh_get_mat4_from_sqt(v3 pos, quat rot, v3 scale) {
	mat4 res = from_quat_to_mat4(rot);
	res.c0.x *= scale.x; res.c0.y *= scale.x; res.c0.z *= scale.x;
	res.c1.x *= scale.y; res.c1.y *= scale.y; res.c1.z *= scale.y;
	res.c2.x *= scale.z; res.c2.y *= scale.z; res.c2.z *= scale.z;
	res.c3 = kh_vec4(pos, 1.0f);
	return(res);
}