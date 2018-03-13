#define FRAME_BUFFER_BYTES_PER_PIXEL 4
#define MAX_DATA_HANDLES_IN_HASH 512
// TODO(flo): support multiple lights and different light types (point, spot)
#define MAX_LIGHTS 1

enum LightType {
	LightType_directional,
	LightType_point,
	LightType_spot,
	LightType_count,
};

struct Light {
	LightType type;
	v3 color;
	f32 cutoff;
	f32 outer_cutoff;
	v3 pos;
	v3 dir;
	f32 scale;
	f32 z_near;
	f32 z_far;
};

struct Skybox {
	AssetID right;
	AssetID left;
	AssetID bottom;
	AssetID top;
	AssetID back;
	AssetID front;
};

enum MaterialValue {
	MaterialValue_texture,
	MaterialValue_f32,
	MaterialValue_v4,
	MaterialValue_count,
};

struct MaterialParam {
	char *name;
	MaterialValue type;
	u32 advance_size;
};

struct MaterialInfo {
	u32 param_count;
	u32 first_param_index;
	u32 texture_count;
	u32 size;
};

struct RenderEntry {
	u32 next_in_mesh_renderer;
	u32 bone_transform_offset;
	u32 scene_index; //0 = this is not part of the scene entries
	// TODO(flo): Transform_SQT
	mat4 tr;
};

struct MeshRenderer {
	AssetID mesh;
	u32 entry_count;
	u32 first_entry;
	b32 loaded;
	// TODO(flo): CLEANUP(flo) : highly dislike this backtracking!
	u32 mat_off;
	MeshRenderer *next;
};

struct Material {
	// TODO(flo): CLEANUP(flo): highly dislike this backtracking!
	u32 shading_off;
	Material *next;
	MeshRenderer *first;
};

struct Shading {
	VertexFormat format;
	ShadingType type;
	MaterialType mat_type;
	u32 shader_index;
	u32 texture_count;
	u32 size;
	Shading *next;
	Material *first;
};

struct VertexBuffer {
	VertexFormat format;
	VertexBuffer *next;
	Shading *first;
};

struct MaterialHashEl {
	// NOTE(flo): should be attribute by the renderer layer (opengl etc.. if needed)
	u32 key;
	u16 shader_index;
	u16 buffer_index;
	u32 one_past_next_in_hash;

	// NOTE(flo): should be reset every time we load a program
	u32 cmd_offset;
};

struct RenderManager {

	Camera camera;
	u32 width, height;
	u32 wnd_w, wnd_h;

	// -----------------------------------------------
	// NOTE(flo): we need to clear this each frame
	u32 entry_count;
	u32 batch_count;
	// --------------------------------------------------------

	VertexBuffer *first_vertex_buffer;

	i32 vertex_format_offset_hash[VertexFormat_count_or_none];
	MaterialType shading_mat_type[Shading_count];
	
	u32 mat_buffer_count;
	u32 mat_buffer_max_count;
	MaterialHashEl *mat_buffer;

	MaterialHashEl *mat_used[32];

	u32 commands_at;
	u32 commands_size;
	u8 *commands;

	u32 max_entries;
	RenderEntry *render_entries; 

	// TODO(flo): it seems we only need bone_transformations here
	// TODO(flo): Pool allocator (free list)
	AnimatorArray animators;

	// TODO(flo): Pool allocator (free list)
	// NOTE(flo): we need to clear the count each frame!
	JointTransform joint_tr;

	b32 has_skybox;
	Skybox skybox;
	u32 light_count;
	Light lights[MAX_LIGHTS];

	u32 mat_params_count;
	MaterialParam *mat_params;
	MaterialInfo mat_infos[Type_Material_count];
};

KH_INTERN u32
add_shading(RenderManager *render, ShadingType type, VertexFormat format, u32 *shader_count, u32 max_shader) {
	u32 hash_key = (u32)format * Shading_count + type;
	u32 hash_slot = hash_key & (array_count(render->mat_used) - 1);

	MaterialHashEl **first = render->mat_used + hash_slot;
	MaterialHashEl *search = *first;
	while(search) {
		// NOTE(flo): this should not be already in the hash!
		kh_assert(search->key != hash_key);
		if(!search->one_past_next_in_hash) break;
		search = render->mat_buffer + (search->one_past_next_in_hash - 1);
	}

	kh_assert(render->mat_buffer_count < render->mat_buffer_max_count);
	u32 index = render->mat_buffer_count++;
	MaterialHashEl *added = render->mat_buffer + index;
	kh_assert(*shader_count < max_shader);
	u32 shader_index = *shader_count;
	*shader_count += 1;
	added->shader_index = shader_index;
	added->key = hash_key;
	added->cmd_offset = 0;
	added->buffer_index = index;
	added->one_past_next_in_hash = *first ? (*first)->buffer_index + 1 : 0;
	*first = added;
	return(shader_index);
}