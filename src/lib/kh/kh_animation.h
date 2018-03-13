struct Animator {
	AssetID skeleton;
	AssetID skin;
	AssetID animation_clip;

	u32 joint_count;

	f64 cur_time;
	f64 utime;
	f32 playback_rate;
	b32 loop;

	u32 transformation_offset;

// #ifdef KH_IN_DEVELOPMENT
	b32 DEBUG_DELETE_LERP = false;
	b32 DEBUG_USE_NLERP = false;
// #endif

	u32 reserved___;
};

struct JointTransform {
	u32 count;
	u32 max_count;
	mat4 *data;
};

struct AnimatorArray {
	u32 count;
	u32 max_count;
	Animator *data;
};
