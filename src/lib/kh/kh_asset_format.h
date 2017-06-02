#define MAX_JOINTS_PER_VERTEX 4

// @TODO(flo): support multiple uvs coordinates per mesh
enum VertexFormat {
	VertexFormat_PosNormalUV,
	VertexFormat_PosNormalTangentUV,
	VertexFormat_PosNormalTangentBitangentUV,

	VertexFormat_PosNormalUVSkinned,
	VertexFormat_PosNormalTangentUVSkinned,
	VertexFormat_PosNormalTangentBitangentUVSkinned,

	VertexFormat_count,
};

struct Vertex_PNU {
	v3 pos;
	v3 normal;
	v2 uv0;
};

struct Vertex_PNUT {
	v3 pos;
	v3 normal;
	v2 uv0;
	v3 tangent;
};

struct Vertex_PNUTB {
	v3 pos;
	v3 normal;
	v2 uv0;
	v3 tangent;
	v3 bitangent;
};

struct Vertex_PNUS {
	v3 pos;
	v3 normal;
	v2 uv0;
	u32 ids[MAX_JOINTS_PER_VERTEX];
	f32 weights[MAX_JOINTS_PER_VERTEX];
};

struct Vertex_PNUTS {
	v3 pos;
	v3 normal;
	v2 uv0;
	v3 tangent;
	u32 ids[MAX_JOINTS_PER_VERTEX];
	f32 weights[MAX_JOINTS_PER_VERTEX];
};

struct Vertex_PNUTBS {
	v3 pos;
	v3 normal;
	v2 uv0;
	v3 tangent;
	v3 bitangent;
	u32 ids[MAX_JOINTS_PER_VERTEX];
	f32 weights[MAX_JOINTS_PER_VERTEX];
};

struct VertexAttribute {
	u32 vertex_size;
	i32 pos_offset;
	i32 uv0_offset;
	i32 nor_offset;
};

struct VertexSkinnedAttribute {
	i32 offset;
};


struct Texture2D {
	u32 width;
	u32 height;
	u32 bytes_per_pixel; //TODO(flo): get rid off
};

struct TriangleMesh {
	u32 vert_c;
	u32 tri_c;

	u32 vertex_size;
	VertexFormat format;

	u32 indices_offset;
};

struct Joint {
	i32 parent_id;
};

struct Skeleton {
	u32 joint_count;
};

struct AnimationClip {
	u32 transform;

	u32 sample_count;
	
	f64 duration;
	f64 frame_rate;
};

// TODO(flo): do something with this to renable our font import
// struct FontInfos
// {
// 	u32 tex_w, tex_h;
// 	u32 glyph_count;
// 	FontGlyph *glyphs;
// 	u32 *codepoints_map;
// 	f32 *kernel_advance;
// 	f32 advance_y;
// };


struct FontGlyph {
	u32 code_point;
	f32 advance_width;
	u32 x0, x1;
	u32 y0, y1;
	f32 xoff, yoff;
	//TODO(flo): for what purposes ? 
	f32 xoff1, yoff1;
};

struct Font {
	u32 tex_w, tex_h;
	u32 glyph_count;
	u32 highest_codepoint;
	f32 advance_y;
};

static b32
has_skin(VertexFormat format) {
	kh_assert(format < VertexFormat_count);
	b32 res = (format >= VertexFormat_PosNormalUVSkinned);
	return(res);
}

static u32
get_size_from_vertex_format(VertexFormat format) {
	u32 res = 0;
	switch(format) {
		case VertexFormat_PosNormalUV : { res = sizeof(Vertex_PNU); } break;
		case VertexFormat_PosNormalTangentUV : { res = sizeof(Vertex_PNUT); } break;
		case VertexFormat_PosNormalTangentBitangentUV : { res = sizeof(Vertex_PNUTB); } break;
		case VertexFormat_PosNormalUVSkinned : { res = sizeof(Vertex_PNUS); } break;
		case VertexFormat_PosNormalTangentUVSkinned : { res = sizeof(Vertex_PNUTS); } break;
		case VertexFormat_PosNormalTangentBitangentUVSkinned : { res = sizeof(Vertex_PNUTBS); } break;
		default : { kh_assert(!"unsupported vertex format"); } break;
	}
	return(res);
};

KH_INLINE VertexSkinnedAttribute
get_skinned_attribute_offset(VertexFormat format) {
	VertexSkinnedAttribute res;
	kh_assert(has_skin(format));
	res.offset = -1;

	switch(format) {
		case VertexFormat_PosNormalUVSkinned : {
			res.offset = KH_OFFSETOF(Vertex_PNUS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUS, ids) == 4);
		};
		case VertexFormat_PosNormalTangentUVSkinned : {
			res.offset = KH_OFFSETOF(Vertex_PNUTS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUTS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUTS, ids) == 4);

		};
		case VertexFormat_PosNormalTangentBitangentUVSkinned : {
			res.offset = KH_OFFSETOF(Vertex_PNUTBS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUTBS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUTBS, ids) == 4);
		};
	};
	return(res);
}

KH_INLINE VertexAttribute
get_vertex_attribute(VertexFormat format) {
	VertexAttribute res;
	res.vertex_size = 0;
	res.pos_offset = -1;
	res.uv0_offset = -1;
	res.nor_offset = -1;
	switch(format) {
		case VertexFormat_PosNormalUV : {
			res.vertex_size = sizeof(Vertex_PNU);
			res.pos_offset = KH_OFFSETOF(Vertex_PNU, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNU, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNU, normal);
		} break;
		case VertexFormat_PosNormalTangentUV : {
			res.vertex_size = sizeof(Vertex_PNUT);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUT, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUT, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUT, normal);
		} break;
		case VertexFormat_PosNormalTangentBitangentUV : {
			res.vertex_size = sizeof(Vertex_PNUTB);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTB, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTB, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTB, normal);
		} break;
		case VertexFormat_PosNormalUVSkinned : {
			res.vertex_size = sizeof(Vertex_PNUS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUS, normal);
		} break;
		case VertexFormat_PosNormalTangentUVSkinned : {
			res.vertex_size = sizeof(Vertex_PNUTS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTS, normal);
		} break;
		case VertexFormat_PosNormalTangentBitangentUVSkinned : {
			res.vertex_size = sizeof(Vertex_PNUTBS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTBS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTBS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTBS, normal);
		} break;
		default : { kh_assert(!"unsupported vertex format"); } break;
	}
	kh_assert(res.pos_offset != -1);
	kh_assert(res.uv0_offset != -1);
	kh_assert(res.nor_offset != -1);
	kh_assert(res.vertex_size > 0);	
	return(res);
}