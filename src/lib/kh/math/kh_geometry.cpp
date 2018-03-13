KH_INLINE Rect
create_rect(v2 pos, v2 size) {
	Rect res;
	res.min = pos;
	res.max = pos + size;
	return(res);
}


inline Recti
rect_union(Recti a, Recti b) {
	Recti res;
	res.min_x = (a.min_x < b.min_x) ? a.min_x : b.min_x;
	res.min_y = (a.min_y < b.min_y) ? a.min_y : b.min_y;
	res.max_x = (a.max_x > b.max_x) ? a.max_x : b.max_x;
	res.max_y = (a.max_y > b.max_y) ? a.max_y : b.max_y;
	return(res);
}

inline Rect
rect_union(Rect a, Rect b) {
	Rect res;
	res.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
	res.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
	res.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
	res.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;
	return(res);
}

KH_INLINE v2i
get_recti_dim(Recti a) {
	v2i res;
	res.x = a.max_x - a.min_x;
	res.y = a.max_y - a.min_y;
	return(res);
}

KH_INLINE b32
recti_has_area(Recti a) {
	b32 res = ((a.min_x < a.max_x) && (a.min_y < a.max_y));
	return(res);
}

KH_INLINE b32
rect_has_area(Rect a) {
	b32 res = ((a.min.x < a.max.x) && (a.min.y < a.max.y));
	return(res);
}

inline Plane 
compute_plane(v3 a, v3 b, v3 c) {
	Plane res;
	res.normal = kh_normalize_v3(kh_cross_v3(b - a, c - a));
	res.d = kh_dot_v3(res.normal, a);
	return(res);
}

inline AABB
compute_aabb(v3 min, v3 max) {
	AABB res;
	res.min = min;
	res.max = max;
	return(res);
}

inline Triangle
compute_triangle(v3 a, v3 b, v3 c) {
	Triangle res;
	res.v_0 = a;
	res.v_1 = b;
	res.v_2 = c;
	return(res);
}

inline AABB
aabb_union(AABB a, AABB b) {
	AABB res;
	res.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
	res.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
	res.min.z = (a.min.z < b.min.z) ? a.min.z : b.min.z;
	res.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
	res.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;
	res.max.z = (a.max.z > b.max.z) ? a.max.z : b.max.z;
	return(res);

}

inline AABB
get_sphere_bounding_box(Sphere *sphere) {
	AABB res;
	v3 radius = kh_vec3(sphere->radius, sphere->radius, sphere->radius);
	res = compute_aabb(sphere->center - radius, sphere->center + radius);
	return(res);
}

inline AABB
get_triangle_bounding_box(Triangle *tri) {
	AABB res;
	f32 min_x = kh_min_f32(kh_min_f32(tri->v_0.x, tri->v_1.x), tri->v_2.x);
	f32 min_y = kh_min_f32(kh_min_f32(tri->v_0.y, tri->v_1.y), tri->v_2.y);
	f32 min_z = kh_min_f32(kh_min_f32(tri->v_0.z, tri->v_1.z), tri->v_2.z);
	f32 max_x = kh_max_f32(kh_max_f32(tri->v_0.x, tri->v_1.x), tri->v_2.x);
	f32 max_y = kh_max_f32(kh_max_f32(tri->v_0.y, tri->v_1.y), tri->v_2.y);
	f32 max_z = kh_max_f32(kh_max_f32(tri->v_0.z, tri->v_1.z), tri->v_2.z);

	res = compute_aabb(kh_vec3(min_x, min_y, min_z), kh_vec3(max_x, max_y, max_z));
	return(res);
}