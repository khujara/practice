KH_INLINE rect
create_rect(v2 pos, v2 size)
{
	rect res;
	res.min = pos;
	res.max = pos + size;
	return(res);
}

KH_INLINE v2i
get_recti_dim(recti a)
{
	v2i res;
	res.x = a.max_x - a.min_x;
	res.y = a.max_y - a.min_y;
	return(res);
}

KH_INLINE b32
recti_has_area(recti a)
{
	b32 res = ((a.min_x < a.max_x) && (a.min_y < a.max_y));
	return(res);
}

KH_INLINE b32
rect_has_area(rect a)
{
	b32 res = ((a.min.x < a.max.x) && (a.min.y < a.max.y));
	return(res);
}

inline plane 
compute_plane(v3 a, v3 b, v3 c)
{
	plane res;
	res.normal = kh_normalize_v3(kh_cross_v3(b - a, c - a));
	res.d = kh_dot_v3(res.normal, a);
	return(res);
}

inline aabb
compute_aabb(v3 min, v3 max)
{
	aabb res;
	res.min = min;
	res.max = max;
	return(res);
}

inline triangle
compute_triangle(v3 a, v3 b, v3 c)
{
	triangle res;
	res.a = a;
	res.b = b;
	res.c = c;
	return(res);
}
