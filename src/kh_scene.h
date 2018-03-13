// TODO(flo): component based?
struct Entity {
	u32 mesh_renderer;
	u32 animator;
	// TODO(flo): we should replace this by Transform_SQT
	mat4 transform;
};

struct SceneEntity {
	// TODO(flo): we certainly do not need this
	u32 index;
	Entity entity;
	SceneEntity *next_free;
};

struct Scene {
	LinearArena *arena;

	u32 max_pool_count;
	u32 pool_count;
	u32 entity_count;
	SceneEntity *entity_pool;
	SceneEntity *first_free_entity;
	SceneEntity free_sentinel;
};