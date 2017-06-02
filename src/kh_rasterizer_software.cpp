#define VERT_PER_TRI 3
#define VERT_PER_QUAD 4

struct TriangleEdge
{
	i32 start_y, end_y;
	f32 x, step_x;

	f32 texcoord_x, texcoord_xstep;
	f32 texcoord_y, texcoord_ystep;
	f32 one_over_z, one_over_zstep;
	f32 depth, depth_step;
	f32 light, light_step;
};

struct Vertex_SSE
{
	__m128 pos_x;
	__m128 pos_y;
	__m128 pos_z;

	__m128 texcoord_x;
	__m128 texcoord_y;

	__m128 normal_x;
	__m128 normal_y;
	__m128 normal_z;
};

#if DEBUG_SSE
#include <iacaMarks.h>
#else
#define IACA_VC64_START
#define IACA_VC64_END
#endif

#define BLOCK_RASTER_SIZE 4 //NOTE(flo): do not touch this!

// TODO(flo): texture bilinear sampling, gamma correction, premultipled alpha
KH_INLINE void
rasterize_triangle_SSE_4x4(SoftwareFrameBuffer *target, Assets *assets, AssetID diffuse_id, v4 color, Vertex_SSE vert_0, 
                           Vertex_SSE vert_1, Vertex_SSE vert_2, DirectionalLight *dirlight, const u32 src_tri_count = 4)
{
	// NOTE(flo): we compute values (interpolants, deltas...) for our half edges function for src_tri_count triangles first
	LoadedAsset loaded_diffuse = get_loaded_asset(assets, diffuse_id, AssetType_tex2d);
	const Texture2D diffuse = loaded_diffuse.type->tex2d;
	const u8 *diffuse_memory = loaded_diffuse.data;

	u32 pitch = target->pixels.pitch;
	u32 target_w = target->pixels.w;
	u32 src_pitch = diffuse.width * diffuse.bytes_per_pixel;

	__m128 one_SSE = _mm_set1_ps(1.0f);
	__m128 zero_SSE = _mm_set1_ps(0.0f);
	__m128 half_SSE = _mm_set1_ps(0.5f);
	__m128i zero_SSEi = _mm_set1_epi32(0);
	__m128i one_SSEi = _mm_set1_epi32(1);
	__m128 inv_255_SSE = _mm_set1_ps((f32)ONE_OVER_255);
	__m128 one_255_SSE = _mm_set1_ps(255.0f);

	__m128i w_SSE = _mm_set1_epi32(target->pixels.w - 1);
	__m128i h_SSE = _mm_set1_epi32(target->pixels.h - 1);

	__m128 w_m1 = _mm_set1_ps((f32)(diffuse.width - 1));
	__m128 h_m1 = _mm_set1_ps((f32)(diffuse.height - 1));
	__m128i t_pitch = _mm_set1_epi32((diffuse.bytes_per_pixel * diffuse.width));
	__m128i t_bpp = _mm_set1_epi32(diffuse.bytes_per_pixel);

	__m128i mask_ff000000 = _mm_set1_epi32(0xFF000000);
	__m128i mask_ff = _mm_set1_epi32(0xFF);
	__m128i mask_ffff = _mm_set1_epi32(0xFFFF);
	__m128i mask_ffffffff = _mm_set1_epi32(0xFFFFFFFF);

	__m128 color_r = _mm_set1_ps(color.r);
	__m128 color_g = _mm_set1_ps(color.g);
	__m128 color_b = _mm_set1_ps(color.b);
	__m128 color_a = _mm_set1_ps(color.a);

	__m128 light_dir_x = _mm_set1_ps(-dirlight->dir.x);
	__m128 light_dir_y = _mm_set1_ps(-dirlight->dir.y);
	__m128 light_dir_z = _mm_set1_ps(-dirlight->dir.z);

	__m128 ambient_coeff = _mm_set1_ps(0.1f);
	__m128 light_coeff = _mm_set1_ps(0.9f);

	__m128 depth[3] = { vert_0.pos_z, vert_1.pos_z, vert_2.pos_z };
	__m128 one_over_z[3] = { _mm_div_ps(one_SSE, vert_0.pos_z), _mm_div_ps(one_SSE, vert_1.pos_z), 
		_mm_div_ps(one_SSE, vert_2.pos_z) };
	__m128 texcoord_x[3] = { _mm_mul_ps(one_over_z[0], vert_0.texcoord_x), _mm_mul_ps(one_over_z[1], vert_1.texcoord_x),
		_mm_mul_ps(one_over_z[2], vert_2.texcoord_x) };
	__m128 texcoord_y[3] = { _mm_mul_ps(one_over_z[0], vert_0.texcoord_y), _mm_mul_ps(one_over_z[1], vert_1.texcoord_y),
		_mm_mul_ps(one_over_z[2], vert_2.texcoord_y) };

	__m128 dot_0_x = _mm_mul_ps(vert_0.normal_x, light_dir_x);
	__m128 dot_0_y = _mm_mul_ps(vert_0.normal_y, light_dir_y);
	__m128 dot_0_z = _mm_mul_ps(vert_0.normal_z, light_dir_z);
	__m128 dot_0_SSE = _mm_add_ps(_mm_add_ps(dot_0_x, dot_0_y), dot_0_z);
	__m128 saturate_0_SSE = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, dot_0_SSE));
	__m128 light_0 = _mm_add_ps(_mm_mul_ps(saturate_0_SSE, light_coeff), ambient_coeff);

	__m128 dot_1_x = _mm_mul_ps(vert_1.normal_x, light_dir_x);
	__m128 dot_1_y = _mm_mul_ps(vert_1.normal_y, light_dir_y);
	__m128 dot_1_z = _mm_mul_ps(vert_1.normal_z, light_dir_z);
	__m128 dot_1_SSE = _mm_add_ps(_mm_add_ps(dot_1_x, dot_1_y), dot_1_z);
	__m128 saturate_1_SSE = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, dot_1_SSE));
	__m128 light_1 = _mm_add_ps(_mm_mul_ps(saturate_1_SSE, light_coeff), ambient_coeff);

	__m128 dot_2_x = _mm_mul_ps(vert_2.normal_x, light_dir_x);
	__m128 dot_2_y = _mm_mul_ps(vert_2.normal_y, light_dir_y);
	__m128 dot_2_z = _mm_mul_ps(vert_2.normal_z, light_dir_z);
	__m128 dot_2_SSE = _mm_add_ps(_mm_add_ps(dot_2_x, dot_2_y), dot_2_z);
	__m128 saturate_2_SSE = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, dot_2_SSE));
	__m128 light_2 = _mm_add_ps(_mm_mul_ps(saturate_2_SSE, light_coeff), ambient_coeff);

	__m128 light[3] = { light_0, light_1, light_2 };
	// __m128 light[3] = { _mm_set1_ps(1.0f), _mm_set1_ps(1.0f), _mm_set1_ps(1.0f) };

	__m128 x0_SSE = vert_0.pos_x;
	__m128 y0_SSE = vert_0.pos_y;
	__m128 x1_SSE = vert_1.pos_x;
	__m128 y1_SSE = vert_1.pos_y;
	__m128 x2_SSE = vert_2.pos_x;
	__m128 y2_SSE = vert_2.pos_y;

	__m128 x1_minus_x0_SSE = _mm_sub_ps(x1_SSE, x0_SSE);
	__m128 x2_minus_x0_SSE = _mm_sub_ps(x2_SSE, x0_SSE);
	__m128 y1_minus_y0_SSE = _mm_sub_ps(y1_SSE, y0_SSE);
	__m128 y2_minus_y0_SSE = _mm_sub_ps(y2_SSE, y0_SSE);

	__m128 triangle_area_times_two = _mm_sub_ps(_mm_mul_ps(x1_minus_x0_SSE, y2_minus_y0_SSE), 
		_mm_mul_ps(x2_minus_x0_SSE, y1_minus_y0_SSE));

	u32 sub_pixel_bits = 4;

	__m128i max_triangle_area = _mm_set1_epi32(0xFF);

	__m128i abs_tri_area = _mm_abs_epi32(_mm_cvtps_epi32(triangle_area_times_two));
	__m128i sub_pixel_mask = _mm_cmplt_epi32(abs_tri_area, max_triangle_area);

	if(!_mm_test_all_ones(sub_pixel_mask))
	{
		sub_pixel_bits = 4;
	}

	i32 sub_pixel_precision = 1 << sub_pixel_bits; 
	__m128 sp_pre_SSE = _mm_set1_ps((f32)sub_pixel_precision);
	__m128i sp_pre_SSEi = _mm_set1_epi32(sub_pixel_precision);

	__m128i x0_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, x0_SSE));
	__m128i y0_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, y0_SSE));
	__m128i x1_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, x1_SSE));
	__m128i y1_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, y1_SSE));
	__m128i x2_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, x2_SSE));
	__m128i y2_fp_SSE = _mm_cvtps_epi32(_mm_mul_ps(sp_pre_SSE, y2_SSE));

	__m128 one_over_dx_SSE = _mm_div_ps(one_SSE, triangle_area_times_two);
	__m128 one_over_dy_SSE = _mm_sub_ps(zero_SSE, one_over_dx_SSE);

	/* NOTE(flo) : Interpolant
		Linear interpolation between points a line :
 		(y - y0) / (x - x0) = (y1 - y0) / (x1 - x0)
		or (x - x0) / (y - y0) = (x1 - x0) / (y1 - y0)

		in our case :
		x1 = vert_1.pos.x
		y1 = vert_1.pos.y
		x0 = vert_0.pos.x
		y0 = vert_0.pos.y
		x2 = vert_2.pos.x
		y2 = vert_2.pos.y


		we want x when y = vert_0.pos.y so y = y0
		so : (x - x0) / (y2 - y0) = (x1 - x0) / (y1 - y0)
		multiply both side by (y0 - y2) we have :
				x - x0 = (x1 - x0) / (y1 - y0) * (y2 - y0);
				x = (x1 - x0) / (y1 - y0) * (y2 - y0) + x0;
				x = vert_0.pos.x + (vert_2.pos.y - vert_0.pos.y) *
					(vert_1.pos.x - vert_0.pos.x) / (vert_1.pos.y - vert_0.pos.y);
		we can derive to have our linear blend such as a + t*(b - a) if we switch division/mul :
			x = x0 + ((y2 - y0) / (y1 - y0)) * (x1 - x0);
			t = (y2 - y0) / (y1 - y0);
			x = linear_blend(x0, x1, t);

		we have C as our interpolant such as
			(C - c0) / (y - y0) = (c1 - c0) / (y1 - y0)
		we want to interpolate over dx = C - c2 / x - x2
		same equation for C since we want C when y = vert_2.pos.y or y = y2
			C = c0 + (y2 - y0) * (c1 - c0) / (y1 - y0);
			C = c0 + ((y2 - y0) / (y1 - y0)) * (c1 - c0)
			C = linear_blend(c2, c0, t);

		dc = linear_blend(c0, c1, t) - c2;
		dx = linear_blend(x0, x1, t) - x2;

		we can derive to find :
		dc/dx = (((c1 - c0) * (y2 - y0)) + ((c0 - c2) * (y1 - y0))) /
			(((x1 - x0) * (y2 - y0)) + ((x0 - x2) * (y1 - y0)))
		or
		dc/dx = (((c1 - c0) * (y2 - y0)) - ((c2 - c0) * (y1 - y0))) /
			(((x1 - x0) * (y2 - y0)) - ((x2 - x0) * (y1 - y0)))

		if we derive for dy we have :
		dc/dy = (((c1 -c0) * (x2 - x0)) + ((c0 - c2) * (x1 - x0))) /
			(((x2 - x0) * (y1 - y0)) + ((x0 - x1) * (y2 - y0)))
		or
		dc/dy = (((c1 -c0) * (x2 - x0)) - ((c2 - c0) * (x1 - x0))) /
			(((x2 - x0) * (y1 - y0)) - ((x1 - x0) * (y2 - y0)))
		so dy = -dx;
	*/

	// NOTE(flo): texcoord u
	__m128 c1_minus_c0_SSE = _mm_sub_ps(texcoord_x[1], texcoord_x[0]);
	__m128 c2_minus_c0_SSE = _mm_sub_ps(texcoord_x[2], texcoord_x[0]);
	__m128 a_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, y2_minus_y0_SSE), _mm_mul_ps(c2_minus_c0_SSE, y1_minus_y0_SSE));
	__m128 b_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, x2_minus_x0_SSE), _mm_mul_ps(c2_minus_c0_SSE, x1_minus_x0_SSE));
	__m128 u_stepx_SSE = _mm_mul_ps(a_SSE, one_over_dx_SSE);
	__m128 u_stepy_SSE = _mm_mul_ps(b_SSE, one_over_dy_SSE);

	// NOTE(flo): texcoord v
	c1_minus_c0_SSE = _mm_sub_ps(texcoord_y[1], texcoord_y[0]);
	c2_minus_c0_SSE = _mm_sub_ps(texcoord_y[2], texcoord_y[0]);
	a_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, y2_minus_y0_SSE), _mm_mul_ps(c2_minus_c0_SSE, y1_minus_y0_SSE));
	b_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, x2_minus_x0_SSE), _mm_mul_ps(c2_minus_c0_SSE, x1_minus_x0_SSE));
	__m128 v_stepx_SSE = _mm_mul_ps(a_SSE, one_over_dx_SSE);
	__m128 v_stepy_SSE = _mm_mul_ps(b_SSE, one_over_dy_SSE);

	// NOTE(flo): perspective correct
	c1_minus_c0_SSE = _mm_sub_ps(one_over_z[1], one_over_z[0]);
	c2_minus_c0_SSE = _mm_sub_ps(one_over_z[2], one_over_z[0]);
	a_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, y2_minus_y0_SSE), _mm_mul_ps(c2_minus_c0_SSE, y1_minus_y0_SSE));
	b_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, x2_minus_x0_SSE), _mm_mul_ps(c2_minus_c0_SSE, x1_minus_x0_SSE));
	__m128 z_stepx_SSE = _mm_mul_ps(a_SSE, one_over_dx_SSE);
	__m128 z_stepy_SSE = _mm_mul_ps(b_SSE, one_over_dy_SSE);

	// NOTE(flo): depth
	c1_minus_c0_SSE = _mm_sub_ps(depth[1], depth[0]);
	c2_minus_c0_SSE = _mm_sub_ps(depth[2], depth[0]);
	a_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, y2_minus_y0_SSE), _mm_mul_ps(c2_minus_c0_SSE, y1_minus_y0_SSE));
	b_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, x2_minus_x0_SSE), _mm_mul_ps(c2_minus_c0_SSE, x1_minus_x0_SSE));
	__m128 d_stepx_SSE = _mm_mul_ps(a_SSE, one_over_dx_SSE);
	__m128 d_stepy_SSE = _mm_mul_ps(b_SSE, one_over_dy_SSE);

	// NOTE(flo): light
	c1_minus_c0_SSE = _mm_sub_ps(light[1], light[0]);
	c2_minus_c0_SSE = _mm_sub_ps(light[2], light[0]);
	a_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, y2_minus_y0_SSE), _mm_mul_ps(c2_minus_c0_SSE, y1_minus_y0_SSE));
	b_SSE = _mm_sub_ps(_mm_mul_ps(c1_minus_c0_SSE, x2_minus_x0_SSE), _mm_mul_ps(c2_minus_c0_SSE, x1_minus_x0_SSE));
	__m128 l_stepx_SSE = _mm_mul_ps(a_SSE, one_over_dx_SSE);
	__m128 l_stepy_SSE = _mm_mul_ps(b_SSE, one_over_dy_SSE);

	// NOTE(flo): deltas preprocessing, triangles setp
	__m128i y1my2_SSE = _mm_sub_epi32(y1_fp_SSE, y2_fp_SSE);
	__m128i y2my0_SSE = _mm_sub_epi32(y2_fp_SSE, y0_fp_SSE);
	__m128i y0my1_SSE = _mm_sub_epi32(y0_fp_SSE, y1_fp_SSE);
	__m128i x2mx1_SSE = _mm_sub_epi32(x2_fp_SSE, x1_fp_SSE);
	__m128i x0mx2_SSE = _mm_sub_epi32(x0_fp_SSE, x2_fp_SSE);
	__m128i x1mx0_SSE = _mm_sub_epi32(x1_fp_SSE, x0_fp_SSE);

	// NOTE(flo): half edge constants
	__m128i x1y2_fp_SSE = _mm_mullo_epi32(x1_fp_SSE, y2_fp_SSE);
	__m128i y1x2_fp_SSE = _mm_mullo_epi32(y1_fp_SSE, x2_fp_SSE);
	__m128i x2y0_fp_SSE = _mm_mullo_epi32(x2_fp_SSE, y0_fp_SSE);
	__m128i y2x0_fp_SSE = _mm_mullo_epi32(y2_fp_SSE, x0_fp_SSE);
	__m128i x0y1_fp_SSE = _mm_mullo_epi32(x0_fp_SSE, y1_fp_SSE);
	__m128i y0x1_fp_SSE = _mm_mullo_epi32(y0_fp_SSE, x1_fp_SSE);
	__m128i c0_SSE = _mm_sub_epi32(x1y2_fp_SSE, y1x2_fp_SSE);
	__m128i c1_SSE = _mm_sub_epi32(x2y0_fp_SSE, y2x0_fp_SSE);
	__m128i c2_SSE = _mm_sub_epi32(x0y1_fp_SSE, y0x1_fp_SSE);

	// NOTE(flo): get the rect from the 3 vertices points
	__m128i min_x_fp_SSE =  _mm_max_epi32(_mm_min_epi32(_mm_min_epi32(x0_fp_SSE, x1_fp_SSE), x2_fp_SSE), zero_SSEi);
	__m128i max_x_fp_SSE =  _mm_max_epi32(_mm_max_epi32(x0_fp_SSE, x1_fp_SSE), x2_fp_SSE);
	__m128i min_y_fp_SSE =  _mm_max_epi32(_mm_min_epi32(_mm_min_epi32(y0_fp_SSE, y1_fp_SSE), y2_fp_SSE), zero_SSEi);
	__m128i max_y_fp_SSE =  _mm_max_epi32(_mm_max_epi32(y0_fp_SSE, y1_fp_SSE), y2_fp_SSE);
	__m128i min_x_SSE = _mm_srli_epi32(_mm_add_epi32(min_x_fp_SSE, sp_pre_SSEi), sub_pixel_bits);
	__m128i max_x_SSE = _mm_srli_epi32(_mm_add_epi32(max_x_fp_SSE, sp_pre_SSEi), sub_pixel_bits);
	__m128i min_y_SSE = _mm_srli_epi32(_mm_add_epi32(min_y_fp_SSE, sp_pre_SSEi), sub_pixel_bits);
	__m128i max_y_SSE = _mm_srli_epi32(_mm_add_epi32(max_y_fp_SSE, sp_pre_SSEi), sub_pixel_bits);

	// NOTE(flo): clipping
	min_x_SSE = _mm_max_epi32(min_x_SSE, zero_SSEi);
	max_x_SSE = _mm_min_epi32(max_x_SSE, _mm_sub_epi32(w_SSE, one_SSEi));
	min_y_SSE = _mm_max_epi32(min_y_SSE, zero_SSEi);
	max_y_SSE = _mm_min_epi32(max_y_SSE, _mm_sub_epi32(h_SSE, one_SSEi));
	// NOTE(flo): adjust min and max to our block alignment
	min_x_SSE = _mm_and_si128(min_x_SSE, _mm_set1_epi32(~(BLOCK_RASTER_SIZE - 1)));
	min_y_SSE = _mm_and_si128(min_y_SSE, _mm_set1_epi32(~(BLOCK_RASTER_SIZE - 1)));

	__m128 c_x = _mm_sub_ps(_mm_cvtepi32_ps(min_x_SSE), x0_SSE);
	__m128 c_y = _mm_sub_ps(_mm_cvtepi32_ps(min_y_SSE), y0_SSE);

	__m128 u_base_SSE = _mm_add_ps(texcoord_x[0], _mm_add_ps(_mm_mul_ps(u_stepx_SSE, c_x), _mm_mul_ps(u_stepy_SSE, c_y)));
	__m128 v_base_SSE = _mm_add_ps(texcoord_y[0], _mm_add_ps(_mm_mul_ps(v_stepx_SSE, c_x), _mm_mul_ps(v_stepy_SSE, c_y)));
	__m128 z_base_SSE = _mm_add_ps(one_over_z[0], _mm_add_ps(_mm_mul_ps(z_stepx_SSE, c_x), _mm_mul_ps(z_stepy_SSE, c_y)));
	__m128 d_base_SSE = _mm_add_ps(depth[0], _mm_add_ps(_mm_mul_ps(d_stepx_SSE, c_x), _mm_mul_ps(d_stepy_SSE, c_y)));
	__m128 l_base_SSE = _mm_add_ps(light[0], _mm_add_ps(_mm_mul_ps(l_stepx_SSE, c_x), _mm_mul_ps(l_stepy_SSE, c_y)));

	__m128i offset = _mm_setr_epi32(0,1,2,3);
	__m128 offset_ps = _mm_setr_ps(0,1,2,3);

	u32 shift_inc = ((BLOCK_RASTER_SIZE >> 1) + sub_pixel_bits);
	__m128 inc_ps = _mm_set1_ps((f32)BLOCK_RASTER_SIZE);

	for(u32 tri_i = 0; tri_i < src_tri_count; ++tri_i)
	{
		// i32 min_x = min_x_SSE.m128i_i32[tri_i];
		// i32 min_y = min_y_SSE.m128i_i32[tri_i];
		// i32 max_x = max_x_SSE.m128i_i32[tri_i];
		// i32 max_y = max_y_SSE.m128i_i32[tri_i];
		i32 min_x = *((u32 *)(&min_x_SSE) + tri_i);
		i32 min_y = *((u32 *)(&min_y_SSE) + tri_i);
		i32 max_x = *((u32 *)(&max_x_SSE) + tri_i);
		i32 max_y = *((u32 *)(&max_y_SSE) + tri_i);

		u32 test = src_tri_count - tri_i;

		__m128i y1my2 = _mm_set1_epi32(*(((u32 *)&y1my2_SSE) + tri_i));
		__m128i y2my0 = _mm_set1_epi32(*(((u32 *)&y2my0_SSE) + tri_i));
		__m128i y0my1 = _mm_set1_epi32(*(((u32 *)&y0my1_SSE) + tri_i));

		__m128i x2mx1 = _mm_set1_epi32(*(((u32 *)&x2mx1_SSE) + tri_i));
		__m128i x0mx2 = _mm_set1_epi32(*(((u32 *)&x0mx2_SSE) + tri_i));
		__m128i x1mx0 = _mm_set1_epi32(*(((u32 *)&x1mx0_SSE) + tri_i));

		__m128i c0 = _mm_set1_epi32(*(((u32 *)&c0_SSE) +tri_i));
		__m128i c1 = _mm_set1_epi32(*(((u32 *)&c1_SSE) +tri_i));
		__m128i c2 = _mm_set1_epi32(*(((u32 *)&c2_SSE) +tri_i));
	
		// __m128i y1my2 = _mm_set1_epi32(y1my2_SSE.m128i_i32[tri_i]);
		// __m128i y2my0 = _mm_set1_epi32(y2my0_SSE.m128i_i32[tri_i]);
		// __m128i y0my1 = _mm_set1_epi32(y0my1_SSE.m128i_i32[tri_i]);

		// __m128i x2mx1 = _mm_set1_epi32(x2mx1_SSE.m128i_i32[tri_i]);
		// __m128i x0mx2 = _mm_set1_epi32(x0mx2_SSE.m128i_i32[tri_i]);
		// __m128i x1mx0 = _mm_set1_epi32(x1mx0_SSE.m128i_i32[tri_i]);

		// __m128i c0 = _mm_set1_epi32(c0_SSE.m128i_i32[tri_i]);
		// __m128i c1 = _mm_set1_epi32(c1_SSE.m128i_i32[tri_i]);
		// __m128i c2 = _mm_set1_epi32(c2_SSE.m128i_i32[tri_i]);

		//NOTE(flo): correct for fill convention 
		b32 is_top_left_0 = ((y1my2.m128i_i32[tri_i] < 0) || (y1my2.m128i_i32[tri_i] == 0 && x2mx1.m128i_i32[tri_i] > 0));
		b32 is_top_left_1 = ((y2my0.m128i_i32[tri_i] < 0) || (y0my1.m128i_i32[tri_i] == 0 && x0mx2.m128i_i32[tri_i] > 0));
		b32 is_top_left_2 = ((y0my1.m128i_i32[tri_i] < 0) || (y2my0.m128i_i32[tri_i] == 0 && x1mx0.m128i_i32[tri_i] > 0));
		i32 bias_0 = is_top_left_0 ? 0 : -1;
		i32 bias_1 = is_top_left_1 ? 0 : -1;
		i32 bias_2 = is_top_left_2 ? 0 : -1;
		c0 = _mm_add_epi32(c0, _mm_set1_epi32(bias_0));
		c1 = _mm_add_epi32(c1, _mm_set1_epi32(bias_1));
		c2 = _mm_add_epi32(c2, _mm_set1_epi32(bias_2));

		/* NOTE(flo):
			sp = subpixels
			col = [minx + 0 << sp, minx + 1 << sp, minx + 2 << sp, minx + 3 << sp]
			row = [miny + 0 << sp, miny + 1 << sp, miny + 2 << sp, miny + 3 << sp]
		*/ 
		__m128i col = _mm_slli_epi32(_mm_add_epi32(offset, _mm_set1_epi32(min_x)), sub_pixel_bits);
		__m128i row = _mm_slli_epi32(_mm_add_epi32(offset, _mm_set1_epi32(min_y)), sub_pixel_bits);

		// NOTE(flo): compute deltas and increments for each column (x)
		__m128i a0_col = _mm_mullo_epi32(y1my2, col);
		__m128i a1_col = _mm_mullo_epi32(y2my0, col);
		__m128i a2_col = _mm_mullo_epi32(y0my1, col);
		__m128i a0_inc = _mm_slli_epi32(y1my2, shift_inc);
		__m128i a1_inc = _mm_slli_epi32(y2my0, shift_inc);
		__m128i a2_inc = _mm_slli_epi32(y0my1, shift_inc);

		/* NOTE(flo): compute deltas and increments for each row (y)
			edge 12 : (y1 - y2) * x + (x2 - x1) * y + (x1 * y2 + x2 * y1)
			c0 = c12 = (x1*y2 - x2*y1)

			a0_col = [(y1 - y2) * (min_x + 0), (y1 - y2) * (min_x + 1), (y1 - y2) * (min_x + 2)...]
			b0_row = [(x2 - x1) * (min_y + 0) + c0, (x2 - x1) * (miny + 1) + c0, (x2 - x1) * (miny + 2) + c0, ...]
		*/
		__m128i b0_row = _mm_add_epi32(_mm_mullo_epi32(x2mx1, row), c0);
		__m128i b1_row = _mm_add_epi32(_mm_mullo_epi32(x0mx2, row), c1);
		__m128i b2_row = _mm_add_epi32(_mm_mullo_epi32(x1mx0, row), c2);
		__m128i b0_inc = _mm_slli_epi32(x2mx1, shift_inc);
		__m128i b1_inc = _mm_slli_epi32(x0mx2, shift_inc);
		__m128i b2_inc = _mm_slli_epi32(x1mx0, shift_inc);

		__m128 advance_x = _mm_setr_ps(0,1,2,3);
		__m128 advance_y = _mm_set1_ps(0.0f);

		// __m128 u_begin = _mm_set1_ps(*(((f32 *)&u_base_SSE) + tri_i));
		// __m128 v_begin = _mm_set1_ps(*(((f32 *)&v_base_SSE) + tri_i));
		// __m128 z_begin = _mm_set1_ps(*(((f32 *)&z_base_SSE) + tri_i));
		// __m128 d_begin = _mm_set1_ps(*(((f32 *)&d_base_SSE) + tri_i));
		// __m128 l_begin = _mm_set1_ps(*(((f32 *)&l_base_SSE) + tri_i));
		// __m128 u_advx  = _mm_set1_ps(*(((f32 *)&u_stepx_SSE) + tri_i));
		// __m128 u_advy  = _mm_set1_ps(*(((f32 *)&u_stepy_SSE) + tri_i));
		// __m128 v_advx  = _mm_set1_ps(*(((f32 *)&v_stepx_SSE) + tri_i));
		// __m128 v_advy  = _mm_set1_ps(*(((f32 *)&v_stepy_SSE) + tri_i));
		// __m128 z_advx  = _mm_set1_ps(*(((f32 *)&z_stepx_SSE) + tri_i));
		// __m128 z_advy  = _mm_set1_ps(*(((f32 *)&z_stepy_SSE) + tri_i));
		// __m128 d_advx  = _mm_set1_ps(*(((f32 *)&d_stepx_SSE) + tri_i));
		// __m128 d_advy  = _mm_set1_ps(*(((f32 *)&d_stepy_SSE) + tri_i));
		// __m128 l_advx  = _mm_set1_ps(*(((f32 *)&l_stepx_SSE) + tri_i));
		// __m128 l_advy  = _mm_set1_ps(*(((f32 *)&l_stepy_SSE) + tri_i));

		__m128 u_begin = _mm_set1_ps(u_base_SSE.m128_f32[tri_i]);
		__m128 v_begin = _mm_set1_ps(v_base_SSE.m128_f32[tri_i]);
		__m128 z_begin = _mm_set1_ps(z_base_SSE.m128_f32[tri_i]);
		__m128 d_begin = _mm_set1_ps(d_base_SSE.m128_f32[tri_i]);
		__m128 l_begin = _mm_set1_ps(l_base_SSE.m128_f32[tri_i]);

		__m128 u_advx = _mm_set1_ps(u_stepx_SSE.m128_f32[tri_i]);
		__m128 u_advy = _mm_set1_ps(u_stepy_SSE.m128_f32[tri_i]);
		__m128 v_advx = _mm_set1_ps(v_stepx_SSE.m128_f32[tri_i]);
		__m128 v_advy = _mm_set1_ps(v_stepy_SSE.m128_f32[tri_i]);
		__m128 z_advx = _mm_set1_ps(z_stepx_SSE.m128_f32[tri_i]);
		__m128 z_advy = _mm_set1_ps(z_stepy_SSE.m128_f32[tri_i]);
		__m128 d_advx = _mm_set1_ps(d_stepx_SSE.m128_f32[tri_i]);
		__m128 d_advy = _mm_set1_ps(d_stepy_SSE.m128_f32[tri_i]);
		__m128 l_advx = _mm_set1_ps(l_stepx_SSE.m128_f32[tri_i]);
		__m128 l_advy = _mm_set1_ps(l_stepy_SSE.m128_f32[tri_i]);


		f32 *zbuffer_row_0 = target->zbuffer.memory + min_x + min_y * target_w;
		f32 *zbuffer_row_1 = zbuffer_row_0 + target_w;
		f32 *zbuffer_row_2 = zbuffer_row_1 + target_w;
		f32 *zbuffer_row_3 = zbuffer_row_2 + target_w;
		u32 zbuffer_pitch = target_w * BLOCK_RASTER_SIZE;

		for(i32 y = min_y; y < max_y; y += BLOCK_RASTER_SIZE) 
		{
			//NOTE(flo): compute fixed point barycentric coordinates for each of the three edges with the 4x4 block area
			__m128i alpha_0 = _mm_add_epi32(a0_col, _mm_set1_epi32(b0_row.m128i_i32[0]));
			__m128i alpha_1 = _mm_add_epi32(a0_col, _mm_set1_epi32(b0_row.m128i_i32[1]));
			__m128i alpha_2 = _mm_add_epi32(a0_col, _mm_set1_epi32(b0_row.m128i_i32[2]));
			__m128i alpha_3 = _mm_add_epi32(a0_col, _mm_set1_epi32(b0_row.m128i_i32[3]));

			__m128i beta_0 = _mm_add_epi32(a1_col, _mm_set1_epi32(b1_row.m128i_i32[0]));
			__m128i beta_1 = _mm_add_epi32(a1_col, _mm_set1_epi32(b1_row.m128i_i32[1]));
			__m128i beta_2 = _mm_add_epi32(a1_col, _mm_set1_epi32(b1_row.m128i_i32[2]));
			__m128i beta_3 = _mm_add_epi32(a1_col, _mm_set1_epi32(b1_row.m128i_i32[3]));

			__m128i gamma_0 = _mm_add_epi32(a2_col, _mm_set1_epi32(b2_row.m128i_i32[0]));
			__m128i gamma_1 = _mm_add_epi32(a2_col, _mm_set1_epi32(b2_row.m128i_i32[1]));
			__m128i gamma_2 = _mm_add_epi32(a2_col, _mm_set1_epi32(b2_row.m128i_i32[2]));
			__m128i gamma_3 = _mm_add_epi32(a2_col, _mm_set1_epi32(b2_row.m128i_i32[3]));

			__m128 u_rowy = _mm_add_ps(u_begin, _mm_mul_ps(u_advy, advance_y));			
			__m128 v_rowy = _mm_add_ps(v_begin, _mm_mul_ps(v_advy, advance_y));			
			__m128 z_rowy = _mm_add_ps(z_begin, _mm_mul_ps(z_advy, advance_y));			
			__m128 d_rowy = _mm_add_ps(d_begin, _mm_mul_ps(d_advy, advance_y));			
			__m128 l_rowy = _mm_add_ps(l_begin, _mm_mul_ps(l_advy, advance_y));			

			f32 *d_0 = zbuffer_row_0;
			f32 *d_1 = zbuffer_row_1;
			f32 *d_2 = zbuffer_row_2;
			f32 *d_3 = zbuffer_row_3;

			advance_x = offset_ps;

			for(i32 x = min_x; x < max_x; x += BLOCK_RASTER_SIZE)
			{
				// NOTE(flo): check if we find an edge of the triangle in the 4x4 block area (half-space function)
				__m128i m_0 = _mm_or_si128(_mm_or_si128(alpha_0, beta_0), gamma_0);
				__m128i m_1 = _mm_or_si128(_mm_or_si128(alpha_1, beta_1), gamma_1);
				__m128i m_2 = _mm_or_si128(_mm_or_si128(alpha_2, beta_2), gamma_2);
				__m128i m_3 = _mm_or_si128(_mm_or_si128(alpha_3, beta_3), gamma_3);

				__m128i mask_0 = _mm_cmplt_epi32(zero_SSEi, m_0);
				__m128i mask_1 = _mm_cmplt_epi32(zero_SSEi, m_1);
				__m128i mask_2 = _mm_cmplt_epi32(zero_SSEi, m_2);
				__m128i mask_3 = _mm_cmplt_epi32(zero_SSEi, m_3);

				__m128i mask_zero = _mm_or_si128(_mm_or_si128(_mm_or_si128(mask_0, mask_1), mask_2), mask_3);
				// __m128i mask_one = _mm_and_si128(_mm_and_si128(_mm_and_si128(mask_0, mask_1), mask_2), mask_3);

				// NOTE(flo): no edges here
				if(!_mm_test_all_zeros(mask_zero, mask_zero)) {

					__m128 u_row_0 = _mm_add_ps(u_rowy, _mm_mul_ps(u_advx, advance_x));
					__m128 u_row_1 = _mm_add_ps(u_row_0, u_advy);
					__m128 u_row_2 = _mm_add_ps(u_row_1, u_advy);
					__m128 u_row_3 = _mm_add_ps(u_row_2, u_advy);

					__m128 v_row_0 = _mm_add_ps(v_rowy, _mm_mul_ps(v_advx, advance_x));
					__m128 v_row_1 = _mm_add_ps(v_row_0, v_advy);
					__m128 v_row_2 = _mm_add_ps(v_row_1, v_advy);
					__m128 v_row_3 = _mm_add_ps(v_row_2, v_advy);

					__m128 z_row_0 = _mm_add_ps(z_rowy, _mm_mul_ps(z_advx, advance_x));
					__m128 z_row_1 = _mm_add_ps(z_row_0, z_advy);
					__m128 z_row_2 = _mm_add_ps(z_row_1, z_advy);
					__m128 z_row_3 = _mm_add_ps(z_row_2, z_advy);

					__m128 d_row_0 = _mm_add_ps(d_rowy, _mm_mul_ps(d_advx, advance_x));
					__m128 d_row_1 = _mm_add_ps(d_row_0, d_advy);
					__m128 d_row_2 = _mm_add_ps(d_row_1, d_advy);
					__m128 d_row_3 = _mm_add_ps(d_row_2, d_advy);

					__m128 l_row_0 = _mm_add_ps(l_rowy, _mm_mul_ps(l_advx, advance_x));
					__m128 l_row_1 = _mm_add_ps(l_row_0, l_advy);
					__m128 l_row_2 = _mm_add_ps(l_row_1, l_advy);
					__m128 l_row_3 = _mm_add_ps(l_row_2, l_advy);

					__m128 original_d_0 = _mm_load_ps(d_0);
					__m128 original_d_1 = _mm_load_ps(d_1);
					__m128 original_d_2 = _mm_load_ps(d_2);
					__m128 original_d_3 = _mm_load_ps(d_3);

					__m128 depth_mask_0_ps = _mm_cmplt_ps(d_row_0, original_d_0);
					__m128 depth_mask_1_ps = _mm_cmplt_ps(d_row_1, original_d_1);
					__m128 depth_mask_2_ps = _mm_cmplt_ps(d_row_2, original_d_2);
					__m128 depth_mask_3_ps = _mm_cmplt_ps(d_row_3, original_d_3);		 

					__m128i depth_mask_0 = _mm_castps_si128(depth_mask_0_ps);
					__m128i depth_mask_1 = _mm_castps_si128(depth_mask_1_ps);
					__m128i depth_mask_2 = _mm_castps_si128(depth_mask_2_ps);
					__m128i depth_mask_3 = _mm_castps_si128(depth_mask_3_ps);
					// NOTE(flo): we treat partial/full fill at the same time
					__m128i w_mask_0 = _mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(alpha_0, mask_ffffffff), 
					                                               _mm_cmpgt_epi32(beta_0, mask_ffffffff)), _mm_cmpgt_epi32(gamma_0, mask_ffffffff));
					__m128i w_mask_1 = _mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(alpha_1, mask_ffffffff), 
					                                               _mm_cmpgt_epi32(beta_1, mask_ffffffff)), _mm_cmpgt_epi32(gamma_1, mask_ffffffff));
					__m128i w_mask_2 = _mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(alpha_2, mask_ffffffff), 
					                                               _mm_cmpgt_epi32(beta_2, mask_ffffffff)), _mm_cmpgt_epi32(gamma_2, mask_ffffffff));
					__m128i w_mask_3 = _mm_and_si128(_mm_and_si128(_mm_cmpgt_epi32(alpha_3, mask_ffffffff), 
					                                               _mm_cmpgt_epi32(beta_3, mask_ffffffff)), _mm_cmpgt_epi32(gamma_3, mask_ffffffff));

					w_mask_0 = _mm_and_si128(depth_mask_0, w_mask_0);
					w_mask_1 = _mm_and_si128(depth_mask_1, w_mask_1);
					w_mask_2 = _mm_and_si128(depth_mask_2, w_mask_2);
					w_mask_3 = _mm_and_si128(depth_mask_3, w_mask_3);

					u32 dst_off = x * FRAME_BUFFER_BYTES_PER_PIXEL + y * pitch;
					if(!_mm_test_all_zeros(w_mask_0, w_mask_0))
					{
						__m128 w_mask_ps = _mm_castsi128_ps(w_mask_0);
						__m128 new_depth = _mm_or_ps(_mm_and_ps(w_mask_ps, d_row_0), _mm_andnot_ps(w_mask_ps, original_d_0)); 

						_mm_store_ps(d_0, new_depth);

						__m128 persp = _mm_div_ps(one_SSE, z_row_0);
						__m128 clampu = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(u_row_0, persp)));
						__m128 clampv = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(v_row_0, persp)));
						__m128i src_x = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampu, w_m1), half_SSE));
						__m128i src_y = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampv, h_m1), half_SSE));
						__m128i src_mem_off = _mm_add_epi32(_mm_mullo_epi32(src_x, t_bpp), _mm_mullo_epi32(src_y, t_pitch));

						// TODO(flo): linear blend between texels here! (we certainly need to load more from our u and v coordinates)
						u8 *t_mem_0 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[3];
						u8 *t_mem_1 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[2];
						u8 *t_mem_2 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[1];
						u8 *t_mem_3 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[0];

						__m128i sample;
						if(diffuse.bytes_per_pixel == 1)
						{
							u32 color_0 = (t_mem_0[0] << 24) | (t_mem_0[0] << 16) | (t_mem_0[0] << 8) | (t_mem_0[0] << 0);
							u32 color_1 = (t_mem_1[0] << 24) | (t_mem_1[0] << 16) | (t_mem_1[0] << 8) | (t_mem_1[0] << 0);
							u32 color_2 = (t_mem_2[0] << 24) | (t_mem_2[0] << 16) | (t_mem_2[0] << 8) | (t_mem_2[0] << 0);
							u32 color_3 = (t_mem_3[0] << 24) | (t_mem_3[0] << 16) | (t_mem_3[0] << 8) | (t_mem_3[0] << 0);
							sample = _mm_set_epi32(color_0, color_1, color_2, color_3);
						}
						else if(diffuse.bytes_per_pixel == 3)
						{
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
							sample = _mm_or_si128(sample, mask_ff000000);
						}
						else
						{
							kh_assert(diffuse.bytes_per_pixel == 4);
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
						}

						// TODO(flo): premultiplied alpha and gamma correction
						__m128i texel_rb = _mm_and_si128(sample, _mm_set1_epi32(0xFF00FF));
						__m128i texel_ag = _mm_and_si128(_mm_srli_epi32(sample, 8), _mm_set1_epi32(0xFF00FF));

						__m128 texel_a = _mm_cvtepi32_ps(_mm_srli_epi32(texel_ag, 16));
						__m128 texel_r = _mm_cvtepi32_ps(_mm_srli_epi32(texel_rb, 16));
						__m128 texel_g = _mm_cvtepi32_ps(_mm_and_si128(texel_ag, mask_ffff));
						__m128 texel_b = _mm_cvtepi32_ps(_mm_and_si128(texel_rb, mask_ffff));

						u8 *dst_row = (u8 *)target->pixels.memory + dst_off;
						__m128i orig_dst_row = _mm_load_si128((__m128i *)dst_row);

						__m128 dst_row_b = _mm_cvtepi32_ps(_mm_and_si128(orig_dst_row, mask_ff)); 
						__m128 dst_row_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 8), mask_ff));
						__m128 dst_row_r = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 16), mask_ff));
						__m128 dst_row_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 24), mask_ff));

						texel_r = _mm_mul_ps(texel_r, color_r);
						texel_g = _mm_mul_ps(texel_g, color_g);
						texel_b = _mm_mul_ps(texel_b, color_b);
						texel_a = _mm_mul_ps(texel_a, color_a);

						__m128 inv_a = _mm_sub_ps(one_SSE, _mm_mul_ps(inv_255_SSE, texel_a));
						__m128 final_r = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_r), texel_r);
						__m128 final_g = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_g), texel_g);
						__m128 final_b = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_b), texel_b);
						__m128 final_a = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_a), texel_a);

						// NOTE(flo): lighting
						final_r = _mm_mul_ps(final_r, l_row_0);
						final_g = _mm_mul_ps(final_g, l_row_0);
						final_b = _mm_mul_ps(final_b, l_row_0);

						__m128i s_r = _mm_slli_epi32(_mm_cvtps_epi32(final_r), 16);
						__m128i s_g = _mm_slli_epi32(_mm_cvtps_epi32(final_g), 8);
						__m128i s_b = _mm_cvtps_epi32(final_b);
						__m128i s_a = _mm_slli_epi32(_mm_cvtps_epi32(final_a), 24);
						__m128i store = _mm_or_si128(_mm_or_si128(s_r, s_g), _mm_or_si128(s_b, s_a));
						store = _mm_or_si128(_mm_and_si128(w_mask_0, store), _mm_andnot_si128(w_mask_0, orig_dst_row));
						_mm_store_si128((__m128i *)dst_row, store);
					}

					dst_off += pitch;

					if(!_mm_test_all_zeros(w_mask_1, w_mask_1))
					{
						__m128 w_mask_ps = _mm_castsi128_ps(w_mask_1);
						__m128 new_depth = _mm_or_ps(_mm_and_ps(w_mask_ps, d_row_1), _mm_andnot_ps(w_mask_ps, original_d_1));
						_mm_store_ps(d_1, new_depth);
						__m128 persp = _mm_div_ps(one_SSE, z_row_1);
						__m128 clampu = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(u_row_1, persp)));
						__m128 clampv = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(v_row_1, persp)));
						__m128i src_x = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampu, w_m1), half_SSE));
						__m128i src_y = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampv, h_m1), half_SSE));
						__m128i src_mem_off = _mm_add_epi32(_mm_mullo_epi32(src_x, t_bpp), _mm_mullo_epi32(src_y, t_pitch));
						u8 *t_mem_0 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[3];
						u8 *t_mem_1 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[2];
						u8 *t_mem_2 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[1];
						u8 *t_mem_3 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[0];

						__m128i sample;
						if(diffuse.bytes_per_pixel == 1)
						{
							u32 color_0 = (t_mem_0[0] << 24) | (t_mem_0[0] << 16) | (t_mem_0[0] << 8) | (t_mem_0[0] << 0);
							u32 color_1 = (t_mem_1[0] << 24) | (t_mem_1[0] << 16) | (t_mem_1[0] << 8) | (t_mem_1[0] << 0);
							u32 color_2 = (t_mem_2[0] << 24) | (t_mem_2[0] << 16) | (t_mem_2[0] << 8) | (t_mem_2[0] << 0);
							u32 color_3 = (t_mem_3[0] << 24) | (t_mem_3[0] << 16) | (t_mem_3[0] << 8) | (t_mem_3[0] << 0);
							sample = _mm_set_epi32(color_0, color_1, color_2, color_3);
						}
						else if(diffuse.bytes_per_pixel == 3)
						{
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
							sample = _mm_or_si128(sample, mask_ff000000);
						}
						else
						{
							kh_assert(diffuse.bytes_per_pixel == 4);
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
						}

						__m128i texel_rb = _mm_and_si128(sample, _mm_set1_epi32(0xFF00FF));
						__m128i texel_ag = _mm_and_si128(_mm_srli_epi32(sample, 8), _mm_set1_epi32(0xFF00FF));
						// texel_0_rb = _mm_mullo_epi16(texel_0_rb, texel_0_rb);
						__m128 texel_a = _mm_cvtepi32_ps(_mm_srli_epi32(texel_ag, 16));
						__m128 texel_r = _mm_cvtepi32_ps(_mm_srli_epi32(texel_rb, 16));
						__m128 texel_g = _mm_cvtepi32_ps(_mm_and_si128(texel_ag, mask_ffff));
						__m128 texel_b = _mm_cvtepi32_ps(_mm_and_si128(texel_rb, mask_ffff));
						// texel_0_ag = _mm_mullo_epi16(texel_0_ag, texel_0_ag);

						u8 *dst_row = (u8 *)target->pixels.memory + dst_off;
						__m128i orig_dst_row = _mm_load_si128((__m128i *)dst_row);

						__m128 dst_row_b = _mm_cvtepi32_ps(_mm_and_si128(orig_dst_row, mask_ff)); 
						__m128 dst_row_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 8), mask_ff));
						__m128 dst_row_r = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 16), mask_ff));
						__m128 dst_row_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 24), mask_ff));

						texel_r = _mm_mul_ps(texel_r, color_r);
						texel_g = _mm_mul_ps(texel_g, color_g);
						texel_b = _mm_mul_ps(texel_b, color_b);
						texel_a = _mm_mul_ps(texel_a, color_a);

						__m128 inv_a = _mm_sub_ps(one_SSE, _mm_mul_ps(inv_255_SSE, texel_a));
						__m128 final_r = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_r), texel_r);
						__m128 final_g = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_g), texel_g);
						__m128 final_b = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_b), texel_b);
						__m128 final_a = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_a), texel_a);

						final_r = _mm_mul_ps(final_r, l_row_1);
						final_g = _mm_mul_ps(final_g, l_row_1);
						final_b = _mm_mul_ps(final_b, l_row_1);

						__m128i s_r = _mm_slli_epi32(_mm_cvtps_epi32(final_r), 16);
						__m128i s_g = _mm_slli_epi32(_mm_cvtps_epi32(final_g), 8);
						__m128i s_b = _mm_cvtps_epi32(final_b);
						__m128i s_a = _mm_slli_epi32(_mm_cvtps_epi32(final_a), 24);
						__m128i store = _mm_or_si128(_mm_or_si128(s_r, s_g), _mm_or_si128(s_b, s_a));
						store = _mm_or_si128(_mm_and_si128(w_mask_1, store), _mm_andnot_si128(w_mask_1, orig_dst_row));

						_mm_store_si128((__m128i *)dst_row, store);
					}

					dst_off += pitch;

					if(!_mm_test_all_zeros(w_mask_2, w_mask_2))
					{
						__m128 w_mask_ps = _mm_castsi128_ps(w_mask_2);
						__m128 new_depth = _mm_or_ps(_mm_and_ps(w_mask_ps, d_row_2), _mm_andnot_ps(w_mask_ps, original_d_2));
						_mm_store_ps(d_2, new_depth);
						__m128 persp = _mm_div_ps(one_SSE, z_row_2);
						__m128 clampu = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(u_row_2, persp)));
						__m128 clampv = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(v_row_2, persp)));
						__m128i src_x = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampu, w_m1), half_SSE));
						__m128i src_y = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampv, h_m1), half_SSE));
						__m128i src_mem_off = _mm_add_epi32(_mm_mullo_epi32(src_x, t_bpp), _mm_mullo_epi32(src_y, t_pitch));
						u8 *t_mem_0 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[3];
						u8 *t_mem_1 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[2];
						u8 *t_mem_2 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[1];
						u8 *t_mem_3 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[0];

						__m128i sample;
						if(diffuse.bytes_per_pixel == 1)
						{
							u32 color_0 = (t_mem_0[0] << 24) | (t_mem_0[0] << 16) | (t_mem_0[0] << 8) | (t_mem_0[0] << 0);
							u32 color_1 = (t_mem_1[0] << 24) | (t_mem_1[0] << 16) | (t_mem_1[0] << 8) | (t_mem_1[0] << 0);
							u32 color_2 = (t_mem_2[0] << 24) | (t_mem_2[0] << 16) | (t_mem_2[0] << 8) | (t_mem_2[0] << 0);
							u32 color_3 = (t_mem_3[0] << 24) | (t_mem_3[0] << 16) | (t_mem_3[0] << 8) | (t_mem_3[0] << 0);
							sample = _mm_set_epi32(color_0, color_1, color_2, color_3);
						}
						else if(diffuse.bytes_per_pixel == 3)
						{
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
							sample = _mm_or_si128(sample, mask_ff000000);
						}
						else
						{
							kh_assert(diffuse.bytes_per_pixel == 4);
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
						}

						__m128i texel_rb = _mm_and_si128(sample, _mm_set1_epi32(0xFF00FF));
						__m128i texel_ag = _mm_and_si128(_mm_srli_epi32(sample, 8), _mm_set1_epi32(0xFF00FF));
						// texel_0_rb = _mm_mullo_epi16(texel_0_rb, texel_0_rb);
						__m128 texel_a = _mm_cvtepi32_ps(_mm_srli_epi32(texel_ag, 16));
						__m128 texel_r = _mm_cvtepi32_ps(_mm_srli_epi32(texel_rb, 16));
						__m128 texel_g = _mm_cvtepi32_ps(_mm_and_si128(texel_ag, mask_ffff));
						__m128 texel_b = _mm_cvtepi32_ps(_mm_and_si128(texel_rb, mask_ffff));
						// texel_0_ag = _mm_mullo_epi16(texel_0_ag, texel_0_ag);

						u8 *dst_row = (u8 *)target->pixels.memory + dst_off;
						__m128i orig_dst_row = _mm_load_si128((__m128i *)dst_row);

						__m128 dst_row_b = _mm_cvtepi32_ps(_mm_and_si128(orig_dst_row, mask_ff)); 
						__m128 dst_row_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 8), mask_ff));
						__m128 dst_row_r = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 16), mask_ff));
						__m128 dst_row_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 24), mask_ff));

						texel_r = _mm_mul_ps(texel_r, color_r);
						texel_g = _mm_mul_ps(texel_g, color_g);
						texel_b = _mm_mul_ps(texel_b, color_b);
						texel_a = _mm_mul_ps(texel_a, color_a);

						__m128 inv_a = _mm_sub_ps(one_SSE, _mm_mul_ps(inv_255_SSE, texel_a));
						__m128 final_r = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_r), texel_r);
						__m128 final_g = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_g), texel_g);
						__m128 final_b = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_b), texel_b);
						__m128 final_a = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_a), texel_a);

						final_r = _mm_mul_ps(final_r, l_row_2);
						final_g = _mm_mul_ps(final_g, l_row_2);
						final_b = _mm_mul_ps(final_b, l_row_2);

						__m128i s_r = _mm_slli_epi32(_mm_cvtps_epi32(final_r), 16);
						__m128i s_g = _mm_slli_epi32(_mm_cvtps_epi32(final_g), 8);
						__m128i s_b = _mm_cvtps_epi32(final_b);
						__m128i s_a = _mm_slli_epi32(_mm_cvtps_epi32(final_a), 24);
						__m128i store = _mm_or_si128(_mm_or_si128(s_r, s_g), _mm_or_si128(s_b, s_a));
						store = _mm_or_si128(_mm_and_si128(w_mask_2, store), _mm_andnot_si128(w_mask_2, orig_dst_row));
						_mm_store_si128((__m128i *)dst_row, store);
					}

					dst_off += pitch;

					if(!_mm_test_all_zeros(w_mask_3, w_mask_3))
					{
						__m128 w_mask_ps = _mm_castsi128_ps(w_mask_3);
						__m128 new_depth = _mm_or_ps(_mm_and_ps(w_mask_ps, d_row_3), _mm_andnot_ps(w_mask_ps, original_d_3));
						_mm_store_ps(d_3, new_depth);
						__m128 persp = _mm_div_ps(one_SSE, z_row_3);
						__m128 clampu = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(u_row_3, persp)));
						__m128 clampv = _mm_max_ps(zero_SSE, _mm_min_ps(one_SSE, _mm_mul_ps(v_row_3, persp)));
						__m128i src_x = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampu, w_m1), half_SSE));
						__m128i src_y = _mm_cvttps_epi32(_mm_add_ps(_mm_mul_ps(clampv, h_m1), half_SSE));
						__m128i src_mem_off = _mm_add_epi32(_mm_mullo_epi32(src_x, t_bpp), _mm_mullo_epi32(src_y, t_pitch));
						u8 *t_mem_0 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[3];
						u8 *t_mem_1 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[2];
						u8 *t_mem_2 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[1];
						u8 *t_mem_3 = (u8 *)diffuse_memory + src_mem_off.m128i_i32[0];

						__m128i sample;
						if(diffuse.bytes_per_pixel == 1)
						{
							u32 color_0 = (t_mem_0[0] << 24) | (t_mem_0[0] << 16) | (t_mem_0[0] << 8) | (t_mem_0[0] << 0);
							u32 color_1 = (t_mem_1[0] << 24) | (t_mem_1[0] << 16) | (t_mem_1[0] << 8) | (t_mem_1[0] << 0);
							u32 color_2 = (t_mem_2[0] << 24) | (t_mem_2[0] << 16) | (t_mem_2[0] << 8) | (t_mem_2[0] << 0);
							u32 color_3 = (t_mem_3[0] << 24) | (t_mem_3[0] << 16) | (t_mem_3[0] << 8) | (t_mem_3[0] << 0);
							sample = _mm_set_epi32(color_0, color_1, color_2, color_3);
						}
						else if(diffuse.bytes_per_pixel == 3)
						{
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
							sample = _mm_or_si128(sample, mask_ff000000);
						}
						else
						{
							kh_assert(diffuse.bytes_per_pixel == 4);
							sample = _mm_set_epi32(*(u32 *)t_mem_0, *(u32 *)t_mem_1, *(u32 *)t_mem_2, *(u32 *)t_mem_3);
						}

						__m128i texel_rb = _mm_and_si128(sample, _mm_set1_epi32(0xFF00FF));
						__m128i texel_ag = _mm_and_si128(_mm_srli_epi32(sample, 8), _mm_set1_epi32(0xFF00FF));
						// texel_0_rb = _mm_mullo_epi16(texel_0_rb, texel_0_rb);
						__m128 texel_a = _mm_cvtepi32_ps(_mm_srli_epi32(texel_ag, 16));
						__m128 texel_r = _mm_cvtepi32_ps(_mm_srli_epi32(texel_rb, 16));
						__m128 texel_g = _mm_cvtepi32_ps(_mm_and_si128(texel_ag, mask_ffff));
						__m128 texel_b = _mm_cvtepi32_ps(_mm_and_si128(texel_rb, mask_ffff));
						// texel_0_ag = _mm_mullo_epi16(texel_0_ag, texel_0_ag);

						u8 *dst_row = (u8 *)target->pixels.memory + dst_off;
						__m128i orig_dst_row = _mm_load_si128((__m128i *)dst_row);

						__m128 dst_row_b = _mm_cvtepi32_ps(_mm_and_si128(orig_dst_row, mask_ff)); 
						__m128 dst_row_g = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 8), mask_ff));
						__m128 dst_row_r = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 16), mask_ff));
						__m128 dst_row_a = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(orig_dst_row, 24), mask_ff));

						texel_r = _mm_mul_ps(texel_r, color_r);
						texel_g = _mm_mul_ps(texel_g, color_g);
						texel_b = _mm_mul_ps(texel_b, color_b);
						texel_a = _mm_mul_ps(texel_a, color_a);

						__m128 inv_a = _mm_sub_ps(one_SSE, _mm_mul_ps(inv_255_SSE, texel_a));
						__m128 final_r = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_r), texel_r);
						__m128 final_g = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_g), texel_g);
						__m128 final_b = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_b), texel_b);
						__m128 final_a = _mm_add_ps(_mm_mul_ps(inv_a, dst_row_a), texel_a);

						final_r = _mm_mul_ps(final_r, l_row_3);
						final_g = _mm_mul_ps(final_g, l_row_3);
						final_b = _mm_mul_ps(final_b, l_row_3);

						__m128i s_r = _mm_slli_epi32(_mm_cvtps_epi32(final_r), 16);
						__m128i s_g = _mm_slli_epi32(_mm_cvtps_epi32(final_g), 8);
						__m128i s_b = _mm_cvtps_epi32(final_b);
						__m128i s_a = _mm_slli_epi32(_mm_cvtps_epi32(final_a), 24);
						__m128i store = _mm_or_si128(_mm_or_si128(s_r, s_g), _mm_or_si128(s_b, s_a));
						store = _mm_or_si128(_mm_and_si128(w_mask_3, store), _mm_andnot_si128(w_mask_3, orig_dst_row));
						_mm_store_si128((__m128i *)dst_row, store);
					}
				}
			    alpha_0 = _mm_add_epi32(alpha_0, a0_inc);
			    alpha_1 = _mm_add_epi32(alpha_1, a0_inc);
			    alpha_2 = _mm_add_epi32(alpha_2, a0_inc);
			    alpha_3 = _mm_add_epi32(alpha_3, a0_inc);
			    beta_0 = _mm_add_epi32(beta_0, a1_inc);
			    beta_1 = _mm_add_epi32(beta_1, a1_inc);
			    beta_2 = _mm_add_epi32(beta_2, a1_inc);
			    beta_3 = _mm_add_epi32(beta_3, a1_inc);
			    gamma_0 = _mm_add_epi32(gamma_0, a2_inc);
			    gamma_1 = _mm_add_epi32(gamma_1, a2_inc);
			    gamma_2 = _mm_add_epi32(gamma_2, a2_inc);
			    gamma_3 = _mm_add_epi32(gamma_3, a2_inc);
			    advance_x = _mm_add_ps(advance_x, inc_ps);

			    d_0 += BLOCK_RASTER_SIZE;
			    d_1 += BLOCK_RASTER_SIZE;
			    d_2 += BLOCK_RASTER_SIZE;
			    d_3 += BLOCK_RASTER_SIZE;

			}
			b0_row = _mm_add_epi32(b0_row, b0_inc);
			b1_row = _mm_add_epi32(b1_row, b1_inc);
			b2_row = _mm_add_epi32(b2_row, b2_inc);
			advance_y = _mm_add_ps(advance_y, inc_ps);

			zbuffer_row_0 += zbuffer_pitch;
			zbuffer_row_1 += zbuffer_pitch;
			zbuffer_row_2 += zbuffer_pitch;
			zbuffer_row_3 += zbuffer_pitch;
		}
	}
}

// TODO(flo): clipping
inline void
rasterize_triangle_scanline(SoftwarePixelsBuffer *target, SoftwareDepthBuffer *zbuffer, Vertex_PNU vert_0, Vertex_PNU vert_1, Vertex_PNU vert_2, v4 color, const Texture2D *diffuse, const u8 *diffuse_memory)
{
	// NOTE(flo) : triangle area = half of magnitude of the 2D cross product
	// we just need to check if the area is positive or negative so we juste compute
	// the parallelogram area
	f32 triangle_area_times_two = ((vert_1.pos.x - vert_0.pos.x) * (vert_2.pos.y - vert_0.pos.y)) -
		((vert_2.pos.x - vert_0.pos.x) * (vert_1.pos.y - vert_0.pos.y));
	// NOTE(flo): we are counterclockwise
	if(triangle_area_times_two < 0)
	{
		return;
	}
		// TODO(flo): we need to swap base on the y values of each vertices
	if(vert_2.pos.y < vert_1.pos.y)
	{
		Vertex_PNU tmp = vert_2;
		vert_2 = vert_1;
		vert_1 = tmp;
	}
	if(vert_1.pos.y < vert_0.pos.y)
	{
		Vertex_PNU tmp = vert_1;
		vert_1 = vert_0;
		vert_0 = tmp;
	}
	if(vert_2.pos.y < vert_1.pos.y)
	{
		Vertex_PNU tmp = vert_2;
		vert_2 = vert_1;
		vert_1 = tmp;
	}

	triangle_area_times_two = ((vert_1.pos.x - vert_0.pos.x) * (vert_2.pos.y - vert_0.pos.y)) -
	((vert_2.pos.x - vert_0.pos.x) * (vert_1.pos.y - vert_0.pos.y));
	b32 counter_clockwise = (triangle_area_times_two >= 0);

		// TODO(flo): textures mapping
	f32 depth[3] = { vert_0.pos.z, vert_1.pos.z, vert_2.pos.z };
	v3 light_dir = kh_vec3(0,0,1);
	f32 light[3] = {
		kh_clamp01_f32(kh_dot_v3(vert_0.normal, light_dir)) * 0.9f + 0.1f,
		kh_clamp01_f32(kh_dot_v3(vert_1.normal, light_dir)) * 0.9f + 0.1f,
		kh_clamp01_f32(kh_dot_v3(vert_2.normal, light_dir)) * 0.9f + 0.1f
	};
	f32 one_over_z[3] = { 1.0f / vert_0.pos.z, 1.0f / vert_1.pos.z, 1.0f / vert_2.pos.z };
	f32 texcoord_x[3] = { vert_0.uv0.x * one_over_z[0], vert_1.uv0.x * one_over_z[1],
		vert_2.uv0.x * one_over_z[2] };
	f32 texcoord_y[3] = { vert_0.uv0.y * one_over_z[0], vert_1.uv0.y * one_over_z[1],
		vert_2.uv0.y * one_over_z[2] };

	/* NOTE(flo) : Interpolant
		Linear interpolation between points a line :
 		(y - y0) / (x - x0) = (y1 - y0) / (x1 - x0)
		or (x - x0) / (y - y0) = (x1 - x0) / (y1 - y0)

		in our case :
		x1 = vert_1.pos.x
		y1 = vert_1.pos.y
		x0 = vert_0.pos.x
		y0 = vert_0.pos.y
		x2 = vert_2.pos.x
		y2 = vert_2.pos.y


		we want x when y = vert_0.pos.y so y = y0
		so : (x - x0) / (y2 - y0) = (x1 - x0) / (y1 - y0)
		multiply both side by (y0 - y2) we have :
				x - x0 = (x1 - x0) / (y1 - y0) * (y2 - y0);
				x = (x1 - x0) / (y1 - y0) * (y2 - y0) + x0;
				x = vert_0.pos.x + (vert_2.pos.y - vert_0.pos.y) *
					(vert_1.pos.x - vert_0.pos.x) / (vert_1.pos.y - vert_0.pos.y);
		we can derive to have our linear blend such as a + t*(b - a) if we switch division/mul :
			x = x0 + ((y2 - y0) / (y1 - y0)) * (x1 - x0);
			t = (y2 - y0) / (y1 - y0);
			x = linear_blend(x0, x1, t);

		we have C as our interpolant such as
			(C - c0) / (y - y0) = (c1 - c0) / (y1 - y0)
		we want to interpolate over dx = C - c2 / x - x2
		same equation for C since we want C when y = vert_2.pos.y or y = y2
			C = c0 + (y2 - y0) * (c1 - c0) / (y1 - y0);
			C = c0 + ((y2 - y0) / (y1 - y0)) * (c1 - c0)
			C = linear_blend(c2, c0, t);

		dc = linear_blend(c0, c1, t) - c2;
		dx = linear_blend(x0, x1, t) - x2;

		we can derive to find :
		dc/dx = (((c1 - c0) * (y2 - y0)) + ((c0 - c2) * (y1 - y0))) /
			(((x1 - x0) * (y2 - y0)) + ((x0 - x2) * (y1 - y0)))
		or
		dc/dx = (((c1 - c0) * (y2 - y0)) - ((c2 - c0) * (y1 - y0))) /
			(((x1 - x0) * (y2 - y0)) - ((x2 - x0) * (y1 - y0)))

		if we derive for dy we have :
		dc/dy = (((c1 -c0) * (x2 - x0)) + ((c0 - c2) * (x1 - x0))) /
			(((x2 - x0) * (y1 - y0)) + ((x0 - x1) * (y2 - y0)))
		or
		dc/dy = (((c1 -c0) * (x2 - x0)) - ((c2 - c0) * (x1 - x0))) /
			(((x2 - x0) * (y1 - y0)) - ((x1 - x0) * (y2 - y0)))
		so dy = -dx;
	*/


	f32 x1_minus_x0 = vert_1.pos.x - vert_0.pos.x;
	f32 y1_minus_y0 = vert_1.pos.y - vert_0.pos.y;
	f32 x2_minus_x0 = vert_2.pos.x - vert_0.pos.x;
	f32 y2_minus_y0 = vert_2.pos.y - vert_0.pos.y;

	f32 dx = (x1_minus_x0 * y2_minus_y0) - (x2_minus_x0 * y1_minus_y0);
	f32 one_over_dx = 1.0f / dx;
	f32 one_over_dy = -one_over_dx;

	v2 dc = {};
	f32 c1_minus_c0 = texcoord_x[1] - texcoord_x[0];
	f32 c2_minus_c0 = texcoord_x[2] - texcoord_x[0];
	dc.x = (c1_minus_c0 * y2_minus_y0) - (c2_minus_c0 * y1_minus_y0);
	dc.y = (c1_minus_c0 * x2_minus_x0) - (c2_minus_c0 * x1_minus_x0);
	v2 texcoord_xstep = kh_vec2(dc.x * one_over_dx, dc.y * one_over_dy);

	c1_minus_c0 = texcoord_y[1] - texcoord_y[0];
	c2_minus_c0 = texcoord_y[2] - texcoord_y[0];
	dc.x = (c1_minus_c0 * y2_minus_y0) - (c2_minus_c0 * y1_minus_y0);
	dc.y = (c1_minus_c0 * x2_minus_x0) - (c2_minus_c0 * x1_minus_x0);
	v2 texcoord_ystep = kh_vec2(dc.x * one_over_dx, dc.y * one_over_dy);

	c1_minus_c0 = one_over_z[1] - one_over_z[0];
	c2_minus_c0 = one_over_z[2] - one_over_z[0];
	dc.x = (c1_minus_c0 * y2_minus_y0) - (c2_minus_c0 * y1_minus_y0);
	dc.y = (c1_minus_c0 * x2_minus_x0) - (c2_minus_c0 * x1_minus_x0);
	v2 one_over_zstep = kh_vec2(dc.x * one_over_dx, dc.y * one_over_dy);

	c1_minus_c0 = depth[1] - depth[0];
	c2_minus_c0 = depth[2] - depth[0];
	dc.x = (c1_minus_c0 * y2_minus_y0) - (c2_minus_c0 * y1_minus_y0);
	dc.y = (c1_minus_c0 * x2_minus_x0) - (c2_minus_c0 * x1_minus_x0);
	v2 depth_step = kh_vec2(dc.x * one_over_dx, dc.y * one_over_dy);

	c1_minus_c0 = light[1] - light[0];
	c2_minus_c0 = light[2] - light[0];
	dc.x = (c1_minus_c0 * y2_minus_y0) - (c2_minus_c0 * y1_minus_y0);
	dc.y = (c1_minus_c0 * x2_minus_x0) - (c2_minus_c0 * x1_minus_x0);
	v2 light_step = kh_vec2(dc.x * one_over_dx, dc.y * one_over_dy);

	u32 src_pitch = diffuse->width * diffuse->bytes_per_pixel;

	TriangleEdge edge_20;
	edge_20.start_y = kh_ceil_f32_to_i32(vert_0.pos.y);
	edge_20.end_y = kh_ceil_f32_to_i32(vert_2.pos.y);
	f32 edge_20_prestep_y = edge_20.start_y - vert_0.pos.y;
	edge_20.step_x = (vert_2.pos.x - vert_0.pos.x) / (vert_2.pos.y - vert_0.pos.y);
	edge_20.x = vert_0.pos.x + edge_20_prestep_y * edge_20.step_x;
	f32 edge_20_prestep_x = edge_20.x - vert_0.pos.x;
	edge_20.texcoord_x = texcoord_x[0] +
	texcoord_xstep.x * edge_20_prestep_x + texcoord_xstep.y * edge_20_prestep_y;
	edge_20.texcoord_xstep = texcoord_xstep.y + texcoord_xstep.x * edge_20.step_x;
	edge_20.texcoord_y = texcoord_y[0] +
	texcoord_ystep.x * edge_20_prestep_x + texcoord_ystep.y * edge_20_prestep_y;
	edge_20.texcoord_ystep = texcoord_ystep.y + texcoord_ystep.x * edge_20.step_x;
	edge_20.one_over_z = one_over_z[0] +
	one_over_zstep.x * edge_20_prestep_x + one_over_zstep.y * edge_20_prestep_y;
	edge_20.one_over_zstep = one_over_zstep.y + one_over_zstep.x * edge_20.step_x;
	edge_20.depth = depth[0] +
	depth_step.x * edge_20_prestep_x + depth_step.y * edge_20_prestep_y;
	edge_20.depth_step = depth_step.y + depth_step.x * edge_20.step_x;
	edge_20.light = light[0] +
	light_step.x * edge_20_prestep_x + light_step.y * edge_20_prestep_y;
	edge_20.light_step = light_step.y + light_step.x * edge_20.step_x;

	TriangleEdge edge_01;
	edge_01.start_y = kh_ceil_f32_to_i32(vert_0.pos.y);
	edge_01.end_y = kh_ceil_f32_to_i32(vert_1.pos.y);
	f32 edge_01_prestep_y = edge_01.start_y - vert_0.pos.y;
	edge_01.step_x = (vert_1.pos.x - vert_0.pos.x) / (vert_1.pos.y - vert_0.pos.y);
	edge_01.x = vert_0.pos.x + edge_01_prestep_y * edge_01.step_x;
	f32 edge_01_prestep_x = edge_01.x - vert_0.pos.x;
	edge_01.texcoord_x = texcoord_x[0] +
	texcoord_xstep.x * edge_01_prestep_x + texcoord_xstep.y * edge_01_prestep_y;
	edge_01.texcoord_xstep = texcoord_xstep.y + texcoord_xstep.x * edge_01.step_x;
	edge_01.texcoord_y = texcoord_y[0] +
	texcoord_ystep.x * edge_01_prestep_x + texcoord_ystep.y * edge_01_prestep_y;
	edge_01.texcoord_ystep = texcoord_ystep.y + texcoord_ystep.x * edge_01.step_x;
	edge_01.one_over_z = one_over_z[0] +
	one_over_zstep.x * edge_01_prestep_x + one_over_zstep.y * edge_01_prestep_y;
	edge_01.one_over_zstep = one_over_zstep.y + one_over_zstep.x * edge_01.step_x;
	edge_01.depth = depth[0] +
	depth_step.x * edge_01_prestep_x + depth_step.y * edge_01_prestep_y;
	edge_01.depth_step = depth_step.y + depth_step.x * edge_01.step_x;
	edge_01.light = light[0] +
	light_step.x * edge_01_prestep_x + light_step.y * edge_01_prestep_y;
	edge_01.light_step = light_step.y + light_step.x * edge_01.step_x;

	TriangleEdge edge_12;
	edge_12.start_y = kh_ceil_f32_to_i32(vert_1.pos.y);
	edge_12.end_y = kh_ceil_f32_to_i32(vert_2.pos.y);
	f32 edge_12_prestep_y = edge_12.start_y - vert_1.pos.y;
	edge_12.step_x = (vert_2.pos.x - vert_1.pos.x) / (vert_2.pos.y - vert_1.pos.y);
	edge_12.x = vert_1.pos.x + edge_12_prestep_y * edge_12.step_x;
	f32 edge_12_prestep_x = edge_12.x - vert_1.pos.x;
	edge_12.texcoord_x = texcoord_x[1] +
	texcoord_xstep.x * edge_12_prestep_x + texcoord_xstep.y * edge_12_prestep_y;
	edge_12.texcoord_xstep = texcoord_xstep.y + texcoord_xstep.x * edge_12.step_x;
	edge_12.texcoord_y = texcoord_y[1] +\
	texcoord_ystep.x * edge_12_prestep_x + texcoord_ystep.y * edge_12_prestep_y;
	edge_12.texcoord_ystep = texcoord_ystep.y + texcoord_ystep.x * edge_12.step_x;
	edge_12.one_over_z = one_over_z[1] +
	one_over_zstep.x * edge_12_prestep_x + one_over_zstep.y * edge_12_prestep_y;
	edge_12.one_over_zstep = one_over_zstep.y + one_over_zstep.x * edge_12.step_x;
	edge_12.depth = depth[1] +
	depth_step.x * edge_12_prestep_x + depth_step.y * edge_12_prestep_y;
	edge_12.depth_step = depth_step.y + depth_step.x * edge_12.step_x;
	edge_12.light = light[1] +
	light_step.x * edge_12_prestep_x + light_step.y * edge_12_prestep_y;
	edge_12.light_step = light_step.y + light_step.x * edge_12.step_x;

	TriangleEdge *left = &edge_20;
	TriangleEdge *right = &edge_01;

	if(!counter_clockwise)
	{
		TriangleEdge *tmp = left;
		left = right;
		right = tmp;
	}

	i32 min_y = edge_01.start_y;
	i32 max_y = edge_01.end_y;

	for(i32 y = min_y; y < max_y; ++y)
	{
		i32 min_x = kh_ceil_f32_to_i32(left->x);
		i32 max_x = kh_ceil_f32_to_i32(right->x);
		f32 prestep_x = min_x - left->x;
		f32 u = left->texcoord_x + texcoord_xstep.x * prestep_x;
		f32 v = left->texcoord_y + texcoord_ystep.x * prestep_x;
		f32 z_row = left->one_over_z + one_over_zstep.x * prestep_x;
		f32 depth_row = left->depth + depth_step.x * prestep_x;
		f32 light_row = left->light + light_step.x * prestep_x;
		for(i32 x = min_x; x < max_x; ++x)
		{
			i32 ind = x + y * target->w;
			if(depth_row < zbuffer->memory[ind])
			{
				zbuffer->memory[ind] = depth_row;
				f32 z = 1.0f / z_row;
				u8* dst = (u8 *)target->memory + x * FRAME_BUFFER_BYTES_PER_PIXEL + y * target->pitch;
				u32 *dst_pixel = (u32 *)dst;
				i32 src_x = (i32)((u * z) * (f32)(diffuse->width - 1) + 0.5f);
				i32 src_y = (i32)((v * z) * (f32)(diffuse->height - 1) + 0.5f);
				u8* src = (u8 *)diffuse_memory + src_x * diffuse->bytes_per_pixel +
				src_y * src_pitch;
				u32 src_pixel = (0xFF << 24 | src[2] << 16 | src[1] << 8 | src[0] << 0);
				*dst_pixel = src_pixel;
			}
			z_row += one_over_zstep.x;
			u += texcoord_xstep.x;
			v += texcoord_ystep.x;
			depth_row += depth_step.x;
			light_row += light_step.x;
		}
		left->x += left->step_x;
		left->texcoord_x += left->texcoord_xstep;
		left->texcoord_y += left->texcoord_ystep;
		left->one_over_z += left->one_over_zstep;
		left->depth += left->depth_step;
		left->light += left->light_step;
		right->x += right->step_x;
		right->texcoord_x += right->texcoord_xstep;
		right->texcoord_y += right->texcoord_ystep;
		right->one_over_z += right->one_over_zstep;
		right->depth += right->depth_step;
		right->light += right->light_step;
	}

	left = &edge_20;
	right = &edge_12;

	if(!counter_clockwise)
	{
		TriangleEdge *tmp = left;
		left = right;
		right = tmp;
	}

	min_y = edge_12.start_y;
	max_y = edge_12.end_y;

	for(i32 y = min_y; y < max_y; ++y)
	{
		i32 min_x = kh_ceil_f32_to_i32(left->x);
		i32 max_x = kh_ceil_f32_to_i32(right->x);
		f32 prestep_x = min_x - left->x;
		f32 u = left->texcoord_x + texcoord_xstep.x * prestep_x;
		f32 v = left->texcoord_y + texcoord_ystep.x * prestep_x;
		f32 z_row = left->one_over_z + one_over_zstep.x * prestep_x;
		f32 depth_row = left->depth + depth_step.x * prestep_x;
		f32 light_row = left->light + light_step.x * prestep_x;
		for(i32 x = min_x; x < max_x; ++x)
		{
			i32 ind = x + y * target->w;
			if(depth_row < zbuffer->memory[ind])
			{
				zbuffer->memory[ind] = depth_row;
				f32 z = 1.0f / z_row;
				u8* dst = (u8 *)target->memory + x * FRAME_BUFFER_BYTES_PER_PIXEL + y * target->pitch;
				u32 *dst_pixel = (u32 *)dst;
				i32 src_x = (i32)((u * z) * (f32)(diffuse->width - 1) + 0.5f);
				i32 src_y = (i32)((v * z) * (f32)(diffuse->height - 1) + 0.5f);
				u8* src = (u8 *)diffuse_memory + src_x * diffuse->bytes_per_pixel +
				src_y * src_pitch;
				u32 src_pixel = (0xFF << 24 | src[2] << 16 | src[1] << 8 | src[0] << 0);
				// u32 *src_pixel = (u32 *)src;
				*dst_pixel = src_pixel;
			}
			z_row += one_over_zstep.x;
			u += texcoord_xstep.x;
			v += texcoord_ystep.x;
			depth_row += depth_step.x;
			light_row += light_step.x;
		}
		left->x += left->step_x;
		left->texcoord_x += left->texcoord_xstep;
		left->texcoord_y += left->texcoord_ystep;
		left->one_over_z += left->one_over_zstep;
		left->depth += left->depth_step;
		left->light += left->light_step;
		right->x += right->step_x;
		right->texcoord_x += right->texcoord_xstep;
		right->texcoord_y += right->texcoord_ystep;
		right->one_over_z += right->one_over_zstep;
		right->depth += right->depth_step;
		right->light += right->light_step;
	}
}

KH_INTERN void
clear_pixels_buffer(SoftwarePixelsBuffer *target, u32 color, u32 start, u32 end)
{
	u32 *pixels = (u32 *)target->memory + start;
	for(u32 i = start; i < end; ++i) {
		*pixels++ = color;
	}
}

KH_INLINE void
clear_depth_buffer(SoftwareDepthBuffer *zbuffer, u32 start, u32 end) {
	f32 *dst = zbuffer->memory + start;
	for(u32 i = start; i < end; i++) {
		*dst++ = F32_MAX;
	}
}
