enum PassType {
	Pass_shadow,
	Pass_zpre,
	Pass_render_3d_scene,
	Pass_skybox,
	Pass_blit,
	Pass_count,
};

enum ShaderType {
	ShaderType_vertex,
	ShaderType_fragment,
	ShaderType_geometry,
	ShaderType_none,
};

#define MAX_HASH_GL_EXTENSIONS 256
struct OglExtension {
	char *name;
	OglExtension *next_in_hash;
};

struct OglExtensionsHash {
	OglExtension *hash[MAX_HASH_GL_EXTENSIONS];
	u32 empty;
	u32 collision_count;
	u32 count;
};

typedef struct DrawElementsIndirectCommand {
	GLuint count;
	GLuint primCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
} DrawElementsIndirectCommand;

struct OglBindlessTextureAddress {
	GLuint64 handle;
	GLfloat page;
};

struct OglTextureAddress {
	GLuint container;
	GLfloat page;
};

struct OglTexture2D {
	GLuint64 bindless;
	GLuint name;
	GLuint container_id;
	u32 slice;
};

struct OglTexture2DContainer {
	u32 arr_index;
	GLuint name;
	GLuint64 bindless;
	u32 slices_count;
	u32 slices_max;
	u32 mipmaps_count;
	GLenum format;
	u32 width;
	u32 height;
	b32 sparse;
};

struct OglCameraTransform {
	mat4 viewproj;
	mat4 view;
	mat4 rot_vp;
	v4 pos;
};

struct OglTransform {
	mat4 model;
};

struct OglPass {
	GLuint framebuffer;
	GLuint prog_name;

	u32 vertexbuffer_size;
	GLuint vertexbuffer;

	u32 matbuffer_size;
	GLuint matbuffer;
};

struct OglShader {
	// NOTE(flo): permanent
	GLuint prog_name;
	VertexFormat format;
	u32 size;

	// NOTE(flo): reset each frame
	u32 render_count;
	u32 cmd_count;
	u32 cmd_offset;
	u32 offset;
	u32 instance_count;

};

struct OglVerticesBufferData {
	GLuint name;
	u32 total_size;
	u32 offset;
};

struct OglIndicesBufferData {
	GLuint name;
	u32 total_size;
	u32 offset;
};

struct OglVertexBuffer {

	u32 mesh_count;
	u32 ibo_at;
	u32 vbo_at;

	OglVerticesBufferData verts;
	OglIndicesBufferData inds;

	u32 attrib_offset;
	u32 attrib_stride;

	b32 skinned;
	u32 anim_offset;
};

struct OglTriangleMesh {
	u32 ind_count;
	u32 vbo_offset;
	u32 ibo_offset;
	u32 draw_id;
};

struct OglLight {
	GLuint framebuffer;
	GLuint buffer;
	GLuint transform;
	GLuint shadow_buffer;
	OglTexture2D shadow_tex;
};

// TODO(flo): we need better handling of this;
struct OglTextureParam {
	GLuint filter;
	GLuint wrap;
	b32 swizzle;
	GLuint swizzles[4];
};

#include "kh_renderer_ogl_3_2.h"
#include "kh_renderer_ogl_4_5.h"

typedef void OGLAPI_init(struct OglAPI *ogl, struct RenderManager *render, struct Assets *assets);
typedef void OGLAPI_render(struct OglAPI *ogl, struct RenderManager *render, struct Assets *assets);
typedef void OGLAPI_display(struct OglAPI *ogl, u32 w, u32 h, void *pixels, b32 blit);
typedef void OGLAPI_reset(struct OglAPI *ogl);
typedef GLuint OGLAPI_create_vert_frag_prog(struct OglAPI *api, char **files, u32 file_count, VertexFormat format, MaterialType type);

enum OglAPI_Version {
	OglAPI_Version_none,
	OglAPI_Version_3_2,
	OglAPI_Version_4_5,
};

struct OglShaderParam {
	VertexFormat format;
	ShadingType type;
	MaterialType instance_type;
	char *frag_file;
	char *vert_file;
};

struct OglAPI {
	OglAPI_Version version = OglAPI_Version_none;
	LinearArena *arena;

	b32 wireframe;
	b32 bindless;
	b32 sparse;
	b32 multisample;
	u32 multisample_count;
	b32 zprepass_enabled;
	u32 target_w;
	u32 target_h;
	OglTexture2D target;

	union {
		struct Ogl_4_5 ver_4_5;
		struct Ogl_3_2 ver_3_2;
	};
	
	OglExtensionsHash exts;

	OGLAPI_init *init;
	OGLAPI_render *render;
	OGLAPI_display *display;
	OGLAPI_reset *reset;
	OGLAPI_create_vert_frag_prog *create_vert_frag_prog;
};


#include "kh_renderer_ogl.cpp"
#include "kh_renderer_ogl_3_2.cpp"
#include "kh_renderer_ogl_4_5.cpp"

KH_INTERN void
ogl_set_api_version(OglAPI_Version ver, OglAPI *ogl) {
	ogl->version = ver;
	if(ver != OglAPI_Version_none) {
		if(ver == OglAPI_Version_4_5) {
			ogl->init = ogl45_init;
			ogl->render = ogl45_render;
			ogl->display = ogl45_display;
			ogl->reset = ogl45_reset;
			ogl->create_vert_frag_prog = ogl45_create_vert_frag_prog;
			ogl->ver_4_5 = {};
		} else if(ver == OglAPI_Version_3_2) {
			ogl->init = ogl32_init;
			kh_assert(!"ogl version not supported");
		}
		OglExtensionsHash *hash = &ogl->exts;
		ogl_set_extensions_hash(hash, ogl->arena);
	} else {
		ogl->init = 0;
		ogl->render = 0;
		ogl->display = 0;
		ogl->reset = 0;
		ogl->create_vert_frag_prog = 0;
	}
}