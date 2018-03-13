#define FOREACH_SAMPLE(SAMPLE) \
	SAMPLE(basic_mesh) \
	SAMPLE(basic_scene) \
	SAMPLE(basic_animation) \
	SAMPLE(scene_pathtracer) \
	SAMPLE(radiance_transfer_view) \

// TODO(flo): we need to manage SO files on linux
#define GENERATE_DLL_NAMES(NAME) "kh_" #NAME "_sample.dll",
#define GENERATE_DLL_ENUM(ENUM) Sample_##ENUM,

enum PracticeSample {
	Sample_none,
	FOREACH_SAMPLE(GENERATE_DLL_ENUM)
};

struct SampleLibrary {
	void *lib;
	b32 is_loaded;
	kh_update *frame_update;

};

enum RendererType {
	RendererType_software,
	RendererType_opengl,
};

struct SDLState {
	SDL_Window *window;
	u32 fullscreen_flags;

	RendererType renderer;

	PracticeSample new_sample;
	PracticeSample cur_sample;
};