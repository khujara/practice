// TODO(flo): since AssetName_count, AssetTag_count and AssetName should not be present here
// we should rearch structs and datas that need these! AssetName_count and AssetTag_count should be
// encoded in the file header

// TODO(flo): we need to have different buffers that can fit to our vertex formats to
// so that we could have a map pointer directly to gpu. We'll need to split our datas and our headers


struct DataCache {
	// GeneralPurposeArena arena;
	LinearArena arena;
};

enum AssetState {
	AssetState_not_loaded,
	AssetState_in_queue,
	AssetState_loaded,
};

struct AssetNameArray {
	u32 first_asset;
	u32 one_past_last_asset;
};

struct AssetFile {
	FileHandle hdl;
	AssetFileHeader header;
	SourceAssetType *asset_type_arr;
};

struct AssetHeader {
	u8 *data;
	u32 gpu_index;
	b32 gpu_reload;
};

struct Asset {
	AssetHeader header;
	u32 state;
	SourceAsset source;

#if defined(KH_IN_DEVELOPMENT) || defined(KH_EDITOR_MODE)
	// TODO(flo): not here should just have an index to an union {AssetName; char *} array
	union {
		AssetName name;
		char *cname;
	};
#endif
}; 

struct LoadAssetTask {
	TaskWithMemory *mem_task;
	Asset *asset;
	FileHandle *hdl;
	void *dst;

	u64 bytes_to_read;
	u64 offset;
	u32 final_state;
};

struct LoadedAsset {
	AssetType *type;
	u8 *data;
};

struct AdditionalAsset
{
	u32 id;
	char *name;
	AdditionalAsset *next_in_hash;
};

struct AssetTagVector {
	f32 matches;
	f32 weights;
};

#define MAX_IN_GAME_ASSETS 128
#define INVALID_GPU_INDEX 0xFFFFFFFF

#ifdef KH_EDITOR_MODE
struct AssetEnumHashEl {
	u32 value;
	char *name;
	AssetEnumHashEl *next_in_hash;
};

struct AssetEnumHash {
	AssetEnumHashEl *elements[256];
	#ifdef KH_IN_DEVELOPMENT
	u32 collision_count;
	u32 remaining_slots;	
	#endif
};
#endif

enum PrimitiveType {
	Primitive_cube,
	Primitive_sphere,
	Primitive_plane,
	Primitive_double_sided_plane,
	Primitive_count,
};

typedef void LoadAssetGPU(struct Assets *assets, AssetID id);

struct Assets {
	LinearArena arena;

	u32 file_count;
	AssetFile *file_arr;
	
	u32 tag_count;
	AssetTag *tag_arr;


	u32 count_from_package;
	u32 count_additional;
	u32 total_count;
	Asset *arr;

#ifdef KH_DEBUG
	u32 hash_collision_count;
#endif
	u32 one_past_last_asset_id;

	AdditionalAsset *additional_assets[MAX_IN_GAME_ASSETS];

	TaskWithMemory load_tasks[4];
	WorkQueue *load_queue;

	AssetNameArray name_arr[AssetName_count];

	char *primitive_names[Primitive_count];

	AssetTagVector tag_vectors[AssetTag_count];

	DataCache cache;

	// TODO(flo): implement this
	LoadAssetGPU *load_asset_gpu;

#ifdef KH_EDITOR_MODE
	AssetEnumHash enum_hash;
#endif
};
