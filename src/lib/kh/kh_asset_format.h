#define MAX_JOINTS_PER_VERTEX 4
#define FOREACH_VERTEX_FORMAT(VERTEXFORMAT) \
	VERTEXFORMAT(VertexFormat_PNU) \
	VERTEXFORMAT(VertexFormat_PNUT) \
	VERTEXFORMAT(VertexFormat_PNUTB) \
	VERTEXFORMAT(VertexFormat_PNUS) \
	VERTEXFORMAT(VertexFormat_PNUTS) \
	VERTEXFORMAT(VertexFormat_PNUTBS) \
	VERTEXFORMAT(VertexFormat_count_or_none) \

// @TODO(flo): support multiple uvs coordinates per mesh
enum VertexFormat {
	FOREACH_VERTEX_FORMAT(GENERATE_ENUM)
};

char *G_VERTEX_FORMATS[] = {
	FOREACH_VERTEX_FORMAT(GENERATE_STR)
};

struct Vertex_PNU {
	v3 pos;
	v3 normal;
	v2 uv0;
};

struct Vertex_PNUC {
	v3 pos;
	v3 normal;
	v2 uv0;
	v3 color;
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
	i32 tan_offset;
	i32 bit_offset;
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
	char debug_name[32];
	i32 parent_id;
};

struct Skeleton {
	u32 joint_count;
};

struct AnimationSkin {
	u32 animation_id;
};

struct AnimationClip {
	u32 transform;

	u32 joint_count;
	u32 sample_count;
	
	f64 duration;
	f64 frame_rate;
};

struct FontGlyph {
	u32 code_point;
	f32 advance_width;
	u32 x0, x1;
	u32 y0, y1;
	f32 xoff, yoff;
	//TODO(flo): for what purposes ? 
	f32 xoff1, yoff1;
};

struct FontRange {
	u32 tex_w, tex_h;
	u32 first_glyph;
	u32 last_glyph;
	f32 advance_y;
};

static b32
has_skin(VertexFormat format) {
	kh_assert(format < VertexFormat_count_or_none);
	b32 res = (format >= VertexFormat_PNUS);
	return(res);
}

static u32
get_size_from_vertex_format(VertexFormat format) {
	u32 res = 0;
	switch(format) {
		case VertexFormat_PNU : { res = sizeof(Vertex_PNU); } break;
		case VertexFormat_PNUT : { res = sizeof(Vertex_PNUT); } break;
		case VertexFormat_PNUTB : { res = sizeof(Vertex_PNUTB); } break;
		case VertexFormat_PNUS : { res = sizeof(Vertex_PNUS); } break;
		case VertexFormat_PNUTS : { res = sizeof(Vertex_PNUTS); } break;
		case VertexFormat_PNUTBS : { res = sizeof(Vertex_PNUTBS); } break;
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
		case VertexFormat_PNUS : {
			res.offset = KH_OFFSETOF(Vertex_PNUS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUS, ids) == 4);
		} break;
		case VertexFormat_PNUTS : {
			res.offset = KH_OFFSETOF(Vertex_PNUTS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUTS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUTS, ids) == 4);

		} break;
		case VertexFormat_PNUTBS : {
			res.offset = KH_OFFSETOF(Vertex_PNUTBS, ids);
			kh_assert(KH_SIZEOF(Vertex_PNUTBS, ids[0]) == 4);
			kh_assert(KH_ARRAYCOUNT(Vertex_PNUTBS, ids) == 4);
		} break;
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
	res.tan_offset = -1;
	res.bit_offset = -1;
	switch(format) {
		case VertexFormat_PNU : {
			res.vertex_size = sizeof(Vertex_PNU);
			res.pos_offset = KH_OFFSETOF(Vertex_PNU, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNU, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNU, normal);
		} break;
		case VertexFormat_PNUT : {
			res.vertex_size = sizeof(Vertex_PNUT);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUT, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUT, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUT, normal);
			res.tan_offset = KH_OFFSETOF(Vertex_PNUT, tangent);
		} break;
		case VertexFormat_PNUTB : {
			res.vertex_size = sizeof(Vertex_PNUTB);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTB, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTB, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTB, normal);
			res.tan_offset = KH_OFFSETOF(Vertex_PNUTB, tangent);
			res.bit_offset = KH_OFFSETOF(Vertex_PNUTB, bitangent);
		} break;
		case VertexFormat_PNUS : {
			res.vertex_size = sizeof(Vertex_PNUS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUS, normal);
		} break;
		case VertexFormat_PNUTS : {
			res.vertex_size = sizeof(Vertex_PNUTS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTS, normal);
			res.tan_offset = KH_OFFSETOF(Vertex_PNUTB, tangent);
		} break;
		case VertexFormat_PNUTBS : {
			res.vertex_size = sizeof(Vertex_PNUTBS);
			res.pos_offset = KH_OFFSETOF(Vertex_PNUTBS, pos);
			res.uv0_offset = KH_OFFSETOF(Vertex_PNUTBS, uv0);
			res.nor_offset = KH_OFFSETOF(Vertex_PNUTBS, normal);
			res.tan_offset = KH_OFFSETOF(Vertex_PNUTB, tangent);
			res.bit_offset = KH_OFFSETOF(Vertex_PNUTB, bitangent);
		} break;
		default : { kh_assert(!"unsupported vertex format"); } break;
	}
	kh_assert(res.vertex_size == get_size_from_vertex_format(format));
	kh_assert(res.pos_offset != -1);
	kh_assert(res.uv0_offset != -1);
	kh_assert(res.nor_offset != -1);
	kh_assert(res.vertex_size > 0);	
	return(res);
}