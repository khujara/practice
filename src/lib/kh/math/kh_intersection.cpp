inline b32
test_plane_sphere(plane p, Sphere s)
{
}

inline b32
test_planee_aabb(plane p, aabb box)
{
}


inline b32
test_ray_triangle(ray r, triangle t)
{
}
	
inline b32
test_ray_aabb(ray r, aabb box)
{
}
	
inline b32
test_ray_sphere(ray r, Sphere s)
{
}

inline b32
test_point_sphere(v3 p, Sphere s)
{

}

inline b32
test_point_circle(v2 point, circle c)
{

}

inline b32
test_point_triangle(v3 point, triangle t)
{

}

inline b32
test_point_rect(v2 point, v2 rect_min, v2 rect_max)
{
	b32 res = ((point.x >= rect_min.x) &&
		(point.y >= rect_min.y) &&
		(point.x < rect_max.x) &&
		(point.y < rect_max.y));
	return(res);
}

inline b32
test_point_rect(v2 p, rect r)
{
	b32 res = test_point_rect(p, r.min, r.max);
	return(res);
}

inline b32
test_aabb_aabb(aabb a, aabb b)
{
	b32 res = false;

	if(a.max.x < b.min.x || a.min.x > b.max.x) return(res);
	if(a.max.z < b.min.z || a.min.z > b.max.z) return(res);
	if(a.max.y < b.min.y || a.min.y > b.max.y) return(res);
	res = true;
	return(res);
}

inline b32
test_aabb_plane(aabb box, plane pl, v3 center)
{
	v3 e = box.max - center;

	f32 r = e.x*kh_abs_f32(pl.normal.x) + e.y*kh_abs_f32(pl.normal.y) + e.z*kh_abs_f32(pl.normal.z);
	f32 s = kh_dot_v3(pl.normal, center) - pl.d;

	b32 res = kh_abs_f32(s) <= r;
	return(res); 
}

inline b32
test_aabb_plane(aabb box, plane pl)
{
	v3 c = (box.max + box.min) * 0.5f;
	b32 res = test_aabb_plane(box, pl, c);
	return(res);
}

inline b32
test_aabb_triangle(aabb box, triangle tri)
{
	v3 c = (box.min + box.max) * 0.5f;
	v3 e = (box.max - box.min) * 0.5f;

	v3 v0 = tri.a - c;
	v3 v1 = tri.b - c;
	v3 v2 = tri.c - c;

	v3 f10 = v1 - v0, f21 = v2 - v1, f02 = v0 - v2;

	f32 p0 = v0.z*v1.y - v0.y*v1.z;
	f32 p2 = v2.z*(v1.y - v0.y) - v2.y*(v1.z - v0.z);
	f32 r = e.y * kh_abs_f32(f10.z) + e.z * kh_abs_f32(f10.y);

	if(kh_max_f32(-kh_max_f32(p0, p2), kh_min_f32(p0, p2)) > r) return(0);

	if((kh_max_f32(v0.x, kh_max_f32(v1.x, v2.x)) < -e.x) || (kh_min_f32(v0.x, kh_min_f32(v1.x, v2.x)) > e.x)) return(0);
	if((kh_max_f32(v0.y, kh_max_f32(v1.y, v2.y)) < -e.y) || (kh_min_f32(v0.y, kh_min_f32(v1.y, v2.y)) > e.y)) return(0);
	if((kh_max_f32(v0.z, kh_max_f32(v1.z, v2.z)) < -e.z) || (kh_min_f32(v0.z, kh_min_f32(v1.z, v2.z)) > e.z)) return(0);

	plane p;
	p.normal = kh_cross_v3(f10, f21);
	p.d = kh_dot_v3(p.normal, v0);

	aabb b2;
	b2.min = box.min - c;
	b2.max = box.max - c;
	b32 res = test_aabb_plane(b2, p);
	return(res);
}

inline recti
rect_intersect(recti a, recti b)
{
	recti res;
	res.min_x = (a.min_x < b.min_x) ? b.min_x : a.min_x;
	res.min_y = (a.min_y < b.min_y) ? b.min_y : a.min_y;
	res.max_x = (a.max_x > b.max_x) ? b.max_x : a.max_x;
	res.min_y = (a.min_y > b.min_y) ? b.min_y : a.min_y;
	return(res);
}

inline recti
rect_union(recti a, recti b)
{
	recti res;
	res.min_x = (a.min_x < b.min_x) ? a.min_x : b.min_x;
	res.min_y = (a.min_y < b.min_y) ? a.min_y : b.min_y;
	res.max_x = (a.max_x > b.max_x) ? a.max_x : b.max_x;
	res.min_y = (a.min_y > b.min_y) ? a.min_y : b.min_y;
	return(res);
}

inline rect
rect_intersect(rect a, rect b)
{
	rect res;
	res.min.x = (a.min.x < b.min.x) ? b.min.x : a.min.x;
	res.min.y = (a.min.y < b.min.y) ? b.min.y : a.min.y;
	res.max.x = (a.max.x > b.max.x) ? b.max.x : a.max.x;
	res.min.y = (a.min.y > b.min.y) ? b.min.y : a.min.y;
	return(res);
}

inline rect
rect_union(rect a, rect b)
{
	rect res;
	res.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
	res.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
	res.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
	res.min.y = (a.min.y > b.min.y) ? a.min.y : b.min.y;
	return(res);
}