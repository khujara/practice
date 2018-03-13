/* @TODO(flo) : MAIN TODO

	- get rid of malloc
	- Implement a 3.1 Opengl API
	OPENGL :
		- multiple lights
		- gamma correction
		- use program pipeline for similar vertex shaders
		- mipmaps
		- bloom
		- deferred shading
		- screen space ambient ocllusion
		- cascade shadow mapping
		- voxel cone tracing
		- pbr
		- IMPORTANT(flo): we need to sync our persistent map buffers !
*/

#define MAX_ENTRIES 256
enum AttributeBinding {
	ATTRIB_VERTEX_DATA = 0,
	ATTRIB_ANIMATION   = 1,
	ATTRIB_DRAWCMDID   = 2,
	ATTRIB_DRAWID      = 3,
};

enum ShaderLocation {
	LOC_POSITION      = 0,
	LOC_NORMAL        = 1,
	LOC_UV0           = 2,
	LOC_TANGENT       = 3,
	LOC_BITANGENT     = 4,
	LOC_BONE_IDS	  = 5,
	LOC_BONE_WEIGHTS  = 6,
	LOC_DRAWID        = 7,
	LOC_INDIRECTCMDID = 8,
};

enum ShaderBinding {
	BIND_MATERIAL        = 0,
	BIND_TRANSFORM       = 1,
	BIND_LIGHT           = 2,
	BIND_CAMERATRANSFORM = 3,
	BIND_VAO0            = 4,
	BIND_LIGHTTRANSFORM  = 5,
	BIND_SHADOWMAP       = 6,
	BIND_BONETRANSFORM   = 7,
	BIND_BONEOFFSET      = 8,
	BIND_TEXTURES        = 0,
};

#define MAX_VERTICES_COUNT megabytes(3)
#define MAX_TRIANGLES_COUNT megabytes(1)
#define MAX_TEXTURES 64
#define MAX_MESHES 64
#define MAX_TEXTURE_BINDINGS 16
#define MAX_DRAWCMD_PER_FRAME 16
#define MAX_SHADERS_USED_FOR_FRAME 16
#define BIND_SKYBOX MAX_TEXTURE_BINDINGS
#define BIND_PREFILTER (BIND_SKYBOX + 1)
#define BIND_BRDF_LUT (BIND_PREFILTER + 1)
struct Ogl_4_5 {
	OglCameraTransform *map_cam_transform;
	OglTransform *map_transforms;

	u32 *map_indirectcmdids;
	u32 *map_drawids;
	u32 *map_boneoffset;

	DrawElementsIndirectCommand cmds[MAX_DRAWCMD_PER_FRAME];

	GLuint cmds_buffer;

	OglTexture2DContainer *shadow_ctr;

	b32 has_skybox;
	GLuint skybox_tex;
	GLuint irradiance_tex;
	GLuint prefilter_tex;
	GLuint brdf_LUT;
	// OglTexture2D brdf_LUT;

	u32 light_count;
	OglLight lights[MAX_LIGHTS];

	// TODO(flo): if we need to have more texture container thant TEXTURE_BINDINGS (which shoudl be retrive from opengl)
	// we need to handle this case.
	u32 texture_container_count;
	GLuint texture_container_names[MAX_TEXTURE_BINDINGS];	
	OglTexture2DContainer texture_containers[MAX_TEXTURE_BINDINGS];

	u32 texture_count;
	// TODO(flo): not on the stack PLIZ!!!
	OglTexture2D textures[MAX_TEXTURES];

	u32 mesh_count;
	// TODO(flo): NOT ON THE STACK PLIIIZZ!!
	OglTriangleMesh meshes[MAX_MESHES];

	OglVertexBuffer vertex_buffers[VertexFormat_count_or_none];

	u32 used_shaders_count;
	u32 used_shaders[MAX_SHADERS_USED_FOR_FRAME];
	u32 shader_max_count;
	u32 shader_count;
	OglShader *shaders;

	OglPass pass[Pass_count];

	GLuint joint_transform;

	GLint shader_storage_buffer_alignment;
	GLint texture_address_alignment;

	GLuint pixel_unpack_buffer;
};
