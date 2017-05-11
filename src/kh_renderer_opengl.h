#ifndef KHJR_OPENGL_RENDER_H

enum PassType {
	Pass_shadow,
	Pass_render_3d_scene,
	Pass_skybox,
	Pass_blit,
	Pass_count,
};

GL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
    if(severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH) {
    	GLenum err = glGetError();	
    	char *err_mess = (char *)message;
    	kh_assert(!"OpenGL Error encountered");
    }

}

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
	GLint reserved;
};

struct OglTextureAddress {
	GLuint container;
	GLfloat page;
	GLint reserved[2];
};

struct OglTexture2D {
	GLuint64 bindless;
	GLuint name;
	GLuint container_id;
	u32 slice;
};

struct OglTexture2DContainer {
	u32 index;
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
	v4 pos;
};

struct OglTransform {
	mat4 model;
};

struct OglPass {
	GLuint framebuffer;
	GLuint vertexbuffer;
	GLuint matbuffer;
};

struct OglMaterial {
	u32 prog_name;
	u32 render_count;
	u32 cmd_count;
	u32 cmd_offset;
	VertexFormat format;
};

struct VerticesBufferData {
	GLuint name;
	u32 total_size;
	u32 offset;
};

struct IndicesBufferData {
	GLuint name;
	u32 total_size;
	u32 offset;
};

struct OglVertexBuffer {

	u32 mesh_count;
	u32 ibo_at;
	u32 vbo_at;

	VerticesBufferData verts;
	IndicesBufferData inds;

	u32 attrib_offset;
	u32 attrib_stride;

	// @TODO(flo) : we'll need this eventually?
	// u32 ibo_max;
	// u32 vbo_max;
};

struct OglTriangleMesh {
	u32 ind_count;
	u32 vbo_offset;
	u32 ibo_offset;
	u32 draw_id;
};

#define MAX_HASH_GL_EXTENSIONS 256
struct OglExtension {
	char *name;
	OglExtension *next_in_hash;
};
struct OglExtensionsHash {
	StackAllocator buffer;
	OglExtension *hash[MAX_HASH_GL_EXTENSIONS];

	u32 empty;
	u32 collision_count;
	u32 count;

};

struct OglSkybox {
	GLuint vertices;
	GLuint texture;
};

struct OglLight {
	GLuint uniform_buffer;
	GLuint framebuffer;
	GLuint texture_buffer;
	GLuint transform_buffer;
};

#define MAX_TEXTURES 64
#define MAX_MESHES 64
#define MAX_TEXTURE_BINDINGS 16
#define MAX_DRAWCMD_PER_FRAME 8
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
};

KH_INTERN void
WIN32DEBUG_opengl_get_shader_log(GLuint shader) {
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		GLchar info_log[4096];
		glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
	}
}

KH_INTERN void
WIN32DEBUG_opengl_get_prog_log(GLuint prog) {
	GLint success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success) {
		GLchar info_log[4096];
		glGetProgramInfoLog(prog, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
	}
}

KH_INTERN void
ogl_assert() {
	GLenum er = glGetError();
	kh_assert(!er);
}

KH_INTERN GLuint
ogl_compile_shader(GLenum type, GLsizei count, char *src, GLint *length) {
	GLuint res = glCreateShader(type);
	glShaderSource(res, count, &src, length);
	glCompileShader(res);
	return(res);
}

KH_INTERN void
ogl_add_ext_to_hash(OglExtensionsHash *hash, char *name) {

	u32 hash_val = hash_key_from_djb2(name);
	u32 hash_slot = hash_val & (MAX_HASH_GL_EXTENSIONS - 1);
	kh_assert(hash_slot < MAX_HASH_GL_EXTENSIONS);

	OglExtension **first = hash->hash + hash_slot;

	hash->count++;
	if(*first) {
		kh_assert(hash->collision_count < array_count(hash->hash));
		hash->collision_count++;
	} else {
		kh_assert(hash->empty >= 0 && hash->empty < array_count(hash->hash));
		hash->empty--;
	}

	OglExtension *app = kh_push_struct(&hash->buffer, OglExtension);

	u32 str_len = string_length(name);
	app->name = (char *)kh_push(&hash->buffer, str_len + 1);
	strings_copy(str_len, name, app->name);
	app->name[str_len] = '\0';
	app->next_in_hash = *first;
	*first = app;
}

KH_INTERN b32
ogl_check_ext(OglExtensionsHash *hash, char *name) {

	u32 hash_val = hash_key_from_djb2(name);
	u32 hash_slot = hash_val & (MAX_HASH_GL_EXTENSIONS - 1);
	kh_assert(hash_slot < MAX_HASH_GL_EXTENSIONS);

	OglExtension **first = hash->hash + hash_slot;
	OglExtension *find = 0;

	// u32 str_len = string_length(name);
	for(OglExtension *search = *first; search; search = search->next_in_hash) {
		if(strings_equals(search->name, name)) {
			find = search;
			break;
		}
	}
	b32 res = (find != 0);
	return(res);
}


KH_INTERN void
ogl_set_extensions_hash(OglExtensionsHash *hash) {
	GLint num_ext;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	hash->empty = array_count(hash->hash) - 1;
	hash->collision_count = 0;

	for(GLint i = 0; i < num_ext; ++i) {
		char *ext = (char *)glGetStringi(GL_EXTENSIONS, i);
		ogl_add_ext_to_hash(hash, ext);
	}
}

KH_INTERN void
ogl_check_ext_list(OglExtensionsHash *hash, char **name, u32 count) {
	KH_FLU(count) {
		char *ext = *(name + i);
		kh_assert(ogl_check_ext(hash, ext));
	}
}

KH_INTERN void
ogl_get_fb_status(GLuint fb, GLenum type) {
	GLenum fb_status = glCheckNamedFramebufferStatus(fb, type);
	switch(fb_status) {
		case GL_FRAMEBUFFER_UNDEFINED :	{kh_assert(!"undefined");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT  : {kh_assert(!"incomplete attachment");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT  : {kh_assert(!"incomplete missing attachment");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER  : {kh_assert(!"incomplete draw buffer");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER  : {kh_assert(!"incomplete read buffer");} break;
		case GL_FRAMEBUFFER_UNSUPPORTED  : {kh_assert(!"unsupported");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE  : {kh_assert(!"incomplete multi sample");} break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS   : {kh_assert(!"incomplete layer target");} break;
	}
}

KH_INTERN GLchar *
ogl_load_shader_from_file(char *header, u32 header_size, char *filename, StackAllocator *memstack) {

	FileHandle shader = g_platform.open_file(filename, memstack, FileAccess_read, FileCreation_only_open);
	u32 size = g_platform.get_file_size(&shader);

	u32 total_size = header_size + size + 1;

	GLchar *res = (GLchar *)kh_push(memstack, total_size);

	strings_copy_NNT(header_size, header, (char *)res);

	u8 *dst = (u8 *)res + header_size; 

	g_platform.read_bytes_of_file(&shader, 0, size, dst);
	res[total_size] = '\0';
	g_platform.close_file(&shader);
	return(res);
}

KH_INTERN VerticesBufferData
ogl_vertices_buffer_data(u32 size) {
	VerticesBufferData res;
	glCreateBuffers(1, &res.name);
	res.total_size = size;
	res.offset = 0;
	glNamedBufferData(res.name, res.total_size, 0, GL_STATIC_DRAW);
	kh_assert(glGetError() != GL_OUT_OF_MEMORY);
	return(res);
}

KH_INTERN IndicesBufferData
ogl_indices_buffer_data(u32 count) {
	IndicesBufferData res;
	glCreateBuffers(1, &res.name);
	res.total_size = count * 3 * sizeof(u32);
	res.offset = 0;
	glNamedBufferData(res.name, res.total_size, 0, GL_STATIC_DRAW);
	kh_assert(glGetError() != GL_OUT_OF_MEMORY);
	return(res);
}

KH_INTERN void
ogl_init_vertex_buffer(OglVertexBuffer *vert_buffer, u32 vertices_count, u32 vertex_size, u32 tri_count) {
	vert_buffer->mesh_count    = 0;
	vert_buffer->ibo_at        = 0;
	vert_buffer->vbo_at        = 0;
	vert_buffer->verts         = ogl_vertices_buffer_data(vertices_count * vertex_size);
	vert_buffer->inds          = ogl_indices_buffer_data(tri_count);
	vert_buffer->attrib_stride = vertex_size;
	vert_buffer->attrib_offset = 0;
	// TODO(flo): user specified for offset
}

KH_INTERN void
ogl_add_vertices_to_buffer_data(VerticesBufferData *buffer, TriangleMesh *mesh) {
	u32 size = mesh->vert_c * mesh->vertex_size;
	// @TODO(flo): linked list of vertex buffer if we're out of size
	kh_assert(buffer->offset + size <= buffer->total_size);
	glNamedBufferSubData(buffer->name, buffer->offset, size, mesh->memory);
	buffer->offset += size;
}

KH_INTERN void
ogl_add_indices_to_buffer_data(IndicesBufferData *buffer, TriangleMesh *mesh) {
	u32 size = mesh->tri_c * 3 * sizeof(u32);
	kh_assert(buffer->offset + size <= buffer->total_size);
	glNamedBufferSubData(buffer->name, buffer->offset, size, mesh->memory + mesh->indices_offset);
	buffer->offset += size;
}

KH_INTERN void
ogl_add_triangle_mesh_memory_to_vbo(OglTriangleMesh *oglmesh, OglVertexBuffer *buffer, TriangleMesh *mesh) {
	oglmesh->ind_count = mesh->tri_c * 3;
	oglmesh->vbo_offset = buffer->vbo_at;
	buffer->vbo_at += mesh->vert_c;
	oglmesh->ibo_offset = buffer->ibo_at;
	buffer->ibo_at += mesh->tri_c * 3;
	oglmesh->draw_id = buffer->mesh_count++;

	ogl_add_vertices_to_buffer_data(&buffer->verts, mesh);
	ogl_add_indices_to_buffer_data(&buffer->inds, mesh);
}

KH_INTERN void
ogl_create_texture_2D(OglTexture2D *ogltex, Texture2D *tex, b32 bindless, GLenum format) {
	glCreateTextures(GL_TEXTURE_2D, 1, &ogltex->name);
	glTextureParameteri(ogltex->name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(ogltex->name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(ogltex->name, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTextureParameteri(ogltex->name, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTextureStorage2D(ogltex->name, 1, GL_RGB8, tex->width, tex->height);
	glTextureSubImage2D(ogltex->name, 0, 0, 0, tex->width, tex->height, GL_BGR, GL_UNSIGNED_BYTE, tex->memory);
	if(bindless) 	{
		ogltex->bindless = glGetTextureHandleARB(ogltex->name);
	}
}

KH_INTERN OglTexture2DContainer *
ogl_create_texture_2D_container(OglState *ogl, u32 slices_count, u32 mipmaps_count, GLenum internalformat, u32 w, u32 h) {
	kh_assert(ogl->texture_container_count < MAX_TEXTURE_BINDINGS);
	u32 id = ogl->texture_container_count++;
	OglTexture2DContainer *res = ogl->texture_containers + id;

	GLuint name;
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &name);

	ogl->texture_container_names[id] = name;

	res->sparse = ogl->sparse;

	if(ogl->sparse) {
		glTextureParameteri(name, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

		GLint index_count = 0, xsize = 0, ysize = 0, zsize = 0;
		GLint best_index = 0, best_xsize = 0, best_ysize = 0;

		glGetInternalformativ(GL_TEXTURE_2D_ARRAY, internalformat, GL_NUM_VIRTUAL_PAGE_SIZES_ARB, 1, &index_count);
		for(GLint i = 0; i < index_count; ++i) {
			glTextureParameteri(name, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, i);
			glGetInternalformativ(GL_TEXTURE_2D_ARRAY, internalformat, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &xsize);
			glGetInternalformativ(GL_TEXTURE_2D_ARRAY, internalformat, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &ysize);
			glGetInternalformativ(GL_TEXTURE_2D_ARRAY, internalformat, GL_VIRTUAL_PAGE_SIZE_Z_ARB, 1, &zsize);

			if(zsize == 1) {
				if(xsize >= best_xsize && ysize >= best_ysize) {
					best_index = i;
					best_xsize = xsize;
					best_ysize = ysize;
				}
			}
		}
		kh_assert(best_index != -1);
		if(w % xsize == 0 && h % ysize == 0) {
			glTextureParameteri(name, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, best_index);
		} else {
			glTextureParameteri(name, GL_TEXTURE_SPARSE_ARB, GL_FALSE);
			res->sparse = false;
		}
	}

	GLint max_slices;
	if(res->sparse) {
		glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB, &max_slices);
	} else {
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_slices);
	}
	kh_assert(slices_count <= (u32)max_slices);

	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(name, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTextureParameteri(name, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTextureStorage3D(name, mipmaps_count, internalformat, w, h, slices_count);

	res->name = name;
	res->bindless = 0;
	if(ogl->bindless) {
		res->bindless = glGetTextureHandleARB(name);
		glMakeTextureHandleResidentARB(res->bindless);
	}

	res->index = id;
	res->slices_count = 0;
	res->slices_max = slices_count;
	res->mipmaps_count = mipmaps_count;
	res->format = internalformat;
	res->width = w;
	res->height = h;

	return(res);
}

KH_INTERN void
ogl_commit_texture(OglTexture2DContainer *container, GLsizei slice, GLboolean commit) {
	GLuint level = 0;
	glTexturePageCommitmentEXT(container->name, level, 0, 0, slice, container->width, container->height, 1, commit);
}

KH_INTERN void
ogl_commit_texture(OglTexture2D *texture, u32 w, u32 h, GLboolean commit) {
	GLuint level = 0;
	glTexturePageCommitmentEXT(texture->name, level, 0, 0, texture->slice, w, h, 1, commit);
}
KH_INTERN void
ogl_set_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *tex, u32 slice) {
	kh_assert(slice < container->slices_max);
	tex->bindless = container->bindless;
	tex->name = container->name;
	tex->slice = slice;
	tex->container_id = container->index;
	if(container->sparse) {
		ogl_commit_texture(container, tex->slice, GL_TRUE);
	}
}

KH_INTERN void
ogl_add_texture_to_container(OglTexture2DContainer *container, OglTexture2D *tex) {
	ogl_set_texture_2d_to_container(container, tex, container->slices_count++);
}

// TODO(flo): remove this, @TEMP
KH_INTERN void
ogl_set_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *ogltex, Texture2D *tex, GLenum format, u32 slice) {
	kh_assert(slice < container->slices_max);
	kh_assert(tex->width == container->width);
	kh_assert(tex->height == container->height);

	ogltex->bindless = container->bindless;
	ogltex->name = container->name;
	ogltex->slice = slice;
	ogltex->container_id = container->index;
	// TODO(flo): specify levels, xoffset and yoffset in our Texture2D
	if(container->sparse) {
		ogl_commit_texture(container, ogltex->slice, GL_TRUE);
	}
#if 0
	// TODO(flo): with PBO we do not have to copy our texture memory twice (file->gpu instead of file->cpu->gpu)
	u32 offset = 4096;
	u32 flags = GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT;
	u32 create_flags = flags | GL_DYNAMIC_STORAGE_BIT;
	u32 size = tex->data.height * tex->data.width * tex->data.bytes_per_pixel;
	GLuint mem_b;
	glCreateBuffers(1, &mem_b);
	glNamedBufferStorage(mem_b, offset + size, 0, create_flags);
	u8 *memory = (u8 *)glMapNamedBufferRange(mem_b, offset, size, flags);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mem_b);
	glTextureSubImage3D(container->name, 0, 0, 0, ogltex.slice, tex->data.width, tex->data.height, 1, format, GL_UNSIGNED_BYTE, (void *)(umm)offset);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#else
	glTextureSubImage3D(container->name, 0, 0, 0, ogltex->slice, tex->width, tex->height, 1, format, GL_UNSIGNED_BYTE, tex->memory);
#endif

}

KH_INTERN void
ogl_add_texture_2D_to_container(OglTexture2DContainer *container, OglTexture2D *ogltex, Texture2D *tex, GLenum format) {
	ogl_set_texture_2d_to_container(container, ogltex, tex, format, container->slices_count++);
}

#define KHJR_OPENGL_RENDER_H
#endif