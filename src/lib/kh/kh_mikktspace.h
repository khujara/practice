#include "ext/mikktspace.h"
#include "ext/mikktspace.c"

struct OutVertices {
	v3 *tangents;
	f32 *sign;
};

struct MikkTSpaceUserData {
	AssetContents contents;
	OutVertices out;
};

struct VertexHashEl {
	u32 index;
	Vertex_PNUT vertex;
	VertexHashEl *next_in_hash;
};

struct VertexHash {
	u32 max_el_count;
	VertexHashEl **el;

	u32 max_count;
	u32 buffer_count;
	VertexHashEl *buffer;

	u32 collision_count;
};

struct HashResult {
	b32 is_new;
	u32 index;
};

struct IndexList {
	u32 index;
	IndexList *next;
};

struct IndexMap {
	IndexList **ind_list;
	u32 buffer_max_count;
	u32 buffer_count;
	IndexList *buffer;
};

int mikk_get_num_faces(const SMikkTSpaceContext *ctx) {
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 res = user_data->contents.type.trimesh.tri_c;
	return(res);
}

int mikk_get_num_vertices_of_face(const SMikkTSpaceContext *ctx, const int iFace) {
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 res = 3;
	return(res);
}

void mikk_get_pos(const SMikkTSpaceContext *ctx, float *out_pos, const int iFace, const int iVert) {
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 *indices = (u32 *)((u8 *)user_data->contents.memory + user_data->contents.type.trimesh.indices_offset);
	u32 *index = indices + iFace * 3 + iVert;
	Vertex_PNU *vertex = (Vertex_PNU *)user_data->contents.memory + *index;
	out_pos[0] = vertex->pos.x;
	out_pos[1] = vertex->pos.y;
	out_pos[2] = vertex->pos.z;
}

void mikk_get_normal(const SMikkTSpaceContext *ctx, float *out_normal, const int iFace, const int iVert) {
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 *indices = (u32 *)((u8 *)user_data->contents.memory + user_data->contents.type.trimesh.indices_offset);
	u32 *index = indices + iFace * 3 + iVert;
	Vertex_PNU *vertex = (Vertex_PNU *)user_data->contents.memory + *index;
	out_normal[0] = vertex->normal.x;
	out_normal[1] = vertex->normal.y;
	out_normal[2] = vertex->normal.z;
}

void mikk_get_uv0(const SMikkTSpaceContext *ctx, float *out_uv0, const int iFace, const int iVert) {
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 *indices = (u32 *)((u8 *)user_data->contents.memory + user_data->contents.type.trimesh.indices_offset);
	u32 *index = indices + iFace * 3 + iVert;
	Vertex_PNU *vertex = (Vertex_PNU *)user_data->contents.memory + *index;
	out_uv0[0] = vertex->uv0.x;
	out_uv0[1] = vertex->uv0.y;
}

void mikk_set_tspace_basic(const SMikkTSpaceContext *ctx, const float *in_tangents, const float sign,
                            const int iFace, const int iVert) {
	// NOTE(flo): use this to fill our own buffer of tangents
	// IMPORTANT(flo): as specified we need to regenerate our index list and not use
	// one that has been provided by the source contents!
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	u32 *indices = (u32 *)((u8 *)user_data->contents.memory + user_data->contents.type.trimesh.indices_offset);
	u32 *index = indices + iFace * 3 + iVert;
	Vertex_PNU *vertex = (Vertex_PNU *)user_data->contents.memory + *index;

	v3 tangent = *(v3 *)in_tangents;
	// v3 bitangent = sign * kh_cross_v3(vertex->normal, tangent);
	user_data->out.tangents[iFace * 3 + iVert] = tangent;
	user_data->out.sign[iFace * 3 + iVert] = sign;
	// user_data->out.bitangents[iFace * 3 + iVert] = bitangent;
}

void mikk_set_tspace(const SMikkTSpaceContext *ctx, const float *in_tangents, const float *in_bitangents,
                      const float tan_len, const float bitan_len, const tbool orientation,
                      const int iFace, const int iVert) {
	// NOTE(flo): use this to fill our own buffer of tangents/bitangents
	MikkTSpaceUserData *user_data = (MikkTSpaceUserData *)ctx->m_pUserData;
	NOT_IMPLEMENTED;
}

inline Vertex_PNUTB
get_pnutb(const Vertex_PNUT *pnut, f32 sign) {
	Vertex_PNUTB res;
	res.pos = pnut->pos;
	res.normal = pnut->normal;
	res.uv0 = pnut->uv0;
	res.tangent = pnut->tangent;
	// res.bitangent = sign * kh_cross_v3(res.normal, res.tangent);
	res.bitangent = sign * kh_cross_v3(res.tangent, res.normal);
	return(res);
}

inline Vertex_PNUT
get_pnut(const Vertex_PNU *pnu, const v3 *tangent) {
	Vertex_PNUT res;
	res.pos = pnu->pos;
	res.normal = pnu->normal;
	res.uv0 = pnu->uv0;
	res.tangent = *tangent;
	return(res);
}

// TODO(flo): better hash function!!!
static u32
get_key_from_vertex_pnut(const Vertex_PNUT *vert) {
	u32 pr[] = {
		73856093,
		19349663,
		83492791,
		53390501,
		35821229,
		48083213,
		64997599,
		9737239,
		94481161,
		106423979,
		68586887,
		71735143,

	};	
	u32 p = (*(u32 *)&vert->pos.x * pr[0]) ^ (*(u32 *)&vert->pos.y * pr[1]) ^ (*(u32 *)&vert->pos.z * pr[2]); 
	u32 n = (*(u32 *)&vert->normal.x * pr[4]) ^ (*(u32 *)&vert->normal.y * pr[5]) ^ (*(u32 *)&vert->normal.z * pr[6]); 
	u32 u = (*(u32 *)&vert->uv0.x * pr[0]) ^ (*(u32 *)&vert->uv0.y * pr[1]);
	u32 t = (*(u32 *)&vert->tangent.x * pr[2]) ^ (*(u32 *)&vert->tangent.y * pr[3]) ^ (*(u32 *)&vert->tangent.z * pr[4]); 
	u32 res = p ^ n ^ u ^ t;
	return(res);
}

static b32
vertex_pnut_are_equals(const Vertex_PNUT *a, const Vertex_PNUT *b) {
	b32 res = 	(a->pos == b->pos) && 
				(a->normal == b->normal) && 
				(a->uv0 == b->uv0) &&
				(a->tangent == b->tangent);
	return(res);
}

static HashResult
get_or_create_vertex_hash_el(VertexHash *hash, const Vertex_PNUT *vert) {
	HashResult res = {};
	u32 hash_key = get_key_from_vertex_pnut(vert);
	u32 hash_slot = hash_key % hash->max_el_count;
	kh_assert(hash_slot < hash->max_el_count);

	VertexHashEl **first = hash->el + hash_slot;
	VertexHashEl *find = 0;
	for(VertexHashEl *search = *first; search; search = search->next_in_hash) {
		if(vertex_pnut_are_equals(&search->vertex, vert)) {
			find = search;
			break;
		}
	}
	if(!find) {
		hash->collision_count += (*first) ? 1 : 0;
		kh_assert(hash->buffer_count < hash->max_count);
		u32 index = hash->buffer_count++;
		find = hash->buffer + index;
		find->index = index;
		find->vertex = *vert;
		find->next_in_hash = *first;
		*first = find;
		res.is_new = true;
	}

	res.index = find->index;

	return(res);
}

inline void 
reset_hash(VertexHash *hash) {
	kh_lu0(el_i, hash->max_el_count) {
		VertexHashEl **el = hash->el + el_i;
		*el = 0;
	}
	hash->collision_count = 0;
	hash->buffer_count = 0;
}

static AssetContents 
add_mikktspace(AssetContents *in_contents, LinearArena *temp_arena, LinearArena *contents_arena, VertexFormat out_format,
               IndexMap *index_map) {
	AssetContents res = {};
	kh_assert(in_contents->type.key == AssetType_trimesh);
	kh_assert(in_contents->type.trimesh.format == VertexFormat_PNU);

	TriangleMesh trimesh = in_contents->type.trimesh;
	u32 vertex_size = get_size_from_vertex_format(out_format);

	MikkTSpaceUserData user_data = {};
	user_data.contents = *in_contents;
	user_data.out.tangents = (v3 *)kh_push(temp_arena, sizeof(v3) * trimesh.tri_c * 3);
	user_data.out.sign = (f32 *)kh_push(temp_arena, sizeof(f32) * trimesh.tri_c * 3);
	SMikkTSpaceContext mikk_ctx;
	mikk_ctx.m_pUserData = &user_data;
	SMikkTSpaceInterface *mikk_interface = (SMikkTSpaceInterface *)kh_push(temp_arena, sizeof(SMikkTSpaceInterface));
	mikk_interface->m_getNumFaces = mikk_get_num_faces;
	mikk_interface->m_getNumVerticesOfFace = mikk_get_num_vertices_of_face;
	mikk_interface->m_getPosition = mikk_get_pos;
	mikk_interface->m_getNormal = mikk_get_normal;
	mikk_interface->m_getTexCoord = mikk_get_uv0;
	mikk_interface->m_setTSpaceBasic = mikk_set_tspace_basic;
	// mikk_interface->m_setTSpace = mikk_set_tspace;
	mikk_ctx.m_pInterface = mikk_interface;
	bool mikk_res = genTangSpaceDefault(&mikk_ctx);

	kh_assert(mikk_res);

	VertexHash hash = {};
	// NOTE(flo): this is the maximum vertices that we could have
	hash.max_count = trimesh.tri_c * 3; 
	// TODO(flo): is this accurate ?
	hash.max_el_count = hash.max_count;
	hash.buffer = (VertexHashEl *)kh_push(temp_arena, sizeof(VertexHashEl) * hash.max_count);
	hash.el =  (VertexHashEl **)kh_push(temp_arena, sizeof(VertexHashEl *) * hash.max_el_count);

	// TODO(flo): can we do this in one go ?
	Vertex_PNU *src_verts = (Vertex_PNU *)user_data.contents.memory;
	u32 *src_inds = (u32 *)((u8 *)user_data.contents.memory + trimesh.indices_offset);
	kh_lu0(ind_i, trimesh.tri_c * 3) {
		u32 index = src_inds[ind_i];
		v3 src_tangent = user_data.out.tangents[ind_i];
		Vertex_PNU vert_pnu = src_verts[index];
		Vertex_PNUT vert = get_pnut(&vert_pnu, &src_tangent);
		get_or_create_vertex_hash_el(&hash, &vert);
	}

	u32 vertex_count = hash.buffer_count; 
	u32 indices_count = trimesh.tri_c * 3;
	u32 vertices_size = vertex_count * vertex_size;
	u32 indices_size = indices_count * sizeof(u32);

	res.type.trimesh.vert_c = vertex_count;
	res.type.trimesh.tri_c = trimesh.tri_c;
	res.type.trimesh.vertex_size = vertex_size;
	res.type.trimesh.format = out_format;
	res.type.trimesh.indices_offset = vertices_size; 
	res.type.key = AssetType_trimesh;
	res.size = vertices_size + indices_size;
	res.memory = kh_push(contents_arena, res.size);

	reset_hash(&hash);
	u8 *out_verts = (u8 *)res.memory;
	u32 *out_inds = (u32 *)((u8 *)res.memory + vertices_size);
	kh_lu0(ind_i, trimesh.tri_c * 3) {
		u32 src_index = src_inds[ind_i];

		v3 src_tangent = user_data.out.tangents[ind_i];
		f32 sign = user_data.out.sign[ind_i];

		Vertex_PNU vert_pnu = src_verts[src_index];

		Vertex_PNUT vert_pnut = get_pnut(&vert_pnu, &src_tangent);
		HashResult hash_res = get_or_create_vertex_hash_el(&hash, &vert_pnut);

		IndexList **first_ind = index_map->ind_list + src_index;
		if(*first_ind) {
			b32 find = false;
			for(IndexList *search = *first_ind; search; search = search->next) {
				if(search->index == hash_res.index) {
					find = true;
					break;
				}
				if(!find) {
					IndexList *new_index = index_map->buffer + index_map->buffer_count++; 
					new_index->index = hash_res.index;
					new_index->next = *first_ind;
					*first_ind = new_index;
				}
			}
		} else {
			IndexList *new_index = index_map->buffer + index_map->buffer_count++; 
			new_index->index = hash_res.index;
			new_index->next = 0;
			*first_ind = new_index;
		}


		out_inds[ind_i] = hash_res.index;
		if(hash_res.is_new) {
			switch(out_format) {
				case VertexFormat_PNUTBS :
				case VertexFormat_PNUTB : {
					Vertex_PNUTB vert_pnutb = get_pnutb(&vert_pnut, sign);
					Vertex_PNUTB *dst = (Vertex_PNUTB *)(out_verts + hash_res.index * vertex_size);
					*dst = vert_pnutb;
				} break	;
				case VertexFormat_PNUTS :
				case VertexFormat_PNUT : {
					Vertex_PNUT *dst = (Vertex_PNUT *)(out_verts + hash_res.index * vertex_size);
					*dst = vert_pnut;
				} break;
				INVALID_DEFAULT_CASE;
			}
		}
	}
	return(res);
}