#define MAX_MESH_RENDERER 64
#define MAX_RENDER_ENTRIES 64
#define MAX_MATERIAL_INSTANCE 64
#define FRAME_BUFFER_BYTES_PER_PIXEL 4
#define INVALID_DATA_HANDLE 0xFFFFFFFF
#define MAX_DATA_HANDLES_IN_HASH 512
struct DataHashElement {
	u32 hash_index;
	u32 id;

	// u32 buffer;
	// u32 header_offset;

	u32 data_offset;
	u32 gpu_index;

	DataHashElement *next_in_hash;
};

// TODO(flo): we need to have different buffers that can fit to our vertex formats to
// so that we could have a map pointer directly to gpu. We'll need to split our datas and our headers
struct DataCache {
	DataHashElement hash[MAX_DATA_HANDLES_IN_HASH];

	// GeneralPurposeAllocator headers;
	GeneralPurposeAllocator buffer;
	// u32 buffer_count;
	//GeneralPurposeAllocator *buffers;

};

struct DirectionalLight {
	v3 color;
	f32 ambient_intensity;
	v3 dir;
	f32 diffuse_intensity;
};

struct Skybox {
	u32 right;
	u32 left;
	u32 bottom;
	u32 top;
	u32 back;
	u32 front;
};

struct RenderEntry {
	u32 next_in_mesh_renderer;
	// TODO(flo): Transform_SQT
	mat4 tr;
};

struct MeshRenderer {
	u32 mesh;
	u32 entry_count;
	u32 first_entry;
	MeshRenderer *next;
};

struct MaterialInstance {
	u32 diffuse;
	u32 normal;
	v4 color;
	MaterialInstance *next;
	MeshRenderer *first;
};

struct Material {
	MaterialType type;
	Material *next;
	MaterialInstance *first;
};

struct VertexBuffer {
	VertexFormat format;
	VertexBuffer *next;
	Material *first;
};

struct LoadResult {
	b32 loaded;
	u32 handle;
};

#define INVALID_RENDER_ENTRY 0xFFFFFFFF

// TODO(flo): support multiple lights and different light types (point, spot)
#define MAX_LIGHTS 1
struct RenderManager {

	// -----------------------------------------------
	// NOTE(flo): we need to clear this each frame
	u32 commands_at;
	u32 render_entry_count;
	u32 batch_count;
	// --------------------------------------------------------

	Camera camera;
	u32 width, height;

	VertexBuffer *first_vertex_buffer;
	VertexBuffer *cur_vertex_buffer;
	Material *cur_material;
	MaterialInstance *cur_mat_instance;

	u32 commands_size;
	u8 *commands;
	RenderEntry render_entries[MAX_RENDER_ENTRIES]; 

	b32 has_skybox;
	Skybox skybox;
	u32 light_count;
	DirectionalLight lights[MAX_LIGHTS];

	DataCache *cache;
};

KH_INTERN u32 
get_handle_from_data_cache(DataCache *cache, u32 id) {
	u32 res = 0;

	u32 hash_key = id & (MAX_DATA_HANDLES_IN_HASH - 1);
	kh_assert(hash_key < MAX_DATA_HANDLES_IN_HASH);

	DataHashElement *search = cache->hash + hash_key;
	if(search->hash_index == INVALID_DATA_HANDLE) {
		res = search->hash_index;
	} else if(search->id == id) {
		res = search->hash_index;
	} else {
		search = search->next_in_hash;
		while(search && search->id != id) {
			search = search->next_in_hash;
		}
		res = (search) ? search->hash_index : INVALID_DATA_HANDLE; 
	}
	return(res);
}

KH_INTERN void
add_handle_to_data_cache(DataCache *cache, u32 id) {
	const u32 MAX_TRIAL_COUNT = MAX_DATA_HANDLES_IN_HASH - 1;

	u32 hash_key = id & MAX_TRIAL_COUNT;
	kh_assert(hash_key < MAX_DATA_HANDLES_IN_HASH);

	DataHashElement *el = cache->hash + hash_key;
	if(el->hash_index == INVALID_DATA_HANDLE) {
		el->hash_index = hash_key;
		el->data_offset = (u32)cache->buffer.used;
		el->id = id;
	} else {
		u32 trial_count = 0;
		hash_key = (hash_key + 1) & MAX_TRIAL_COUNT;
		DataHashElement *search = cache->hash + hash_key;
		while(search->hash_index != INVALID_DATA_HANDLE && trial_count < MAX_TRIAL_COUNT) {
			hash_key = (hash_key + 1) & MAX_TRIAL_COUNT;
			search = cache->hash + hash_key;
			trial_count++;
		}
		kh_assert(trial_count < MAX_TRIAL_COUNT);
		search->hash_index = hash_key;
		search->data_offset = (u32)cache->buffer.used;
		search->id = id;
		el->next_in_hash = search;
	}
}

#define get_datas(cache,el, type) (type *)get_datas_(cache, el)
KH_INLINE u8 *
get_datas_(DataCache *cache, DataHashElement *el) {
	kh_assert(el);
	u8 *res = cache->buffer.base + el->data_offset;
	return(res);
}

#define get_datas_from_handle(cache, hdl, type) (type *)get_datas_from_handle_(cache, hdl)
KH_INLINE u8 *
get_datas_from_handle_(DataCache *cache, u32 hdl) {
	kh_assert(hdl != INVALID_DATA_HANDLE);
	DataHashElement *el = cache->hash + hdl;
	u8 *res = cache->buffer.base + el->data_offset;
	return(res);
}

KH_INTERN LoadResult
texture_loaded(Assets *assets, DataCache *cache, TextureID id) {
	LoadResult res;
	res.handle = get_handle_from_data_cache(cache, id.val);
	res.loaded = false;
	if(id.val) {
		Asset *asset = assets->arr + id.val;
		res.loaded = ((res.handle != INVALID_DATA_HANDLE) && (asset->state == AssetState_loaded));
		if(!res.loaded) {
			load_texture_2d(assets, cache, id);
		}
	} else {
		res.loaded = (res.handle != INVALID_DATA_HANDLE);
	}
	return(res);
}

KH_INTERN LoadResult
tri_mesh_loaded(Assets* assets, DataCache *cache, TriangleMeshID id) {
	LoadResult res;
	res.handle = get_handle_from_data_cache(cache, id.val);
	res.loaded = false;
	if(id.val) {
		Asset *asset = assets->arr + id.val;
		res.loaded = ((res.handle != INVALID_DATA_HANDLE) && (asset->state == AssetState_loaded));
		if(!res.loaded) {
			load_tri_mesh(assets, cache, id);
		}
	} else {
		res.loaded = (res.handle != INVALID_DATA_HANDLE);
	}
	return(res);
}

KH_INTERN u32
texture_load_force_immediate(Assets *assets, DataCache *cache, TextureID id) {

	Asset *asset = assets->arr + id.val;
	kh_assert(id.val);
	// u32 res = get_handle_from_data_cache(cache, id.val);
	if(asset->state != AssetState_loaded) {
		load_texture_immediate(assets, cache, id);
	}
	u32 res = get_handle_from_data_cache(cache, id.val);
	kh_assert(res != INVALID_DATA_HANDLE);
	return(res);
}

KH_INTERN u32
tri_mesh_load_force_immediate(Assets *assets, DataCache *cache, TriangleMeshID id) {
	Asset *asset = assets->arr + id.val;
	kh_assert(id.val);
	if(asset->state != AssetState_loaded) {
		load_tri_mesh_immediate(assets, cache, id);
	}
	u32 res = get_handle_from_data_cache(cache, id.val);
	kh_assert(res != INVALID_DATA_HANDLE);
	return(res);
}

KH_INTERN void
define_light(RenderManager *render, DirectionalLight light) {
	kh_assert(render->light_count < MAX_LIGHTS);
	render->lights[render->light_count++] = light;
}

// NOTE(flo): for now skybox and light should only be sent once, and not at each frame like other render commands
KH_INTERN void
define_skybox(Assets *assets, RenderManager *render, TextureID right, TextureID left, TextureID bottom, 
              TextureID top, TextureID back, TextureID front) {
	// texture_loaded(assets, render->cache, right);
	// texture_loaded(assets, render->cache, left);
	// texture_loaded(assets, render->cache, bottom);
	// texture_loaded(assets, render->cache, top);
	// texture_loaded(assets, render->cache, back);
	// texture_loaded(assets, render->cache, front);
	// g_platform.complete_all_queue_works(assets->load_queue);
	render->skybox.right = texture_load_force_immediate(assets, render->cache, right);
	render->skybox.left = texture_load_force_immediate(assets, render->cache, left);
	render->skybox.bottom = texture_load_force_immediate(assets, render->cache, bottom);
	render->skybox.top = texture_load_force_immediate(assets, render->cache, top);
	render->skybox.back = texture_load_force_immediate(assets, render->cache, back);
	render->skybox.front = texture_load_force_immediate(assets, render->cache, front);
	render->has_skybox = true;
}


KH_INLINE void
begin_render_frame(RenderManager *render) {
	render->batch_count = 0;
	render->render_entry_count = 0;
	render->commands_at = 0;
	render->first_vertex_buffer = 0;
}

KH_INTERN void
begin_vertex_format(RenderManager *render, VertexFormat format) {
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);

	VertexBuffer *entry = (VertexBuffer *)(render->commands + render->commands_at);
	entry->first = 0; 
	entry->format = format;
	entry->next = render->first_vertex_buffer;
	render->first_vertex_buffer = entry;

	render->cur_vertex_buffer = entry;
	render->commands_at += sizeof(VertexBuffer);
}

KH_INTERN void
begin_material(RenderManager *render, MaterialType type) {
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
	Material *entry = (Material *)(render->commands + render->commands_at);
	entry->first = 0;
	entry->type = type;

	VertexBuffer *format = render->cur_vertex_buffer;
	kh_assert(format);
	entry->next = format->first;
	format->first = entry;

	render->cur_material = entry;
	render->commands_at += sizeof(Material);
}

KH_INTERN void
begin_material_instance(RenderManager *render, Assets *assets, TextureID diffuse, TextureID normal, v4 color) {
	kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
	LoadResult diffuse_loaded = texture_loaded(assets, render->cache, diffuse);
	LoadResult normal_loaded = texture_loaded(assets, render->cache, normal);

	if(diffuse_loaded.loaded && normal_loaded.loaded) {
		MaterialInstance *entry = (MaterialInstance *)(render->commands + render->commands_at);
		entry->first = 0;
		entry->diffuse = diffuse_loaded.handle;
		entry->normal = normal_loaded.handle;
		entry->color = color;

		Material *mat = render->cur_material;
		kh_assert(mat);
		entry->next = mat->first;
		mat->first = entry;

		render->cur_mat_instance = entry;
		render->commands_at += sizeof(MaterialInstance);
	} else {
		render->cur_mat_instance = 0;
	}
}

KH_INTERN MeshRenderer *
push_mesh_renderer(RenderManager *render, Assets *assets, TriangleMeshID mesh) {
	MeshRenderer *res = 0;
	LoadResult mesh_result = tri_mesh_loaded(assets, render->cache, mesh);
	if(render->cur_mat_instance) {
		if(mesh_result.loaded) {
			kh_assert(render->commands_at + sizeof(VertexBuffer) <= render->commands_size);
			MeshRenderer *entry = (MeshRenderer *)(render->commands + render->commands_at);
			entry->first_entry = INVALID_RENDER_ENTRY;
			entry->entry_count = 0;
			entry->mesh = mesh_result.handle;

			MaterialInstance *mat = render->cur_mat_instance;
			kh_assert(mat);
			entry->next = mat->first;
			mat->first = entry;

			render->commands_at += sizeof(MeshRenderer);
			render->batch_count++;

			res = entry;
		}
	}
	return(res);
}

KH_INLINE void
end_vertex_format(RenderManager *render) {
	kh_assert(render->cur_material == 0);
	render->cur_vertex_buffer = 0;
}

KH_INLINE void
end_material(RenderManager *render) {
	kh_assert(render->cur_mat_instance == 0);
	render->cur_material = 0;
}

KH_INLINE void
end_material_instance(RenderManager *render) {
	render->cur_mat_instance = 0;
}

KH_INTERN void
push_render_entry(RenderManager *render, MeshRenderer *mesh_renderer, mat4 tr) {
	if(mesh_renderer) {
		u32 entry_id = render->render_entry_count++;
		RenderEntry *entry = render->render_entries + entry_id;
		entry->tr = tr;
		entry->next_in_mesh_renderer = mesh_renderer->first_entry;
		mesh_renderer->first_entry = entry_id;
		mesh_renderer->entry_count++;
	}
}

KH_INLINE DataCache
init_data_cache(umm data_size) {
	DataCache res;

	res.buffer.base = (u8 *)g_platform.virtual_alloc(data_size, 0);
	res.buffer.used = 0;
	res.buffer.size = data_size;

	for(u32 i = 0; i < MAX_DATA_HANDLES_IN_HASH; ++i)
	{
		res.hash[i].hash_index = INVALID_DATA_HANDLE;
		res.hash[i].gpu_index = INVALID_DATA_HANDLE;
		res.hash[i].next_in_hash = 0;
	}
	create_dummy_texture(&res);
	return(res);
}
