// @TODO(flo): support multiple uvs coordinates per mesh
enum VertexFormat {
	VertexFormat_PosNormalUV,
	VertexFormat_PosNormalTangentUV,
	VertexFormat_PosNormalTangentBitangentUV,
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

struct VertexAttribute {
	u32 vertex_size;
	i32 pos_offset;
	i32 uv0_offset;
	i32 nor_offset;
};

static u32
get_size_from_vertex_format(VertexFormat format) {
	u32 res = 0;
	switch(format) {
		case VertexFormat_PosNormalUV : { res = sizeof(Vertex_PNU); } break;
		case VertexFormat_PosNormalTangentUV : { res = sizeof(Vertex_PNUT); } break;
		case VertexFormat_PosNormalTangentBitangentUV : { res = sizeof(Vertex_PNUTB); } break;
		default : { kh_assert(!"unsupported vertex format"); } break;
	}
	return(res);
};


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
	}
	kh_assert(res.pos_offset != -1);
	kh_assert(res.uv0_offset != -1);
	kh_assert(res.nor_offset != -1);
	kh_assert(res.vertex_size > 0);	
	return(res);
}

struct Texture2D {
	u32 width;
	u32 height;
	u32 bytes_per_pixel; //TODO(flo): get rid off
	void *memory;
};

struct TriangleMesh {
	u32 vert_c;
	u32 tri_c;

	u8 *memory;
	u32 indices_offset;

	u32 vertex_size;
	VertexFormat format;

};
