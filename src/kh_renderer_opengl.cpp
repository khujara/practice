/* @TODO(flo) : MAIN TODO

	- get rid of malloc
	- Implement a 3.1 Opengl API
	OPENGL :
		- multiple lights
		- gamma correction
		- use program pipeline for similar vertex shaders
		- mipmaps
		- multisampling
		- bloom
		- deferred shading
		- screen space ambient ocllusion
		- cascade shadow mapping
		- animations
		- voxel cone tracing
		- pbr
		- IMPORTANT(flo): we need to sync our persistent map buffers !
	API :
		- Map our vbos and ibos instead of buffer data to avoid having our datas in both cpu and gpu memory
		- use SQT transform in the game update, the mat4 should be generated afterwards
*/

#define MAX_ENTRIES 128

enum AttributeBinding {
	ATTRIB_VERTEX_DATA = 0,
	ATTRIB_DRAWID      = 1,
	ATTRIB_DRAWCMDID   = 2,
	ATTRIB_ANIMATION   = 3,
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
	BIND_COLORS          = 2,
	BIND_LIGHT           = 3,
	BIND_CAMERATRANSFORM = 4,
	BIND_VAO0            = 5,
	BIND_LIGHTTRANSFORM  = 6,
	BIND_SHADOWMAP       = 7,
	BIND_BONETRANSFORM   = 8,
	BIND_BONEOFFSET      = 9,
	BIND_TEXTURES        = 0,
};

struct ShaderFile {
	OglMaterial *material;
	char *vert_file;
	char *frag_file;
	VertexFormat format;
};

enum PassType {
	Pass_shadow,
	Pass_render_3d_scene,
	Pass_skybox,
	Pass_blit,
	Pass_count,
};

#define MAX_COLORS 16
#define MAX_TEXTURES 64
#define MAX_MESHES 64
#define MAX_TEXTURE_BINDINGS 16
#define MAX_DRAWCMD_PER_FRAME 8
#define BIND_SKYBOX MAX_TEXTURE_BINDINGS
struct OglState {
	b32 bindless;
	b32 sparse;
	b32 zprepass_enabled;

	OglCameraTransform *map_cam_transform;
	OglTransform *map_transforms;
	u32 *map_drawcmdsids;
	u32 *map_drawids;

	DrawElementsIndirectCommand cmds[MAX_DRAWCMD_PER_FRAME];

	GLuint color_buffer;
	GLuint cmds_buffer;

	OglTexture2D shadow_map;

	b32 has_skybox;
	OglSkybox skybox;

	u32 light_count;
	OglLight lights[MAX_LIGHTS];

	// TODO(flo): if we need to have more texture container thant TEXTURE_BINDINGS (which shoudl be retrive from opengl)
	// we need to handle this case.
	u32 texture_container_count;
	GLuint texture_container_names[MAX_TEXTURE_BINDINGS];	
	OglTexture2DContainer texture_containers[MAX_TEXTURE_BINDINGS];

	u32 texture_count;
	OglTexture2D textures[MAX_TEXTURES];

	u32 mesh_count;
	OglTriangleMesh meshes[MAX_MESHES];
	OglVertexBuffer vertex_buffers[VertexFormat_count];

	u32 used_material_count;
	u32 used_materials[Material_count];
	OglMaterial materials[Material_count];

	OglExtensionsHash exts;

	OglTexture2D target;

	OglPass ogl_pass[Pass_count];

	GLuint bone_transform;

	u32 *map_boneoffset;
	// GLuint bone_offsets;

};

GL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
    if(severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH) {
    	GLenum err = glGetError();	
    	char *err_mess = (char *)message;
    	kh_assert(!"OpenGL Error encountered");
    }
}

KH_INTERN void
check_required_ogl_extensions(OglExtensionsHash *hash) {
	char *required_exts[] = {
		"GL_ARB_direct_state_access",
		"GL_ARB_buffer_storage",
		"GL_ARB_multi_draw_indirect",
		"GL_ARB_shader_storage_buffer_object",
		"GL_ARB_uniform_buffer_object",
		"GL_ARB_sync",
		"GL_ARB_texture_view",
		"GL_ARB_map_buffer_range",
		"GL_ARB_vertex_attrib_binding",
		// "GL_ARB_sparse_texture",
		// "GL_ARB_bindless_texture",
		// "GL_ARB_shader_draw_parameters",
		// "GL_ARB_NV_command_list",
		// "GL_ARB_parallel_shader_compile",
	};
	ogl_check_ext_list(hash, required_exts, array_count(required_exts));
}

KH_INTERN GLuint
create_vert_frag_prog(StackAllocator *memstack, char *vert_file, char *frag_file, b32 bindless) {
	char vert_header[4096];

	stbsp_sprintf(vert_header,
		"#version 450 core\n"
		"#extension GL_ARB_shader_storage_buffer_object : require\n"
		"#define LOC_POSITION %d\n"
		"#define LOC_NORMAL %d\n"
		"#define LOC_UV0 %d\n"
		"#define LOC_DRAWID %d\n"
		"#define LOC_INDIRECTCMDID %d\n"
		"#define LOC_TANGENT %d\n"
		"#define LOC_BITANGENT %d\n"
		"#define LOC_BONE_IDS %d\n"
		"#define LOC_BONE_WEIGHTS %d\n"
		"#define BIND_TRANSFORM %d\n"
		"#define BIND_CAMERATRANSFORM %d\n"
		"#define BIND_LIGHTTRANSFORM %d\n"
		"#define BIND_BONETRANSFORM %d\n"
		"#define BIND_BONEOFFSET %d\n\n",
		LOC_POSITION, LOC_NORMAL, LOC_UV0, LOC_DRAWID, LOC_INDIRECTCMDID, LOC_TANGENT, LOC_BITANGENT, LOC_BONE_IDS, 
		LOC_BONE_WEIGHTS, BIND_TRANSFORM, BIND_CAMERATRANSFORM, BIND_LIGHTTRANSFORM, BIND_BONETRANSFORM, BIND_BONEOFFSET);

	/* @NOTE(flo): this is only for RenderDoc support since it does not support these extensions atm :
		cf : https://github.com/baldurk/renderdoc/blob/master/renderdoc/driver/gl/gl_driver.cpp
		"Extensions I plan tu support ..."
		list of extensions not supported that we do want to use :
			GL_ARB_bindless_texture
			GL_ARB_sparse_buffer
			GL_ARB_sparse_texture
			GL_ARB_sparse_texture2
			GL_ARB_parallel_shader_compile
	*/
	// TODO(flo): for now we have the same material structure for every program, maybe we want a specific material
	// structure for each program
	char *bindless_header;
	if(bindless) {
		bindless_header = 
			"#extension GL_ARB_bindless_texture : require\n"
			"struct TexAddress {\n"
			"	sampler2DArray container;\n"
			"	float page;\n"
			"};\n"
			"vec4 custom_texture(TexAddress addr, vec2 uv) {\n"
			"	return texture(addr.container, vec3(uv, addr.page));\n"
			"}\n";
	} else {
		bindless_header = 
			"struct TexAddress {\n"
			"	uint container;\n"
			"	float page;\n"
			"};\n"
			"layout (binding = BIND_TEXTURES) uniform sampler2DArray texarray_container[MAX_TEXTURE_BINDINGS];\n"
			"vec4 custom_texture(TexAddress addr, vec2 uv) {\n"
			"	return texture(texarray_container[addr.container], vec3(uv, float(addr.page)));\n"
			"}\n";
	}

	u32 bindless_support_l = string_length(bindless_header);
	char frag_header_begin[4096];
	char frag_header[4096];

	// u32 num_textures = 2;
	/* layout(std140, binding = BIND_MATERIAL) buffer TEXTURE_BLOCK {
			TexAddres tex[num_textures];
		}
	*/

	stbsp_sprintf(frag_header_begin, 
     	"#version 450 core\n"
		"#extension GL_ARB_shader_storage_buffer_object : require\n"
		"#define BIND_MATERIAL %d\n"
		"#define BIND_COLORS %d\n"
		"#define BIND_LIGHT %d\n"
		"#define BIND_SHADOWMAP %d\n"
		"#define BIND_TEXTURES %d\n"
		"#define BIND_SKYBOX %d\n"
		"#define MAX_COLORS %d\n"
		"#define MAX_TEXTURE_BINDINGS %d\n"
		,BIND_MATERIAL, BIND_COLORS, BIND_LIGHT, BIND_SHADOWMAP, BIND_TEXTURES, BIND_SKYBOX,
		MAX_COLORS, MAX_TEXTURE_BINDINGS);

	u32 frag_header_begin_l = string_length(frag_header_begin);

	strings_copy_NNT(frag_header_begin_l, frag_header_begin, frag_header);
	strings_copy(bindless_support_l, bindless_header, frag_header + frag_header_begin_l);

	u32 vert_header_l = string_length(vert_header);
	u32 frag_header_l = string_length(frag_header);

	GLchar *vert_src = ogl_load_shader_from_file(vert_header, vert_header_l, vert_file, memstack);
	GLchar *frag_src = ogl_load_shader_from_file(frag_header, frag_header_l, frag_file, memstack);

	GLuint vert_name = ogl_compile_shader(GL_VERTEX_SHADER, 1, vert_src, NULL);
	GLuint frag_name = ogl_compile_shader(GL_FRAGMENT_SHADER, 1, frag_src, NULL);

	GLuint res = glCreateProgram();
	glAttachShader(res, vert_name);
	glAttachShader(res, frag_name);
	glLinkProgram(res);

	WIN32DEBUG_opengl_get_shader_log(vert_name);
	WIN32DEBUG_opengl_get_shader_log(frag_name);
	WIN32DEBUG_opengl_get_prog_log(res);

	glDeleteShader(vert_name);
	glDeleteShader(frag_name);

	return(res);
}

KH_INLINE OglTexture2DContainer *
create_texture_2D_container(OglState *ogl, u32 slices_count, u32 mipmaps_count, GLenum internalformat, u32 w, u32 h) {
	kh_assert(ogl->texture_container_count < MAX_TEXTURE_BINDINGS);
	u32 id = ogl->texture_container_count++;
	OglTexture2DContainer *res = ogl->texture_containers + id; 
	*res = ogl_create_texture_2D_container(slices_count, mipmaps_count, internalformat, w, h, id, ogl->sparse, ogl->bindless);
	ogl->texture_container_names[id] = res->name;
	return(res);
}

KH_INTERN void
create_vert_frag_prog(OglState *ogl, MaterialType type, StackAllocator *memstack, char *vert_file, char *frag_file, VertexFormat format) {
	OglMaterial *mat = ogl->materials + type;
	mat->prog_name = create_vert_frag_prog(memstack, vert_file, frag_file, ogl->bindless);
	mat->format = format;
}

// TODO(flo): do a better job than O(n) for searching here
KH_INTERN OglTexture2DContainer *
get_texture_container(OglState *ogl, u32 w, u32 h, u32 mipmaps_count, GLenum format) {
	OglTexture2DContainer *res = 0;
	for(u32 i = 0; i < ogl->texture_container_count; ++i) {
		OglTexture2DContainer *search = ogl->texture_containers + i;
		if(search->width == w && search->height == h && search->mipmaps_count == mipmaps_count && search->format == format) {
			res = search;
			break;
		}
	}
	if(!res) {
		res = create_texture_2D_container(ogl, 2, mipmaps_count, format, w, h);	
	}
	kh_assert(res);
	return(res);
}

KH_INLINE OglTexture2D *
get_ogl_texture_2d(OglState *ogl, RenderManager *render, Assets *assets, AssetID id) {
	OglTexture2D *res;
	Asset *asset = get_asset(assets, id, AssetType_tex2d);
	if(asset->header.gpu_index == INVALID_GPU_INDEX) {
		kh_assert(ogl->texture_count <= MAX_TEXTURES);
		u32 index = ogl->texture_count++;
		asset->header.gpu_index = index;
		res = ogl->textures + index;
		Texture2D tex = asset->source.type.tex2d;
		u8 *tex_data = get_datas_from_asset(assets, asset);
		kh_assert(tex.bytes_per_pixel == 3);
		OglTexture2DContainer *container = get_texture_container(ogl, tex.width, tex.height, 1, GL_RGB8);
		ogl_add_texture_2D_to_container(container, res, &tex, tex_data, GL_BGR);

	} else {
		res = ogl->textures + asset->header.gpu_index;
	}
	return(res);
}

KH_INLINE OglTriangleMesh *
get_ogl_triangle_mesh(OglState *ogl, RenderManager *render, Assets *assets, AssetID id, VertexFormat format) {
	OglTriangleMesh *res;
	Asset *asset = get_asset(assets, id, AssetType_trimesh);
	if(asset->header.gpu_index == INVALID_GPU_INDEX) {
		kh_assert(ogl->mesh_count <= MAX_MESHES);
		u32 index = ogl->mesh_count++;
		asset->header.gpu_index = index;
		res = ogl->meshes + index;
		OglVertexBuffer *vertex_buffer = ogl->vertex_buffers + format;
		TriangleMesh mesh = asset->source.type.trimesh;
		u8 *mesh_data = get_datas_from_asset(assets, asset);
		kh_assert(mesh.format == format);
		ogl_add_triangle_mesh_memory_to_vbo(res, vertex_buffer, &mesh, mesh_data);
	} else {
		res = ogl->meshes + asset->header.gpu_index;
	}
	return(res);
}

KH_INLINE void
ogl_set_skybox(OglState *ogl, RenderManager *render, Assets *assets) {
	GLfloat skybox_vert[] = {
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
	};

	GLuint vertices;
	glCreateBuffers(1, &vertices);
	glNamedBufferData(vertices, sizeof(skybox_vert), skybox_vert, GL_STATIC_DRAW);
	ogl->ogl_pass[Pass_skybox].vertexbuffer = vertices;

	LoadedAsset right = get_loaded_asset(assets, render->skybox.right, AssetType_tex2d);
	LoadedAsset left = get_loaded_asset(assets, render->skybox.left, AssetType_tex2d);
	LoadedAsset bottom = get_loaded_asset(assets, render->skybox.bottom, AssetType_tex2d);
	LoadedAsset top = get_loaded_asset(assets, render->skybox.top, AssetType_tex2d);
	LoadedAsset back = get_loaded_asset(assets, render->skybox.back, AssetType_tex2d);
	LoadedAsset front = get_loaded_asset(assets, render->skybox.front, AssetType_tex2d);

	GLuint skybox_tex;
	// glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &skybox_tex);
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skybox_tex);
	glTextureParameteri(skybox_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(skybox_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// glTextureStorage3D(skybox_tex, 1, GL_RGB8, right->width, right->height, 6);
	glTextureStorage2D(skybox_tex, 1, GL_RGB8, right.type->tex2d.width, right.type->tex2d.height);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 0, right.type->tex2d.width, right.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, right.data);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 1, left.type->tex2d.width, left.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, left.data);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 2, bottom.type->tex2d.width, bottom.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, bottom.data);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 3, top.type->tex2d.width, top.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, top.data);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 4, back.type->tex2d.width, back.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, back.data);
	glTextureSubImage3D(skybox_tex, 0, 0, 0, 5, front.type->tex2d.width, front.type->tex2d.height, 1, GL_BGR, GL_UNSIGNED_BYTE, front.data);

	glBindTextureUnit(BIND_SKYBOX, skybox_tex);

	ogl->skybox.vertices = vertices;
	ogl->skybox.texture = skybox_tex;
}

KH_INTERN void
ogl_delete_skybox(OglState *ogl) {
	if(ogl->has_skybox) {
		// glBindTextureUnit(BIND_SKYBOX, 0);
		glDeleteBuffers(1, &ogl->skybox.vertices);
		glDeleteTextures(1, &ogl->skybox.texture);
		ogl->skybox.vertices = 0;
		ogl->skybox.texture = 0;
		ogl->has_skybox = false;
	}
}

KH_INTERN void
ogl_set_light(OglState *ogl, DirectionalLight *light, u32 light_index) {

	v3 light_dir = light->dir;

	GLuint dirlight_buff;
	glCreateBuffers(1, &dirlight_buff);
	glNamedBufferStorage(dirlight_buff, sizeof(DirectionalLight), light, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHT, dirlight_buff);

	OglTexture2DContainer *container_shadow = get_texture_container(ogl, 4096, 4096, 1, GL_DEPTH_COMPONENT24);
	// @TEMP: we need to handle this proper
	ogl_set_texture_2d_to_container(container_shadow, &ogl->shadow_map, light_index);

	GLuint *shadow_fb = &ogl->ogl_pass[Pass_shadow].framebuffer;
	glCreateFramebuffers(1, shadow_fb);

	// GLuint sh_tex;
	// glGenTextures(1, &sh_tex);
	// glTextureView(sh_tex, GL_TEXTURE_2D, container_shadow->name, GL_DEPTH_COMPONENT24, 0, 1, 0, 1);
	// glNamedFramebufferTexture(*shadow_fb, GL_DEPTH_ATTACHMENT, sh_tex, 0);
	glNamedFramebufferTextureLayer(*shadow_fb, GL_DEPTH_ATTACHMENT, ogl->shadow_map.name, 0, ogl->shadow_map.slice);
	ogl_get_fb_status(*shadow_fb, GL_FRAMEBUFFER);

	GLuint shadow_b;
	glCreateBuffers(1, &shadow_b);
	if(ogl->bindless) {
		OglBindlessTextureAddress addr = {ogl->shadow_map.bindless, (f32)ogl->shadow_map.slice, 0};
		glNamedBufferData(shadow_b, sizeof(OglBindlessTextureAddress), &addr, GL_STATIC_DRAW);
	} else {
		OglTextureAddress addr = {ogl->shadow_map.container_id, (f32)ogl->shadow_map.slice, 0, 0};
		glNamedBufferData(shadow_b, sizeof(OglTextureAddress), &addr, GL_STATIC_DRAW);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_SHADOWMAP, shadow_b);

	mat4 light_view;

	const v3 up_axis = kh_vec3(0, 1.0f, 0.0f);

	v3 z_axis = light_dir;
	v3 x_axis = kh_cross_v3(up_axis, z_axis);
	v3 y_axis = kh_cross_v3(z_axis, x_axis);

	light_view.m00 = x_axis.x; light_view.m01 = y_axis.x; light_view.m02 = z_axis.x; light_view.m03 = 0;
	light_view.m10 = x_axis.y; light_view.m11 = y_axis.y; light_view.m12 = z_axis.y; light_view.m13 = 0;
	light_view.m20 = x_axis.z; light_view.m21 = y_axis.z; light_view.m22 = z_axis.z; light_view.m23 = 0;

	light_view.m30 = -kh_dot_v3(x_axis, -light_dir);
	light_view.m31 = -kh_dot_v3(y_axis, -light_dir);
	light_view.m32 = -kh_dot_v3(z_axis, -light_dir);
	light_view.m33 = 1.0f;

	f32 scale = 15.0f;
	mat4 light_proj = orthographic_off_center_lh(-scale, scale, -scale, scale, 0.0f, 40.0f);
	mat4 lighttransform = light_view * light_proj;

	GLuint lighttr_buf;
	glCreateBuffers(1, &lighttr_buf);
	glNamedBufferStorage(lighttr_buf, sizeof(mat4), &lighttransform, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHTTRANSFORM, lighttr_buf);
}

KH_INTERN void
ogl_delete_light(OglState *ogl) {
	if(ogl->light_count > 0) {
		kh_assert(ogl->light_count == 1);
		OglLight *light = ogl->lights + 0;

		glDeleteFramebuffers(1, &light->framebuffer);
		glDeleteBuffers(1, &light->uniform_buffer);
		glDeleteBuffers(1, &light->texture_buffer);
		glDeleteBuffers(1, &light->transform_buffer);

		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_SHADOWMAP, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHT, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHTTRANSFORM, 0);

		light->framebuffer = 0;
		light->uniform_buffer = 0;
		light->texture_buffer = 0;
		light->transform_buffer = 0;

		ogl->light_count = 0;
		
	}
}

KH_INTERN void
ogl_DEBUG_draw_texture(OglState *ogl, OglTexture2D *texture, u32 off_x, u32 off_y, u32 w, u32 h) {
	glDisable(GL_DEPTH_TEST);
	glViewport(off_x, off_y, w, h);
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, sizeof(OglBindlessTextureAddress), 0, GL_STREAM_DRAW);
	if(ogl->bindless) {
		OglBindlessTextureAddress texaddr = {texture->bindless, (f32)texture->slice, 0};
		glNamedBufferSubData(buffer, 0, sizeof(OglBindlessTextureAddress), &texaddr);
	} else {
		OglTextureAddress texaddr = {texture->container_id, (f32)texture->slice, 0, 0};
		glNamedBufferSubData(buffer, 0, sizeof(OglTextureAddress), &texaddr);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, buffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDeleteBuffers(1, &buffer);
	glEnable(GL_DEPTH_TEST);
}

KH_INTERN void
ogl_DEBUG_draw_axis(GLuint prog_name, v3 pos, v3 x_axis, v3 y_axis, v3 z_axis, float width) {
	glUseProgram(prog_name);
	GLfloat line_verts[] = {
		pos.x, 	  pos.y,    pos.z,		1.0f, 0.0f, 0.0f,
		x_axis.x, x_axis.y, x_axis.z,	1.0f, 0.0f, 0.0f,

		pos.x, 	  pos.y,    pos.z,		0.0f, 1.0f, 0.0f,
		y_axis.x, y_axis.y, y_axis.z,	0.0f, 1.0f, 0.0f,

		pos.x, 	  pos.y,    pos.z,		0.0f, 0.0f, 1.0f,
		z_axis.x, z_axis.y, z_axis.z,	0.0f, 0.0f, 1.0f,	
	};
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, sizeof(line_verts), line_verts, GL_STREAM_DRAW);
	glBindVertexBuffer(ATTRIB_VERTEX_DATA, buffer, 0, 6 * sizeof(GLfloat));
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(width);
	glDrawArrays(GL_LINES, 0, 6);
	glDeleteBuffers(1, &buffer);

}

KH_INLINE void
ogl_display_buffer(OglState *ogl, u32 w, u32 h) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
    u32 new_w = (u32)((f32)h * (16.0f / 9.0f));
    u32 pad_w = (w - new_w) / 2;
    u32 pad_h = 0;
    u32 new_h = h;
    if(new_w > w) {
    	new_w = w;
    	pad_w = 0;
    	new_h = (u32)((f32)w * (9.0f / 16.0f));
    	kh_assert(new_h <= h);
    	pad_h = (h - new_h) / 2;
    }
	glViewport(pad_w, pad_h, new_w, new_h);

	OglPass *blit_pass = ogl->ogl_pass + Pass_blit;
	glUseProgram(ogl->materials[Material_rendertarget].prog_name);
	glBindVertexBuffer(ATTRIB_VERTEX_DATA, blit_pass->vertexbuffer, 0, 3 * sizeof(GLfloat));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, blit_pass->matbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// ogl_DEBUG_draw_texture(ogl, &ogl->shadow_map, 0, 0, 200, 200);

}

#define MAX_VERTICES_COUNT megabytes(3)
#define MAX_TRIANGLES_COUNT megabytes(1)

KH_INTERN void
create_vertex_buffer(OglState *ogl, VertexFormat format) {

	OglVertexBuffer *vert_buffer = ogl->vertex_buffers + format;
	u32 size = get_size_from_vertex_format(format);
	b32 skinned = false;
	u32 anim_offset = 0;
	if(has_skin(format)) {
		skinned = true;
		i32 offset = get_skinned_attribute_offset(format).offset;
		kh_assert(offset != -1);
		anim_offset = (u32)offset;
	}
	ogl_init_vertex_buffer(vert_buffer, size, MAX_VERTICES_COUNT, MAX_TRIANGLES_COUNT, skinned, anim_offset);
}

KH_INTERN void
ogl_start(OglState *ogl, RenderManager *render, Assets *assets) {

	OglExtensionsHash *hash = &ogl->exts;
	ogl_set_extensions_hash(hash);
	check_required_ogl_extensions(hash);

	u32 flags = GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT;
	u32 create_flags = flags | GL_DYNAMIC_STORAGE_BIT;

	ogl->bindless = ogl_check_ext(hash, "GL_ARB_bindless_texture");
	ogl->sparse = ogl_check_ext(hash, "GL_ARB_sparse_texture");
	// ogl->sparse = false;
	// bindless = false;
	ogl->zprepass_enabled = false;

	DataCache *cache = &assets->cache;

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenGLDebugCallback, 0);

	// NOTE(flo): INIT MATERIALS
	{
		StackAllocator shader_stack = {};
		TransientStack tmp_shader = kh_begin_transient(&shader_stack);

		create_vert_frag_prog(ogl, Material_rendertarget, &shader_stack, "shaders/fbo_texture.vert", 
		                      "shaders/fbo_texture.frag", VertexFormat_PosNormalUV);
		create_vert_frag_prog(ogl, Material_phong, &shader_stack, "shaders/phongbasis.vert", 
		                      "shaders/phongbasis.frag", VertexFormat_PosNormalUV);
		create_vert_frag_prog(ogl, Material_normalmap, &shader_stack, "shaders/normalmapping.vert", 
		                      "shaders/normalmapping.frag", VertexFormat_PosNormalTangentBitangentUV);
		create_vert_frag_prog(ogl, Material_normalmapskinned, &shader_stack, "shaders/normalmapping_skinned.vert", 
		                      "shaders/normalmapping_skinned.frag", VertexFormat_PosNormalTangentBitangentUVSkinned);
		create_vert_frag_prog(ogl, Material_shadowmap, &shader_stack, "shaders/shadowmap.vert", 
		                      "shaders/shadowmap.frag", VertexFormat_PosNormalUV);
		create_vert_frag_prog(ogl, Material_skybox, &shader_stack, "shaders/skybox.vert", 
		                      "shaders/skybox.frag", VertexFormat_PosNormalUV);
		create_vert_frag_prog(ogl, Material_notexture, &shader_stack, "shaders/notexture.vert", 
		                      "shaders/notexture.frag", VertexFormat_PosNormalUV);
		create_vert_frag_prog(ogl, Material_zprepass, &shader_stack, "shaders/zprepass.vert", 
		                     "shaders/zprepass.frag", VertexFormat_PosNormalUV);
		kh_end_transient(&tmp_shader);
		kh_clear(&shader_stack);
	}

	// NOTE(flo): INIT MESHES AND VBOS
	{
		create_vertex_buffer(ogl, VertexFormat_PosNormalUV);
		create_vertex_buffer(ogl, VertexFormat_PosNormalTangentBitangentUV);
		create_vertex_buffer(ogl, VertexFormat_PosNormalTangentBitangentUVSkinned);
	}

	// TODO(flo): hash for this ? we need to create on fly
	create_texture_2D_container(ogl, 2, 1, GL_RGB8, 4096, 4096);
	create_texture_2D_container(ogl, 2, 1, GL_RGB8, 512, 512);
	create_texture_2D_container(ogl, 2, 1, GL_RGB8, 1024, 1024);
	create_texture_2D_container(ogl, 1, 1, GL_DEPTH_COMPONENT24, 4096, 4096);

	// NOTE(flo): INIT TEXTURES
	{
		// @TODO(flo): pack in TEXTURE_2D_ARRAY all the textures that got the same 
		//  #levels, internalformat, width and height (we'll need a hash for this)
		// and keep the depth (zoffset and the handle)
		// @TODO(flo): allocate directly to vbo with bufferstorage and mapbuffer so we do not need texture->memory 
		// and mesh->memory, the entry will just need a buffer id and a buffer offset
		// TODO(flo): each material should have its own buffer that we can bind with the bindbufferbase
		// since we do not want to pass too much textures if we do not need them
		glCreateBuffers(1, &ogl->ogl_pass[Pass_render_3d_scene].matbuffer);
		if(ogl->bindless) {
			glNamedBufferStorage(ogl->ogl_pass[Pass_render_3d_scene].matbuffer, 
			                     sizeof(OglBindlessTextureAddress) * 64, 0, GL_DYNAMIC_STORAGE_BIT);
		} else {
			glNamedBufferStorage(ogl->ogl_pass[Pass_render_3d_scene].matbuffer, 
			                     sizeof(OglTextureAddress) * 64, 0, GL_DYNAMIC_STORAGE_BIT);
		}
	}

	// NOTE(flo): INIT ANIMATIONS
	{
		glCreateBuffers(1, &ogl->bone_transform);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_BONETRANSFORM, ogl->bone_transform);

		GLuint bone_offset;
		glCreateBuffers(1, &bone_offset);
		glNamedBufferStorage(bone_offset, sizeof(u32) * MAX_ENTRIES, NULL, flags);
		ogl->map_boneoffset = (u32 *)glMapNamedBufferRange(bone_offset, 0, sizeof(u32) * MAX_ENTRIES, flags);

		// glNamedBufferData(bone_offset, sizeof(u32) * MAX_ENTRIES, 0, GL_DYNAMIC_DRAW);
		// ogl->bone_offsets = bone_offset;

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_BONEOFFSET, bone_offset);
	}

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// NOTE(flo): BLIT PASS
	OglTexture2DContainer *container_fb = create_texture_2D_container(ogl, 1, 1, GL_RGBA8, render->width, render->height);
	ogl_add_texture_to_container(container_fb, &ogl->target);
	{
		GLfloat fb_pos[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
		};
		GLuint vertices;
		glCreateBuffers(1, &vertices);
		glNamedBufferData(vertices, sizeof(fb_pos), fb_pos, GL_STATIC_DRAW);
		ogl->ogl_pass[Pass_blit].vertexbuffer = vertices;
		ogl->ogl_pass[Pass_blit].framebuffer = 0;
	}

	// v3 light_dir = kh_vec3(0.0f, -0.5f, 0.86602540378f);
	// f32 length_sqr = kh_dot_v3(light_dir, light_dir);

	// @NOTE(flo): 3D SCENE PASS
	{
		glCreateFramebuffers(1, &ogl->ogl_pass[Pass_render_3d_scene].framebuffer);
		GLuint rb;
		glCreateRenderbuffers(1, &rb);
		glNamedRenderbufferStorage(rb, GL_DEPTH_COMPONENT, render->width, render->height);
		glNamedFramebufferRenderbuffer(ogl->ogl_pass[Pass_render_3d_scene].framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);

		glNamedFramebufferTextureLayer(ogl->ogl_pass[Pass_render_3d_scene].framebuffer, GL_COLOR_ATTACHMENT0, ogl->target.name, 0, ogl->target.slice);
		// NOTE(flo): we specify that we want to draw the COLOR_ATTACHMENT0 of our framebuffer, it seems not to be
		// necessary
		// GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
		// glDrawBuffers(1, bufs);
		ogl_get_fb_status(ogl->ogl_pass[Pass_render_3d_scene].framebuffer, GL_FRAMEBUFFER);

		glCreateBuffers(1, &ogl->ogl_pass[Pass_blit].matbuffer);
		if(ogl->bindless) {
			OglBindlessTextureAddress addr = {ogl->target.bindless, (f32)ogl->target.slice, 0};
			glNamedBufferData(ogl->ogl_pass[Pass_blit].matbuffer, sizeof(OglBindlessTextureAddress), &addr, GL_STATIC_DRAW);
		}
		else {
			OglTextureAddress addr = {ogl->target.container_id, (f32)ogl->target.slice, 0, 0};
			glNamedBufferData(ogl->ogl_pass[Pass_blit].matbuffer, sizeof(OglTextureAddress), &addr, GL_STATIC_DRAW);
		}

		umm pos_offset    = 0;
		umm norm_offset   = pos_offset + 3 * sizeof(GLfloat);
		umm uv_offset     = norm_offset + 3 * sizeof(GLfloat);
		umm tan_offset    = uv_offset + 2 *sizeof(GLfloat);
		umm bi_offset     = tan_offset + 3 * sizeof(GLfloat);
		// umm id_offset     = bi_offset + 3 * sizeof(GLfloat);
		umm id_offset     = 0;
		umm weight_offset = id_offset + 4 * sizeof(GLint);

		// TODO(flo): for now we assume that for every vertex format all locations are in the same place
		// only the stride can change. Do we need to discard this assumption ?
		glVertexAttribFormat(LOC_POSITION, 3, GL_FLOAT, GL_FALSE, pos_offset);
		glVertexAttribFormat(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, norm_offset);
		glVertexAttribFormat(LOC_UV0, 2, GL_FLOAT, GL_FALSE, uv_offset);
		glVertexAttribFormat(LOC_TANGENT, 3, GL_FLOAT, GL_FALSE, tan_offset);
		glVertexAttribFormat(LOC_BITANGENT, 3, GL_FLOAT, GL_FALSE, bi_offset);

		glVertexAttribIFormat(LOC_BONE_IDS, 4, GL_INT, id_offset);
		glVertexAttribFormat(LOC_BONE_WEIGHTS, 4, GL_FLOAT, GL_FALSE, weight_offset);

		glVertexAttribBinding(LOC_POSITION, ATTRIB_VERTEX_DATA);
		glVertexAttribBinding(LOC_NORMAL, ATTRIB_VERTEX_DATA);
		glVertexAttribBinding(LOC_UV0, ATTRIB_VERTEX_DATA);
		glVertexAttribBinding(LOC_TANGENT, ATTRIB_VERTEX_DATA);
		glVertexAttribBinding(LOC_BITANGENT, ATTRIB_VERTEX_DATA);

		glVertexAttribBinding(LOC_BONE_IDS, ATTRIB_ANIMATION);
		glVertexAttribBinding(LOC_BONE_WEIGHTS, ATTRIB_ANIMATION);

		glEnableVertexAttribArray(LOC_POSITION);
		glEnableVertexAttribArray(LOC_NORMAL);
		glEnableVertexAttribArray(LOC_UV0);
		glEnableVertexAttribArray(LOC_TANGENT);
		glEnableVertexAttribArray(LOC_BITANGENT);
		glEnableVertexAttribArray(LOC_BONE_IDS);
		glEnableVertexAttribArray(LOC_BONE_WEIGHTS);

		GLuint transform;
		glCreateBuffers(1, &transform);
		glNamedBufferStorage(transform, sizeof(OglTransform) * MAX_ENTRIES, NULL, create_flags);
		ogl->map_transforms = (OglTransform *)glMapNamedBufferRange(transform, 0, sizeof(OglTransform) * MAX_ENTRIES, flags);

		GLuint idbo;
		glCreateBuffers(1, &idbo);
		glNamedBufferStorage(idbo, sizeof(u32) * MAX_ENTRIES, 0, create_flags);
		ogl->map_drawids = (u32 *)glMapNamedBufferRange(idbo, 0, sizeof(u32) * MAX_ENTRIES, flags);
		glVertexAttribIFormat(LOC_DRAWID, 1, GL_UNSIGNED_INT, 0);
		glVertexAttribBinding(LOC_DRAWID, ATTRIB_DRAWID);
		glVertexBindingDivisor(ATTRIB_DRAWID, 1);
		glEnableVertexAttribArray(LOC_DRAWID);
		glBindVertexBuffer(ATTRIB_DRAWID, idbo, 0, sizeof(GL_UNSIGNED_INT));

		GLuint cmd_id;
		glCreateBuffers(1, &cmd_id);
		glNamedBufferStorage(cmd_id, sizeof(u32) * MAX_ENTRIES, 0, create_flags);
		ogl->map_drawcmdsids = (u32 *)glMapNamedBufferRange(cmd_id, 0, sizeof(u32) * MAX_ENTRIES, flags);
		glVertexAttribIFormat(LOC_INDIRECTCMDID, 1, GL_UNSIGNED_INT, 0);
		glVertexAttribBinding(LOC_INDIRECTCMDID, ATTRIB_DRAWCMDID);
		glVertexBindingDivisor(ATTRIB_DRAWCMDID, 1);
		glEnableVertexAttribArray(LOC_INDIRECTCMDID);
		glBindVertexBuffer(ATTRIB_DRAWCMDID, cmd_id, 0, sizeof(GL_UNSIGNED_INT));

		glCreateBuffers(1, &ogl->color_buffer);
		glNamedBufferStorage(ogl->color_buffer, sizeof(v4) * MAX_COLORS, 0, GL_DYNAMIC_STORAGE_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_TRANSFORM, transform);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_COLORS, ogl->color_buffer);

		GLuint cam_tr;
		glCreateBuffers(1, &cam_tr);
		glNamedBufferStorage(cam_tr, sizeof(OglCameraTransform), 0, create_flags);
		ogl->map_cam_transform = (OglCameraTransform *)glMapNamedBufferRange(cam_tr, 0, sizeof(OglCameraTransform), flags);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_CAMERATRANSFORM, cam_tr);

		glCreateBuffers(1, &ogl->cmds_buffer);
		glNamedBufferData(ogl->cmds_buffer, sizeof(DrawElementsIndirectCommand) * MAX_DRAWCMD_PER_FRAME, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ogl->cmds_buffer);
	}

	if(!ogl->bindless) {
		glBindTextures(BIND_TEXTURES, ogl->texture_container_count, ogl->texture_container_names);
	}
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

}
#undef ogl_prog

KH_INLINE void
ogl_render_command(OglMaterial *mat) {
	u64 indirect = mat->cmd_offset * sizeof(DrawElementsIndirectCommand);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)indirect, mat->cmd_count, 0);
}

KH_INLINE void
ogl_render_material(OglMaterial *mat) {
	glUseProgram(mat->prog_name);
	ogl_render_command(mat);
}


// TODO(flo): remove allocation
KH_INTERN void
ogl_update(OglState *ogl, RenderManager *render, Assets *assets) {

	DataCache *cache = &assets->cache;
	#define NUM_TEXTURE 2
	u32 batch_count = render->batch_count;
	u32 cmd_count = 0;
	u32 base_instance = 0;
	u32 total_entry_count = 0;
	ogl->used_material_count = 0;


	if(render->batch_count > 0 && render->entry_count > 0) {

		glNamedBufferData(ogl->bone_transform, render->bone_tr.count * sizeof(mat4), render->bone_tr.data, GL_DYNAMIC_DRAW);

		u8 *tex_addresses = 0;
		v4 *colors = (v4 *)malloc(sizeof(v4) * batch_count);
		if(ogl->bindless) {
			u32 size = sizeof(OglBindlessTextureAddress) * batch_count * NUM_TEXTURE;
			tex_addresses = (u8 *)malloc(size);
			kh_assert((size % 16 == 0));

		} else {
			u32 size = sizeof(OglTextureAddress) * batch_count * NUM_TEXTURE;
			tex_addresses = (u8 *)malloc(size);
			kh_assert((size % 16 == 0));
		}

		for(VertexBuffer *buffer = render->first_vertex_buffer; buffer; buffer = buffer->next) {
			VertexFormat fmt = buffer->format;
			for(Material *mat = buffer->first; mat; mat = mat->next) {
				MaterialType type = mat->type;
				OglMaterial *oglmat = ogl->materials + type;
				oglmat->cmd_count = 0;
				oglmat->render_count = 0;
				kh_assert(oglmat->format == fmt);
				// oglmat->format = fmt;
				oglmat->cmd_offset = cmd_count;
				ogl->used_materials[ogl->used_material_count++] = (u32)type;

				for(MaterialInstance *instance = mat->first; instance; instance = instance->next) {

					OglTexture2D *diffuse = 0;
					OglTexture2D *normal = 0;

					if(instance->diffuse.val && get_asset_state(assets, instance->diffuse) == AssetState_loaded) {
						diffuse = get_ogl_texture_2d(ogl, render, assets, instance->diffuse);
					}
					if(instance->normal.val && get_asset_state(assets, instance->normal) == AssetState_loaded) {
						normal = get_ogl_texture_2d(ogl, render, assets, instance->normal);
					}

					for(MeshRenderer *meshr = instance->first; meshr; meshr = meshr->next) {
						u32 entry_count = meshr->entry_count;
						if(entry_count > 0) {	
							u32 cmd_ind = cmd_count++;
							kh_assert(cmd_count < MAX_DRAWCMD_PER_FRAME);


							oglmat->cmd_count++;
							oglmat->render_count += entry_count;

							if(ogl->bindless) {
								OglBindlessTextureAddress *texaddr = (OglBindlessTextureAddress *)tex_addresses + cmd_ind * NUM_TEXTURE;
								if(diffuse) {
									texaddr[0] = {diffuse->bindless, (f32)diffuse->slice, 0};
								} else {
									texaddr[0] = {};
								}
								if(normal) {
									texaddr[1] = {normal->bindless, (f32)normal->slice, 0};
								} else {
									texaddr[1] = {};
								}
							} else {
								OglTextureAddress *texaddr = (OglTextureAddress *)tex_addresses + cmd_ind * NUM_TEXTURE;
								if(diffuse) {
									texaddr[0] = {diffuse->container_id, (f32)diffuse->slice, 0, 0};
								} else {
									texaddr[0] = {};
								}
								if(normal) {
									texaddr[1] = {normal->container_id, (f32)normal->slice, 0, 0};
								} else {
									texaddr[1] = {};
								}
							}
							colors[cmd_ind] = instance->color;

							OglTriangleMesh *mesh = get_ogl_triangle_mesh(ogl, render, assets, meshr->mesh, fmt);

							DrawElementsIndirectCommand *cmd = ogl->cmds + cmd_ind;
							cmd->primCount = entry_count;
							cmd->count = mesh->ind_count;
							cmd->firstIndex = mesh->ibo_offset;
							cmd->baseVertex = mesh->vbo_offset; 
							cmd->baseInstance = base_instance;
							base_instance += cmd->primCount;

							u32 entry_ind = meshr->first_entry;
							for(u32 i = 0; i < entry_count; ++i) {
								RenderEntry *entry = render->render_entries + entry_ind;

								u32 *bone_offset = ogl->map_boneoffset + total_entry_count;
								ogl->map_boneoffset[total_entry_count] = 0;

								if(entry->bone_transform_offset != INVALID_U32_OFFSET) {
									ogl->map_boneoffset[total_entry_count] = entry->bone_transform_offset + 1;	
								}

								OglTransform *map_tr = ogl->map_transforms + total_entry_count;
								map_tr->model = entry->tr;
								ogl->map_drawids[total_entry_count] = total_entry_count;
								ogl->map_drawcmdsids[total_entry_count] = cmd_ind;

								total_entry_count++;
								entry_ind = entry->next_in_mesh_renderer;

							}
						}
					}
				}
			}
		}
		kh_assert(total_entry_count < MAX_ENTRIES);
		kh_assert(render->entry_count == total_entry_count);
		if(ogl->bindless) {
			glNamedBufferSubData(ogl->ogl_pass[Pass_render_3d_scene].matbuffer, 
			                     0, sizeof(OglBindlessTextureAddress) * batch_count * NUM_TEXTURE, tex_addresses);
		} else {
			glNamedBufferSubData(ogl->ogl_pass[Pass_render_3d_scene].matbuffer,
			                     0, sizeof(OglTextureAddress) * batch_count * NUM_TEXTURE, tex_addresses);
		}
		glNamedBufferSubData(ogl->color_buffer, 0, sizeof(v4) * batch_count, colors);
		glNamedBufferSubData(ogl->cmds_buffer, 0, sizeof(DrawElementsIndirectCommand) * cmd_count, ogl->cmds);

		free(tex_addresses);
		free(colors);
	}
}


KH_INTERN void
ogl_render(OglState *ogl, RenderManager *render, Assets *assets) {

	ogl_update(ogl, render, assets);

	Camera *cam = &render->camera;
	mat4 vp = cam->view * cam->projection;

	mat4 view = cam->view;
	view.m30 = 0;
	view.m31 = 0;
	view.m32 = 0;
	view.m33 = 1;

	OglCameraTransform *cam_tr = ogl->map_cam_transform;
	// @TODO(flo): refactoring our camera stuff to avoid debugging garbage datas for some hours~!
	cam_tr->viewproj = vp;
	cam_tr->view = view * cam->projection;
	cam_tr->pos = kh_vec4(cam->tr.pos, 1.0f);

	u32 vertex_format = 0xFFFFFFFF;

	// TODO(flo): do not like this
	if(!ogl->has_skybox && render->has_skybox) {
		ogl_set_skybox(ogl, render, assets);
		ogl->has_skybox = true;
	}

	if(ogl->light_count < render->light_count) {
		u32 light_index = ogl->light_count++;
		DirectionalLight *light = render->lights + light_index;
		ogl_set_light(ogl, light, light_index);
	}
	//glDepthFunc
	//glDepthMask


	// TODO(flo): handle animations in the shadow pass!
	if(render->entry_count > 0) {
		glEnable(GL_DEPTH_TEST);
		if(ogl->light_count > 0) {
			OglPass *shadow_pass = &ogl->ogl_pass[Pass_shadow];
			glBindFramebuffer(GL_FRAMEBUFFER, shadow_pass->framebuffer);
			glViewport(0, 0, 4096, 4096);
			glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(ogl->materials[Material_shadowmap].prog_name);
			// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, 0);
			// OglVertexBuffer *vert_buffer = g_state->vertex_buffers + 0;
			// glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);	
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);
			for(u32 i = 0; i < ogl->used_material_count; ++i) {
				OglMaterial *oglmat = ogl->materials + ogl->used_materials[i];
				if(oglmat->render_count > 0) {
					if(vertex_format != (u32)oglmat->format) {
						vertex_format = (u32)oglmat->format;
						OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
						if(vert_buffer->skinned) {
							glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, vert_buffer->anim_offset,
							                   vert_buffer->attrib_stride);
						}
						glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);	
					}
					ogl_render_command(oglmat);
				}
			}
		}

		if(ogl->zprepass_enabled) {
			glUseProgram(ogl->materials[Material_zprepass].prog_name);
			for(u32 i = 0; i < ogl->used_material_count; ++i) {
				OglMaterial *oglmat = ogl->materials + ogl->used_materials[i];
				if(oglmat->render_count > 0) {
					if(vertex_format != (u32)oglmat->format) {
						vertex_format = (u32)oglmat->format;
						OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
						glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);	
					}
					ogl_render_command(oglmat);
				}
			}
			glDepthFunc(GL_LEQUAL);
		}
	}
	OglPass *scene_pass = &ogl->ogl_pass[Pass_render_3d_scene];
	glBindFramebuffer(GL_FRAMEBUFFER, scene_pass->framebuffer);
	glViewport(0,0,render->width,render->height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// for(u32 i = 0; i < render->entry_count; ++i) {
	// 	RenderEntry *entry = render->render_entries + i;
	// 	if(entry->bone_transform_offset != INVALID_U32_OFFSET) {
	// 		mat4 *first = render->bone_tr.data + entry->bone_transform_offset;
	// 		for(u32 j = 0; j < 97; ++j) {
	// 			mat4 local_tr = first[j];
	// 			mat4 tr = local_tr * entry->tr;
	// 			v3 pos = kh_vec3(tr.c3);
	// 			ogl_DEBUG_draw_axis(ogl->materials[Material_notexture].prog_name, pos, pos + kh_vec3(tr.c0), pos + kh_vec3(tr.c1), pos + kh_vec3(tr.c2), 1.0f);


	// 		}

	// 	}

	// }

	if(render->entry_count > 0) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, scene_pass->matbuffer);
		// glDrawBuffer(GL_COLOR_ATTACHMENT1);
		vertex_format = 0xFFFFFFFF;
		for(u32 i = 0; i < ogl->used_material_count; ++i) {
			OglMaterial *oglmat = ogl->materials + ogl->used_materials[i];
			if(oglmat->render_count > 0) {
				if(vertex_format != (u32)oglmat->format) {
					vertex_format = (u32)oglmat->format;
					OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
					if(vert_buffer->skinned) {
						glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, vert_buffer->anim_offset, vert_buffer->attrib_stride);
					}
					glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);
				}
				ogl_render_material(oglmat);
			}
		}	
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}


	if(ogl->has_skybox) {
		glDepthFunc(GL_LEQUAL);
		OglPass *skybox_pass = &ogl->ogl_pass[Pass_skybox];
		glUseProgram(ogl->materials[Material_skybox].prog_name);
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, skybox_pass->vertexbuffer, 0, 3 * sizeof(GLfloat));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);
	}
}

// TODO(flo): OGL exit