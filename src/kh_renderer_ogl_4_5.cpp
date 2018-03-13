GL_DEBUG_CALLBACK(ogl45_debug_callback) {
    if(severity == GL_DEBUG_SEVERITY_HIGH) {
    	GLenum err = glGetError();	
    	char *err_mess = (char *)message;
    	kh_assert(!"OpenGL Error encountered");
    } else if(severity == GL_DEBUG_SEVERITY_MEDIUM){
    	GLenum err = glGetError();	
    	if(err) kh_assert(!"OpenGL Error encountered");
    	char *err_mess = (char *)message;
    	OutputDebugStringA(message);
    	OutputDebugStringA("\n");
    } else {
    	char *err_mess = (char *)message;
    	OutputDebugStringA(message);
    	OutputDebugStringA("\n");
    }
}

KH_INTERN void
ogl45_check_required_ogl_extensions(OglExtensionsHash *hash) {
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

KH_INTERN void
ogl45_get_fb_status(GLuint fb, GLenum type) {
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
	kh_assert(fb_status == GL_FRAMEBUFFER_COMPLETE);
}

KH_INTERN OglVerticesBufferData
ogl45_vertices_buffer_data(u32 size) {
	OglVerticesBufferData res;
	glCreateBuffers(1, &res.name);
	res.total_size = size;
	res.offset = 0;
	glNamedBufferData(res.name, res.total_size, 0, GL_STATIC_DRAW);
	kh_assert(glGetError() != GL_OUT_OF_MEMORY);
	return(res);
}

KH_INTERN OglIndicesBufferData
ogl45_indices_buffer_data(u32 count) {
	OglIndicesBufferData res;
	glCreateBuffers(1, &res.name);
	res.total_size = count * 3 * sizeof(u32);
	res.offset = 0;
	glNamedBufferData(res.name, res.total_size, 0, GL_STATIC_DRAW);
	kh_assert(glGetError() != GL_OUT_OF_MEMORY);
	return(res);
}

KH_INTERN void
ogl45_init_vertex_buffer(OglVertexBuffer *vert_buffer, u32 vertex_size, u32 vertices_count, u32 tri_count, 
                       b32 skinned, u32 anim_offset) {
	vert_buffer->mesh_count    = 0;
	vert_buffer->ibo_at        = 0;
	vert_buffer->vbo_at        = 0;
	vert_buffer->verts         = ogl45_vertices_buffer_data(vertices_count * vertex_size);
	vert_buffer->inds          = ogl45_indices_buffer_data(tri_count);
	vert_buffer->attrib_stride = vertex_size;
	// TODO(flo): user specified for offset
	vert_buffer->attrib_offset = 0;
	vert_buffer->skinned	   = skinned;
	vert_buffer->anim_offset   = anim_offset;
}

KH_INTERN void
ogl45_add_vertices_to_buffer_data(OglVerticesBufferData *buffer, TriangleMesh *mesh, u8 *data) {
	u32 size = mesh->vert_c * mesh->vertex_size;
	// @TODO(flo): linked list of vertex buffer if we're out of size
	kh_assert(buffer->offset + size <= buffer->total_size);
	glNamedBufferSubData(buffer->name, buffer->offset, size, data);
	buffer->offset += size;
}

KH_INTERN void
ogl45_add_indices_to_buffer_data(OglIndicesBufferData *buffer, TriangleMesh *mesh, u8 *data) {
	u32 size = mesh->tri_c * 3 * sizeof(u32);
	kh_assert(buffer->offset + size <= buffer->total_size);
	glNamedBufferSubData(buffer->name, buffer->offset, size, data + mesh->indices_offset);
	buffer->offset += size;
}

KH_INTERN void
ogl45_add_triangle_mesh_memory_to_vbo(OglTriangleMesh *oglmesh, OglVertexBuffer *buffer, TriangleMesh *mesh, u8 *data) {
	oglmesh->ind_count = mesh->tri_c * 3;
	oglmesh->vbo_offset = buffer->vbo_at;
	buffer->vbo_at += mesh->vert_c;
	oglmesh->ibo_offset = buffer->ibo_at;
	buffer->ibo_at += mesh->tri_c * 3;
	oglmesh->draw_id = buffer->mesh_count++;

	ogl45_add_vertices_to_buffer_data(&buffer->verts, mesh, data);
	ogl45_add_indices_to_buffer_data(&buffer->inds, mesh, data);
}

KH_INTERN void
ogl45_create_texture_2D(OglTexture2D *ogltex, Texture2D *tex, u8 *data, b32 bindless, GLenum format) {
	glCreateTextures(GL_TEXTURE_2D, 1, &ogltex->name);
	glTextureParameteri(ogltex->name, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(ogltex->name, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(ogltex->name, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTextureParameteri(ogltex->name, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTextureStorage2D(ogltex->name, 1, GL_RGB8, tex->width, tex->height);
	glTextureSubImage2D(ogltex->name, 0, 0, 0, tex->width, tex->height, GL_BGR, GL_UNSIGNED_BYTE, data);
	if(bindless) 	{
		ogltex->bindless = glGetTextureHandleARB(ogltex->name);
	}
}

KH_INLINE OglTexture2DContainer
ogl45_create_texture_2D_container(u32 slices_count, u32 mipmaps_count, GLenum internalformat, u32 w, u32 h, u32 arr_index, b32 sparse, b32 bindless, OglTextureParam *param) {
	OglTexture2DContainer res;

	GLuint name;
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &name);

	res.sparse = sparse;

	if(sparse) {
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
			res.sparse = false;
		}
	}

	GLint max_slices;
	if(res.sparse) {
		glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB, &max_slices);
	} else {
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_slices);
	}
	kh_assert(slices_count <= (u32)max_slices);

	glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, param->filter);
	glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, param->filter);
	glTextureParameteri(name, GL_TEXTURE_WRAP_S, param->wrap);
	glTextureParameteri(name, GL_TEXTURE_WRAP_T, param->wrap);
	if(param->swizzle) {
		glTextureParameteri(name, GL_TEXTURE_SWIZZLE_R, param->swizzles[0]);
		glTextureParameteri(name, GL_TEXTURE_SWIZZLE_G, param->swizzles[1]);
		glTextureParameteri(name, GL_TEXTURE_SWIZZLE_B, param->swizzles[2]);
		glTextureParameteri(name, GL_TEXTURE_SWIZZLE_A, param->swizzles[3]);
	}
	glTextureStorage3D(name, mipmaps_count, internalformat, w, h, slices_count);

	res.name = name;
	res.bindless = 0;
	if(bindless) {
		res.bindless = glGetTextureHandleARB(name);
		glMakeTextureHandleResidentARB(res.bindless);
	}

	res.arr_index = arr_index;
	res.slices_count = 0;
	res.slices_max = slices_count;
	res.mipmaps_count = mipmaps_count;
	res.format = internalformat;
	res.width = w;
	res.height = h;

	return(res);
}

KH_INTERN void
ogl45_commit_texture(OglTexture2DContainer *container, GLsizei slice, GLboolean commit) {
	GLuint level = 0;
	glTexturePageCommitmentEXT(container->name, level, 0, 0, slice, container->width, container->height, 1, commit);
}

KH_INTERN void
ogl45_commit_texture(OglTexture2D *texture, u32 w, u32 h, GLboolean commit) {
	GLuint level = 0;
	glTexturePageCommitmentEXT(texture->name, level, 0, 0, texture->slice, w, h, 1, commit);
}
KH_INTERN void
ogl45_set_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *tex, u32 slice) {
	kh_assert(slice < container->slices_max);
	tex->bindless = container->bindless;
	tex->name = container->name;
	tex->slice = slice;
	tex->container_id = container->arr_index;
	if(container->sparse) {
		ogl45_commit_texture(container, tex->slice, GL_TRUE);
	}
}

KH_INTERN void
ogl45_add_texture_to_container(OglTexture2DContainer *container, OglTexture2D *tex) {
	ogl45_set_texture_2d_to_container(container, tex, container->slices_count++);
}

// TODO(flo): remove this, @TEMP
KH_INTERN void
ogl45_set_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *ogltex, Texture2D *tex, u8 *data, 
                                GLenum format, u32 slice) {
	kh_assert(slice < container->slices_max);
	kh_assert(tex->width == container->width);
	kh_assert(tex->height == container->height);

	ogltex->bindless = container->bindless;
	ogltex->name = container->name;
	ogltex->slice = slice;
	ogltex->container_id = container->arr_index;
	// TODO(flo): specify levels, xoffset and yoffset in our Texture2D
	if(container->sparse) {
		ogl45_commit_texture(container, ogltex->slice, GL_TRUE);
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
	glTextureSubImage3D(container->name, 0, 0, 0, ogltex->slice, tex->width, tex->height, 1, format, GL_UNSIGNED_BYTE, data);
#endif

}

KH_INTERN void
ogl45_add_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *ogltex, Texture2D *tex, u8 *data,
                                GLenum format) {
	ogl45_set_texture_2d_to_container(container, ogltex, tex, data, format, container->slices_count++);
}

KH_INTERN void
ogl45_add_texture_2d_to_container(OglTexture2DContainer *container, OglTexture2D *dst) {
	u32 slice = container->slices_count++;
	dst->bindless = container->bindless;
	dst->name = container->name;
	dst->slice = slice;
	dst->container_id = container->arr_index;
	if(container->sparse) {
		ogl45_commit_texture(container, slice, GL_TRUE);
	}
}

// TODO(flo): use our new MaterialInfo and MaterialParam for this!
KH_INTERN void
ogl45_get_material_struct_from_type(char *dst, MaterialType type) {
	char *str = "";
	switch(type) {
		case Type_Material_T1F3 : {
			str = 
			"struct Material {\n"
			"	TexAddress diffuse;\n"
			"	float spec_power;\n"
			"	float spec_intensity;\n"
			"	float reflect_intensity;\n"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		case Type_Material_T2F2 : {
			str = 
			"struct Material {\n"
			"	TexAddress diffuse;\n"
			"	TexAddress normal;\n"
			"	float spec_power;\n"
			"	float spec_intensity;\n"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		case Type_Material_T2F3 : {
			str = 
			"struct Material {\n"
			"	TexAddress diffuse;\n"
			"	TexAddress normal;\n"
			"	float roughness;\n"
			"	float metallic;\n"
			"	float ao;\n"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		case Type_Material_T5 : {
			str = 
			"struct Material {\n"
			"	TexAddress diffuse;\n"
			"	TexAddress normal;\n"
			"	TexAddress roughness;\n"
			"	TexAddress metallic;\n"
			"	TexAddress ao;\n"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		case Type_Material_F4 : {
			str = 
			"struct Material {\n"
			"	vec4 color;"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		case Type_Material_T1 : {
			str = 
			"struct Material {\n"
			"	TexAddress tex;"
			"};\n"
			"layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {\n"
			"	Material mat[];\n"
			"};\n";
		} break;
		default : {
		} break;
	}
	kh_printf(dst, "%s", str);
}

KH_INTERN void
ogl_get_attributes_from_vertex_format(char *dst, VertexFormat format) {
	kh_printf(dst, 
	          "layout (location = LOC_DRAWID) in int in_drawid;\n"
	          "layout (location = LOC_INDIRECTCMDID) in int in_indirect_cmdid;\n"
      		  // "#define IndirectDrawCommandID gl_DrawIDARB\n"
	          // "#define IndirectDrawCommandID in_indirect_cmdid\n"
	          "#define IndirectDrawCommandID in_indirect_cmdid\n"
	          "layout (location = LOC_POSITION) in vec3 in_pos;\n"
	          "layout (location = LOC_NORMAL) in vec3 in_normal;\n"
	          "layout (location = LOC_UV0) in vec2 in_uv;\n"
	          );
	switch(format) {
		case VertexFormat_PNU :
		case VertexFormat_PNUS : {} break; 
		case VertexFormat_PNUT : 
		case VertexFormat_PNUTS : {
			kh_printf(dst,
			          "%slayout(location = LOC_TANGENT) in vec3 in_tangent;\n", 
			          dst);
		} break;
		case VertexFormat_PNUTB : 
		case VertexFormat_PNUTBS : {
			kh_printf(dst,
			          "%s"
			          "layout(location = LOC_TANGENT) in vec3 in_tangent;\n" 
			          "layout(location = LOC_BITANGENT) in vec3 in_bitangent;\n"
			          "#define HAS_TB\n", 
			          dst);

		} break;
		INVALID_DEFAULT_CASE;
	}
	if(has_skin(format)) {
		kh_printf(dst,
		          "%s"
		          "layout (location = LOC_BONE_IDS) in ivec4 in_bone_ids;\n"
		          "layout (location = LOC_BONE_WEIGHTS) in vec4 in_bone_weights;\n"
		          "layout (std140, binding = BIND_BONETRANSFORM) buffer BONE_TRANSFORM {\n"
		          "	mat4 bones[];\n"
		          "};\n"
		          "layout (std430, binding = BIND_BONEOFFSET) buffer BONE_OFFSET {\n"
		          "	uint offset[];\n"
		          "};\n"
		          "uint get_offset() {\n"
		          "uint res = offset[in_drawid];\n"
		          "	return res;\n"
		          "}\n"
		          "#define HAS_SKIN\n",
		          dst);
	}
}

KH_INTERN GLuint
ogl45_create_vert_frag_prog(OglAPI *api, char **files, u32 file_count, 
                            VertexFormat format = VertexFormat_count_or_none, MaterialType type = Type_Material_none) {
	char vert_header[8192];
	kh_printf(vert_header,
		"#version 450 core\n"
		"#extension GL_ARB_shader_storage_buffer_object : require\n"
		// "#extension GL_ARB_shader_draw_parameters : require\n"
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
		"#define BIND_BONEOFFSET %d\n"
		"#define BIND_MATERIAL %d\n\n"
		"struct CameraTransform {\n"
	    "	mat4 vp;\n"
	    "	mat4 view;\n"
	    "	mat4 rot_vp;\n"
	    "	vec4 pos;\n"
	    "};\n"
	    "struct Transform {\n"
	    "	mat4 model;\n"
	    "};"
	    "layout (std140, binding = BIND_CAMERATRANSFORM) uniform CAMERA_BLOCK {\n"
		"	CameraTransform cam;\n"
		"};\n"
		"layout (std140, binding = BIND_TRANSFORM) buffer TRANSFORM_BLOCK {\n"
		"	Transform tr[];\n"
		"};\n"
		"layout (std140, binding = BIND_LIGHTTRANSFORM) uniform LIGHTMATRIX_BLOCK {\n"
		"	mat4 light_matrix;\n"
		"};\n",
		LOC_POSITION, LOC_NORMAL, LOC_UV0, LOC_DRAWID, LOC_INDIRECTCMDID, LOC_TANGENT, LOC_BITANGENT, LOC_BONE_IDS, 
		LOC_BONE_WEIGHTS, BIND_TRANSFORM, BIND_CAMERATRANSFORM, BIND_LIGHTTRANSFORM, BIND_BONETRANSFORM, BIND_BONEOFFSET,
		BIND_MATERIAL);

	if(format != VertexFormat_count_or_none) {
		char attribute_str[1024];
		ogl_get_attributes_from_vertex_format(attribute_str, format);
		kh_printf(vert_header, "%s\n%s\n", vert_header, attribute_str);
	}

	// TODO(flo): this is for renderdoc since it does not support bindless texture and we need it for debuggin, if we do not have
	// bindless texture available we should not be using this OpenGL version?
	char *texture_header;
	if(api->bindless) {
		texture_header = 
			"#extension GL_ARB_bindless_texture : require\n"
			"struct TexAddress {\n"
			"	sampler2DArray container;\n"
			"	float page;\n"
			"};\n"
			"vec4 custom_texture(TexAddress addr, vec2 uv) {\n"
			"	return texture(addr.container, vec3(uv, addr.page));\n"
			"}\n";
	} else {
		texture_header = 
			"struct TexAddress {\n"
			"	uint container;\n"
			"	float page;\n"
			"};\n"
			"layout (binding = BIND_TEXTURES) uniform sampler2DArray texarray_container[MAX_TEXTURE_BINDINGS];\n"
			"vec4 custom_texture(TexAddress addr, vec2 uv) {\n"
			"	return texture(texarray_container[addr.container], vec3(uv, float(addr.page)));\n"
			"}\n";
	}

	char *frag_header_end = 
		"struct Light {\n"
		"	uint type;\n"
		"	float color_x;\n"
		"	float color_y;\n"
		"	float color_z;\n"
		"	float pos_x;\n"
		"	float pos_y;\n"
		"	float pos_z;\n"
		"	float dir_x;\n"
		"	float dir_y;\n"
		"	float dir_z;\n"
		"	float cutoff;\n"
		"	float outer_cutoff;\n"
		"};\n"
		"struct LightProperty {\n"
		"	vec3 color;\n"
		"	float attenuation;\n"
		"	vec3 L;\n"
		"	float intensity;\n"
		"};\n"
		"LightProperty get_light_property(Light l, vec3 wld_p) {\n"
		"	LightProperty res;\n"
		"	res.color = vec3(l.color_x, l.color_y, l.color_z);\n"
		"	if(l.type == LIGHTTYPE_DIRECTIONAL) {\n"
		"		res.attenuation = 1.0;\n"
		"		res.L = normalize(vec3(l.dir_x, l.dir_y, l.dir_z));\n"
		"		res.intensity = 1.0;\n"
		"	} else if(l.type == LIGHTTYPE_POINT) {\n"
		"		vec3 l_pos = vec3(l.pos_x, l.pos_y, l.pos_z);\n"
		"		vec3 to_light = l_pos - wld_p;\n"
		"		float dist = length(to_light);\n"
		"		float inv_dist = 1.0 / dist;\n"
		"		res.attenuation = inv_dist * inv_dist;\n"
		"		res.L = to_light * inv_dist;\n"
		"		res.intensity = 1.0;\n"
		"	} else if(l.type == LIGHTTYPE_SPOT) {\n"
		"		vec3 l_pos = vec3(l.pos_x, l.pos_y, l.pos_z);\n"
		"		vec3 l_dir = vec3(l.dir_x, l.dir_y, l.dir_z);\n"
		"		vec3 to_light = l_pos - wld_p;\n"
		"		float dist = length(to_light);\n"
		"		float inv_dist = 1.0 / dist;\n"
		"		vec3 L = to_light * inv_dist;\n"
		"		float theta = max(dot(l_dir, L), 0.0);\n"
		"		float eps = l.cutoff - l.outer_cutoff;\n"
		"		res.attenuation = inv_dist * inv_dist;\n"
		"		res.L = L;\n"
		"		res.intensity = clamp((theta - l.outer_cutoff) / eps, 0.0, 1.0);\n"
		"	}\n"
		"	return(res);\n"
		"}\n"
		"layout (std140, binding = BIND_LIGHT) uniform LIGHT_BLOCK {\n"
		"	Light light;\n"
		"};\n"
		"layout (std140, binding = BIND_SHADOWMAP) uniform SHADOW_BLOCK {\n"
		"	TexAddress shadow_map;\n"
		"};\n"
		"layout (binding = BIND_SKYBOX) uniform samplerCube skybox;\n";

	char frag_header[8192];
	kh_printf(frag_header, 
 	"#version 450 core\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"#define BIND_MATERIAL %d\n"
	"#define BIND_LIGHT %d\n"
	"#define BIND_SHADOWMAP %d\n"
	"#define BIND_TEXTURES %d\n"
	"#define BIND_SKYBOX %d\n"
	"#define BIND_PREFILTER %d\n"
	"#define BIND_BRDF_LUT %d\n"
	"#define MAX_TEXTURE_BINDINGS %d\n"
	"#define LIGHTTYPE_DIRECTIONAL %u\n"
	"#define LIGHTTYPE_POINT %u\n"
	"#define LIGHTTYPE_SPOT %u\n"
	,BIND_MATERIAL, BIND_LIGHT, BIND_SHADOWMAP, BIND_TEXTURES, BIND_SKYBOX, BIND_PREFILTER, BIND_BRDF_LUT, MAX_TEXTURE_BINDINGS
	,(u32)LightType_directional, (u32)LightType_point, (u32)LightType_spot);

	if(format != VertexFormat_count_or_none) {
		if(has_skin(format)) {
			kh_printf(frag_header,
			          "%s"
			          "#define HAS_SKIN\n",
			          frag_header);
		}
		if(format == VertexFormat_PNUTB || format == VertexFormat_PNUTBS) {
			kh_printf(frag_header,
			          "%s"
			          "#define HAS_TB\n",
			          frag_header);
		}
	}

	kh_printf(frag_header, 
    "%s\n"
	"%s\n"
	"%s",
	frag_header, texture_header, frag_header_end);

	if(type != Type_Material_none) {
		char material_str[1024];
		ogl45_get_material_struct_from_type(material_str, type);	
		kh_printf(frag_header, "%s\n%s\n", frag_header, material_str);
	}

	GLuint res = glCreateProgram();

	// kh_assert(file_count == 2);
	const GLenum shader_types[] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};

	const char* shader_headers[] = {
		vert_header,
		frag_header,
	};
	u32 name_count = 0;
	GLuint names[ShaderType_none];
	TransientLinear tmp = kh_begin_transient(api->arena);
	if(file_count == 1) {
		FileHandle shader = g_platform.open_file(files[0], FileAccess_read, FileCreation_only_open);
		kh_assert(!shader.error);
		u32 file_size = g_platform.get_file_size(&shader);

		char *file_contents = (char *)kh_push(api->arena, file_size);
		g_platform.read_bytes_of_file(&shader, 0 , file_size, file_contents);
		g_platform.close_file(&shader);

		struct ShaderSourceFromFile {
			umm offset;
			umm size;
		};

		StringTokenizer str_tok = {file_contents};
		Token tok = get_token_and_next(&str_tok);

		ShaderSourceFromFile shader_srcs[ShaderType_none] = {};
		ShaderType cur = ShaderType_none;

		while(!token_fit(tok, Token_end_of_file)) {
			if(token_fit(tok, Token_sharp)) {
				tok = get_token_and_next(&str_tok);
				if(token_fit(tok, Token_word)) {
					if(word_fit_NNT(tok, "shader")) {
						if(cur != ShaderType_none) {
							shader_srcs[cur].size = (umm)str_tok.pos - (umm)file_contents - tok.text_length - 1 - shader_srcs[cur].offset;
						}
						tok = get_token_and_next(&str_tok);
						kh_assert(token_fit(tok, Token_word));
						if(word_fit_NNT(tok, "vertex")) {
							cur = ShaderType_vertex;
						} else if(word_fit_NNT(tok, "fragment")) {
							cur = ShaderType_fragment;
						} else {
							kh_assert(!"unrecognized shader type");
						}
						tok = get_token_and_next(&str_tok);
						umm offset = (umm)str_tok.pos - tok.text_length - (umm)file_contents;
						kh_assert(offset < file_size);
						shader_srcs[cur].offset = offset;
						int debugp = 4;
					}
				}
			}
			tok = get_token_and_next(&str_tok);
		}
		if(cur != ShaderType_none) {
			kh_assert(shader_srcs[cur].offset < file_size);
			shader_srcs[cur].size = file_size - shader_srcs[cur].offset;
		}

		for(u32 shader_i = 0; shader_i < ShaderType_none; ++shader_i) {
			u32 shader_size = shader_srcs[shader_i].size;
			if(shader_size) {
				char *header = (char *)shader_headers[shader_i];
				u32 header_size = string_length(header);
				char *shader_src = file_contents + shader_srcs[shader_i].offset;
				u32 total_size = header_size + shader_size + 1;
				GLchar *src = (GLchar *)kh_push(api->arena, total_size);
				kh_printf(src, "%s%.*s", header, shader_size, shader_src);
				src[total_size - 1] = '\0';
				GLuint name = ogl_compile_shader(shader_types[shader_i], 1, src, 0);
				DEBUG_ogl_get_shader_log(name);
				glAttachShader(res, name);
				names[name_count++] = name;
			}
		}
	} else {
		kh_assert(file_count == 2);
		for(u32 file_i = 0; file_i < file_count; ++file_i) {
			char *header = (char *)shader_headers[file_i];
			GLchar *src = ogl_load_shader_from_file(header, string_length(header), files[file_i], api->arena);
			GLuint name = ogl_compile_shader(shader_types[file_i], 1, src, NULL);
			glAttachShader(res, name);
			DEBUG_ogl_get_shader_log(name);
			names[name_count++] = name;
		}
	}

	kh_end_transient(&tmp, true);
	glLinkProgram(res);
	DEBUG_ogl_get_prog_log(res);

	for(u32 name_i = 0; name_i < name_count; ++name_i) {
		glDeleteShader(names[name_i]);
	}
	return(res);
}

KH_INTERN void
ogl45_create_vertex_buffer(Ogl_4_5 *ogl, VertexFormat format) {

	OglVertexBuffer *vert_buffer = ogl->vertex_buffers + format;
	kh_assert(!vert_buffer->verts.total_size);
	kh_assert(!vert_buffer->inds.total_size);

	u32 size = get_size_from_vertex_format(format);
	b32 skinned = false;
	u32 anim_offset = 0;
	if(has_skin(format)) {
		skinned = true;
		i32 offset = get_skinned_attribute_offset(format).offset;
		kh_assert(offset != -1);
		anim_offset = (u32)offset;
	}
	ogl45_init_vertex_buffer(vert_buffer, size, MAX_VERTICES_COUNT, MAX_TRIANGLES_COUNT, skinned, anim_offset);
}

KH_INLINE OglTexture2DContainer *
ogl45_create_texture_2D_container(OglAPI *api, u32 slices_count, u32 mipmaps_count, GLenum internalformat, 
                            u32 w, u32 h, OglTextureParam *param) {
	Ogl_4_5 *ogl = &api->ver_4_5;
	kh_assert(ogl->texture_container_count < MAX_TEXTURE_BINDINGS);
	u32 id = ogl->texture_container_count++;
	OglTexture2DContainer *res = ogl->texture_containers + id; 
	*res = ogl45_create_texture_2D_container(slices_count, mipmaps_count, internalformat, w, h, id, 
	                                       api->sparse, api->bindless, param);
	ogl->texture_container_names[id] = res->name;
	return(res);
}

KH_INTERN void
ogl45_create_shader(RenderManager *render, OglAPI *api, ShadingType type, char **files, u32 file_count, VertexFormat format, 
                    MaterialType mat_type = Type_Material_none) {
	Ogl_4_5 *ogl = &api->ver_4_5;

	// TODO(flo): one above in the API???
	u32 shader_index = add_shading(render, type, format, &ogl->shader_count, ogl->shader_max_count);
	OglShader *ogl_shader = ogl->shaders + shader_index;

	kh_assert(format < VertexFormat_count_or_none);
	OglVertexBuffer *vert_buffer = ogl->vertex_buffers + format;
	if(!vert_buffer->verts.total_size) {
		kh_assert(!vert_buffer->inds.total_size);
		ogl45_create_vertex_buffer(ogl, format);
	}

	ogl_shader->prog_name = ogl45_create_vert_frag_prog(api, files, file_count, format, mat_type);
	ogl_shader->format = format;
	GLuint block_index = glGetProgramResourceIndex(ogl_shader->prog_name, GL_SHADER_STORAGE_BLOCK, "MATERIAL_BLOCK");
	ogl_shader->size = 0;
	if(block_index != -1) {
		GLint size;
		GLenum props = GL_BUFFER_DATA_SIZE;
		glGetProgramResourceiv(ogl_shader->prog_name, GL_SHADER_STORAGE_BLOCK, 
		                       block_index, 1, &props, 1, 0, &size);
		kh_assert(size > 0);
		/* TODO(flo): @IMPORTANT(flo): @ROBUSTNESS(flo): 
			be careful about our alignment for our material shader storage buffer!
		*/
		ogl_shader->size = KH_ALIGN_POW2(size, ogl->texture_address_alignment);
	}
}

// TODO(flo): do a better job than O(n) for searching here (hash function)
KH_INTERN OglTexture2DContainer *
ogl45_get_texture_container(OglAPI *api, u32 w, u32 h, u32 mipmaps_count, GLenum format, 
                            OglTextureParam param = ogl_default_texture_param()) {
	Ogl_4_5 *ogl = &api->ver_4_5;
	OglTexture2DContainer *res = 0;
	for(u32 i = 0; i < ogl->texture_container_count; ++i) {
		OglTexture2DContainer *search = ogl->texture_containers + i;
		if(search->width == w && search->height == h && search->mipmaps_count == mipmaps_count && search->format == format) {
			res = search;
			break;
		}
	}
	if(!res) {
		res = ogl45_create_texture_2D_container(api, 8, mipmaps_count, format, w, h, &param);	
		if(res && !api->bindless) {
			u32 index = ogl->texture_container_count - 1;
			kh_assert(index <= MAX_TEXTURE_BINDINGS);
			glBindTextureUnit(BIND_TEXTURES + index, ogl->texture_container_names[index]);
		}
	}
	kh_assert(res);
	return(res);
}

KH_INLINE OglTexture2D *
ogl45_get_texture_2d(OglAPI *api, RenderManager *render, Assets *assets, AssetID id) {
	Ogl_4_5 *ogl = &api->ver_4_5;
	OglTexture2D *res;
	Asset *asset = get_asset(assets, id, AssetType_tex2d);
	if(asset->header.gpu_index == INVALID_GPU_INDEX) {
		kh_assert(ogl->texture_count <= MAX_TEXTURES);
		u32 index = ogl->texture_count++;
		asset->header.gpu_index = index;
		res = ogl->textures + index;
		Texture2D tex = asset->source.type.tex2d;
		// TODO(flo): INCOMPLETE(flo): do we really support power of two textures?
		kh_assert(is_pow2(tex.width) && is_pow2(tex.height));
		u8 *tex_data = get_datas_from_asset(assets, asset);
		kh_assert(tex.bytes_per_pixel == 3);
		OglTexture2DContainer *container = ogl45_get_texture_container(api, tex.width, tex.height, 1, GL_RGB8);
		ogl45_add_texture_2d_to_container(container, res, &tex, tex_data, GL_BGR);
		asset->header.gpu_reload = false;
	} else {
		res = ogl->textures + asset->header.gpu_index;
		if(asset->header.gpu_reload) {
			Texture2D tex = asset->source.type.tex2d;
			u8 *data = get_datas_from_asset(assets, asset);
			kh_assert(tex.bytes_per_pixel == 3);
			glTextureSubImage3D(res->name, 0, 0, 0, res->slice, tex.width, tex.height, 1, GL_BGR, GL_UNSIGNED_BYTE, data);	
			asset->header.gpu_reload = false;
		}
	}
	return(res);
}

KH_INLINE OglTriangleMesh *
ogl45_get_triangle_mesh(Ogl_4_5 *ogl, RenderManager *render, Assets *assets, AssetID id, VertexFormat format) {
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
		ogl45_add_triangle_mesh_memory_to_vbo(res, vertex_buffer, &mesh, mesh_data);
	} else {
		res = ogl->meshes + asset->header.gpu_index;
		if(asset->header.gpu_reload) {
			OglVertexBuffer *vertex_buffer = ogl->vertex_buffers + format;
			TriangleMesh mesh = asset->source.type.trimesh;
			kh_assert(res->ind_count == mesh.tri_c * 3);
			u8 *data = get_datas_from_asset(assets, asset);
			glNamedBufferSubData(vertex_buffer->verts.name, res->vbo_offset * mesh.vertex_size, 
			                     mesh.vert_c * mesh.vertex_size, data); 
			asset->header.gpu_reload = false;
		}
	}
	return(res);
}

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"
KH_INLINE void
ogl45_set_skybox_test(OglAPI *api, Ogl_4_5 *ogl, RenderManager *render, Assets *assets) {
	GLfloat skybox_vert[] = {
		// back
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		// left
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		// right
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		// front
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		// top
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,

		// bottom
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
	ogl->pass[Pass_skybox].vertexbuffer = vertices;
	ogl->pass[Pass_skybox].vertexbuffer_size = sizeof(skybox_vert);
	char *files[] = {"shaders/gl450/skybox.shader"};
	ogl->pass[Pass_skybox].prog_name = ogl45_create_vert_frag_prog(api, files, array_count(files));
	stbi_set_flip_vertically_on_load(true);
	i32 w,h,n;
	// f32 *pixels = stbi_loadf("newport_loft.hdr", &w, &h, &n, 0);
	// f32 *pixels = stbi_loadf("grand_canyon.hdr", &w, &h, &n, 0);
	f32 *pixels = stbi_loadf("data/textures/sierra_madre.hdr", &w, &h, &n, 0);
	kh_assert(pixels);
	if(pixels) {	
		GLuint hdr_tex, skybox_tex, irradiance_tex, prefilter_tex, brdf_lookup_tex;
		const u32 max_mip_levels = 5;
		const u32 skybox_s = 512;
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// NOTE(flo) : HDR texture
		glCreateTextures(GL_TEXTURE_2D, 1, &hdr_tex);
		glTextureStorage2D(hdr_tex, 1, GL_RGB16F, w, h);
		glTextureSubImage2D(hdr_tex, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, pixels);
		glTextureParameteri(hdr_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(hdr_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(hdr_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(hdr_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// NOTE(flo): skybox texture
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &skybox_tex);
		glTextureStorage2D(skybox_tex, 1, GL_RGB16F, skybox_s, skybox_s);
		glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(skybox_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// glTextureParameteri(skybox_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(skybox_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// NOTE(flo): generate mip map to avoid dot artefacts
	    glTextureParameteri(skybox_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateTextureMipmap(skybox_tex);

		// NOTE(flo): irradiance texture
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &irradiance_tex);
		glTextureStorage2D(irradiance_tex, 1, GL_RGB16F, 32, 32);
		glTextureParameteri(irradiance_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(irradiance_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(irradiance_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(irradiance_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(irradiance_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// NOTE(flo): prefilter texture
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &prefilter_tex);
		glTextureStorage2D(prefilter_tex, max_mip_levels, GL_RGB16F, 128, 128);
		glTextureParameteri(prefilter_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(prefilter_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(prefilter_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTextureParameteri(prefilter_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTextureParameteri(prefilter_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateTextureMipmap(prefilter_tex);

		// NOTE(flo): BRDF LUT
		// OglTexture2DContainer *container = ogl45_get_texture_container(api, 512, 512, 1, GL_RG16F);
		// OglTexture2D brdf_lookup_tex;
		// ogl_add_texture_2d_to_container(container, &brdf_lookup_tex);
		glCreateTextures(GL_TEXTURE_2D, 1, &brdf_lookup_tex);
		glTextureStorage2D(brdf_lookup_tex, 1, GL_RG16F, 512, 512);
		glTextureParameteri(brdf_lookup_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(brdf_lookup_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(brdf_lookup_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTextureParameteri(brdf_lookup_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		GLuint capture_fbo;
		glCreateFramebuffers(1, &capture_fbo);

		mat4 projection = perspective_fov_lh(90.0f, 1.0f, 0.1f, 10.0f);
		mat4 views[6] = {
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(1,0,0), kh_vec3(0, 1, 0)), 
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(-1,0,0), kh_vec3(0, 1, 0)), 
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(0,-1,0), kh_vec3(0, 0, 1)), 
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(0,1,0), kh_vec3(0, 0, -1)), 
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(0,0,1), kh_vec3(0, 1, 0)),
			look_at_matrix_lh(kh_vec3(0,0,0), kh_vec3(0,0,-1), kh_vec3(0, 1, 0)), 
		};

		char *cubemap_files[] = {"shaders/hdr_to_cubemap.vert", "shaders/hdr_to_cubemap.frag"};
		GLuint prog = ogl45_create_vert_frag_prog(api, cubemap_files, array_count(cubemap_files));
		GLuint proj = glGetUniformLocation(prog, "proj");
		GLuint view = glGetUniformLocation(prog, "view");

		char *irradiance_files[] = {"shaders/hdr_to_cubemap.vert", "shaders/hdr_to_irradiance.frag"};
		GLuint irradiance_prog = ogl45_create_vert_frag_prog(api, irradiance_files, array_count(irradiance_files));
		GLuint irradiance_proj = glGetUniformLocation(irradiance_prog, "proj");
		GLuint irradiance_view = glGetUniformLocation(irradiance_prog, "view");

		char *prefilter_files[] = {"shaders/hdr_to_cubemap.vert", "shaders/hdr_to_prefilter.frag"};
		GLuint prefilter_prog = ogl45_create_vert_frag_prog(api, prefilter_files, array_count(prefilter_files));
		GLuint prefilter_proj = glGetUniformLocation(prefilter_prog, "proj");
		GLuint prefilter_view = glGetUniformLocation(prefilter_prog, "view");
		GLuint prefilter_roughness = glGetUniformLocation(prefilter_prog, "roughness");
		GLuint prefilter_resolution = glGetUniformLocation(prefilter_prog, "resolution");

		char *brdf_files[] = {"shaders/hdr_to_brdf.shader"};
		GLuint brdf_prog = ogl45_create_vert_frag_prog(api, brdf_files, array_count(brdf_files));

		glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, vertices, 0, 3 * sizeof(GLfloat));

		// NOTE(flo): render skybox from HDR texture
		glUseProgram(prog);
		glBindTextureUnit(0, hdr_tex);
		glUniformMatrix4fv(proj, 1, GL_FALSE, projection.e);
		glViewport(0, 0, skybox_s, skybox_s);
		for(u32 i = 0; i < 6; ++i) {
			glUniformMatrix4fv(view, 1, GL_FALSE, views[i].e);
			glNamedFramebufferTextureLayer(capture_fbo, GL_COLOR_ATTACHMENT0, skybox_tex, 0, i);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// NOTE(flo): render irradiance (diffuse) texture from skybox
		glUseProgram(irradiance_prog);
		glBindTextureUnit(0, skybox_tex);
		glUniformMatrix4fv(irradiance_proj, 1, GL_FALSE, projection.e);
		glViewport(0, 0, 32, 32);
		for(u32 i = 0; i < 6; ++i) {
			glUniformMatrix4fv(irradiance_view, 1, GL_FALSE, views[i].e);
			glNamedFramebufferTextureLayer(capture_fbo, GL_COLOR_ATTACHMENT0, irradiance_tex, 0, i);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// TODO(flo): render prefilter texture from skybox
		glUseProgram(prefilter_prog);
		glUniformMatrix4fv(prefilter_proj, 1, GL_FALSE, projection.e);
		glUniform1f(prefilter_resolution, (float)skybox_s);
		u32 mip_w = 128;
		u32 mip_h = 128;
		for(u32 mip_i = 0; mip_i < max_mip_levels; ++mip_i) {

			glViewport(0, 0, mip_w, mip_h);
			float roughness = (f32)mip_i / (f32)(max_mip_levels - 1);
			glUniform1f(prefilter_roughness, roughness);

			for(u32 cube_i = 0; cube_i < 6; ++cube_i) {
				glUniformMatrix4fv(prefilter_view, 1, GL_FALSE, views[cube_i].e);
				glNamedFramebufferTextureLayer(capture_fbo, GL_COLOR_ATTACHMENT0, prefilter_tex, mip_i, cube_i);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			mip_w = (mip_w + 1) / 2;
			mip_h = (mip_h + 1) / 2;
		}
		
		// TODO(flo): render BRDF texture
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, ogl->pass[Pass_blit].vertexbuffer, 0, 3 * sizeof(GLfloat));
		glUseProgram(brdf_prog);	
		glViewport(0, 0, 512, 512);
		glClear(GL_COLOR_BUFFER_BIT);
		glNamedFramebufferTexture(capture_fbo, GL_COLOR_ATTACHMENT0, brdf_lookup_tex, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &capture_fbo);
		glDeleteTextures(1, &hdr_tex);
		glUseProgram(0);
		glDeleteProgram(prog);
		glDeleteProgram(irradiance_prog);

		ogl->skybox_tex = skybox_tex;
		ogl->irradiance_tex = irradiance_tex;
		ogl->prefilter_tex = prefilter_tex;
		ogl->brdf_LUT = brdf_lookup_tex;
		glBindTextureUnit(BIND_SKYBOX, ogl->skybox_tex);
		glBindTextureUnit(BIND_PREFILTER, ogl->prefilter_tex);
		glBindTextureUnit(BIND_BRDF_LUT, ogl->brdf_LUT);
	}
}

KH_INLINE void
ogl45_set_skybox(OglAPI *api, Ogl_4_5 *ogl, RenderManager *render, Assets *assets) {
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
	char *shader_files[] = {"shaders/gl450/skybox.shader"};
	ogl->pass[Pass_skybox].vertexbuffer = vertices;
	ogl->pass[Pass_skybox].vertexbuffer_size = sizeof(skybox_vert);
	ogl->pass[Pass_skybox].prog_name = ogl45_create_vert_frag_prog(api, shader_files, array_count(shader_files));

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

	ogl->skybox_tex = skybox_tex;
}

KH_INTERN void
ogl45_delete_skybox(Ogl_4_5 *ogl) {
	if(ogl->has_skybox) {
		// glBindTextureUnit(BIND_SKYBOX, 0);
		glDeleteBuffers(1, &ogl->pass[Pass_skybox].vertexbuffer);
		ogl->pass[Pass_skybox].vertexbuffer_size = 0;
		glDeleteTextures(1, &ogl->skybox_tex);
		glDeleteProgram(ogl->pass[Pass_skybox].prog_name);
		ogl->skybox_tex = 0;
		ogl->has_skybox = false;
	}
}

struct LightToSend {
	u32 type;
	v3 color;
	v3 pos;
	v3 dir;
	f32 cutoff;
	f32 outer_cutoff;
};

KH_INTERN void
ogl45_update_light(OglLight *ogl_light, Light *light, u32 light_index) {
	LightToSend lts;
	lts.type = light->type;
	lts.color = light->color;
	lts.pos = light->pos;
	lts.dir = light->dir;
	lts.cutoff = light->cutoff;
	lts.outer_cutoff = light->outer_cutoff;
	glNamedBufferSubData(ogl_light->buffer, 0, sizeof(LightToSend), &lts);

	mat4 light_inverse_tr;
	v3 up_axis = kh_vec3(0,0,1);
	if(kh_abs_f32(light->dir.z) >= 0.8f) {
		up_axis = kh_vec3(0,1,0);
	}
	v3 z_axis = -light->dir;
	v3 x_axis = kh_cross_v3(up_axis, z_axis);
	v3 y_axis = kh_cross_v3(z_axis, x_axis);

	light_inverse_tr.m00 = x_axis.x; light_inverse_tr.m01 = y_axis.x; light_inverse_tr.m02 = z_axis.x; light_inverse_tr.m03 = 0;
	light_inverse_tr.m10 = x_axis.y; light_inverse_tr.m11 = y_axis.y; light_inverse_tr.m12 = z_axis.y; light_inverse_tr.m13 = 0;
	light_inverse_tr.m20 = x_axis.z; light_inverse_tr.m21 = y_axis.z; light_inverse_tr.m22 = z_axis.z; light_inverse_tr.m23 = 0;

	light_inverse_tr.m30 = -kh_dot_v3(x_axis, light->dir);
	light_inverse_tr.m31 = -kh_dot_v3(y_axis, light->dir);
	light_inverse_tr.m32 = -kh_dot_v3(z_axis, light->dir);
	light_inverse_tr.m33 = 1.0f;
	f32 scale = light->scale;
	mat4 light_proj = orthographic_off_center_lh(-scale, scale, -scale, scale, light->z_near, light->z_far);
	mat4 lighttransform = light_inverse_tr * light_proj;
	glNamedBufferSubData(ogl_light->transform, 0, sizeof(mat4), &lighttransform);
}

KH_INTERN void
ogl45_set_light(OglAPI *api, Light *light, u32 light_index) {
	Ogl_4_5 *ogl = &api->ver_4_5;
	kh_assert(light_index == 0);
	OglLight *ogl_light = ogl->lights + light_index;

	glCreateBuffers(1, &ogl_light->buffer);
	glNamedBufferStorage(ogl_light->buffer, sizeof(LightToSend), 0, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHT, ogl_light->buffer);

	ogl->shadow_ctr = ogl45_get_texture_container(api, 4096, 4096, 1, GL_DEPTH_COMPONENT24);
	ogl45_set_texture_2d_to_container(ogl->shadow_ctr, &ogl_light->shadow_tex, light_index);
	glCreateFramebuffers(1, &ogl_light->framebuffer);

	glNamedFramebufferTextureLayer(ogl_light->framebuffer, GL_DEPTH_ATTACHMENT, ogl_light->shadow_tex.name, 0, ogl_light->shadow_tex.slice);
	ogl45_get_fb_status(ogl_light->framebuffer, GL_FRAMEBUFFER);

	glCreateBuffers(1, &ogl_light->shadow_buffer);
	if(api->bindless) {
		OglBindlessTextureAddress addr = {ogl_light->shadow_tex.bindless, (f32)ogl_light->shadow_tex.slice};
		glNamedBufferData(ogl_light->shadow_buffer, sizeof(OglBindlessTextureAddress), &addr, GL_STATIC_DRAW);
	} else {
		OglTextureAddress addr = {ogl_light->shadow_tex.container_id, (f32)ogl_light->shadow_tex.slice};
		glNamedBufferData(ogl_light->shadow_buffer, sizeof(OglTextureAddress), &addr, GL_STATIC_DRAW);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_SHADOWMAP, ogl_light->shadow_buffer);

	glCreateBuffers(1, &ogl_light->transform);
	glNamedBufferStorage(ogl_light->transform, sizeof(mat4), 0, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHTTRANSFORM, ogl_light->transform);
}

KH_INTERN void
ogl45_delete_light(Ogl_4_5 *ogl, b32 sparse) {
	if(ogl->light_count > 0) {
		kh_assert(ogl->light_count == 1);
		OglLight *light = ogl->lights + 0;

		glDeleteFramebuffers(1, &light->framebuffer);
		glDeleteBuffers(1, &light->buffer);
		glDeleteBuffers(1, &light->shadow_buffer);
		glDeleteBuffers(1, &light->transform);
		if(sparse) {
			ogl45_commit_texture(ogl->shadow_ctr, light->shadow_tex.slice, false);	
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_SHADOWMAP, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHT, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_LIGHTTRANSFORM, 0);

		light->framebuffer = 0;
		light->buffer = 0;
		light->shadow_buffer = 0;
		light->transform = 0;
		ogl->light_count = 0;
		
	}
}

KH_INLINE void
ogl45_display(OglAPI *api, u32 w, u32 h, void *pixels, b32 blit) {
	Ogl_4_5 *ogl = &api->ver_4_5;

	if(!blit) {
		glNamedBufferSubData(ogl->pixel_unpack_buffer, 0, api->target_w * api->target_h * sizeof(u32), pixels);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ogl->pixel_unpack_buffer);
		glTextureSubImage3D(api->target.name, 0, 0, 0, api->target.slice, api->target_w, api->target_h, 1, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	if(blit && api->multisample) {
		OglPass *scene_pass = ogl->pass + Pass_render_3d_scene; 
		OglPass *blit_pass = ogl->pass + Pass_blit;
		glBindFramebuffer(GL_FRAMEBUFFER, blit_pass->framebuffer);
		u32 target_w = api->target_w;
		u32 target_h = api->target_h;
		glBlitNamedFramebuffer(scene_pass->framebuffer, blit_pass->framebuffer, 0, 0, target_w, target_h, 
		                       0, 0, target_w, target_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	f32 aspect_ratio = (f32)api->target_w / (f32)api->target_h;
    u32 new_w = (u32)((f32)h * aspect_ratio);
    u32 pad_w = (w - new_w) / 2;
    u32 pad_h = 0;
    u32 new_h = h;
    if(new_w > w) {
    	f32 inv_ar = (f32)api->target_h / (f32)api->target_w;
    	new_w = w;
    	pad_w = 0;
    	new_h = (u32)((f32)w * inv_ar);
    	kh_assert(new_h <= h);
    	pad_h = (h - new_h) / 2;
    }
	glViewport(pad_w, pad_h, new_w, new_h);
	OglPass *blit_pass = ogl->pass + Pass_blit;
	glUseProgram(blit_pass->prog_name);
	glBindVertexBuffer(ATTRIB_VERTEX_DATA, blit_pass->vertexbuffer, 0, 3 * sizeof(GLfloat));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, blit_pass->matbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// ogl_DEBUG_draw_texture(ogl, &ogl->shadow_map, 0, 0, 200, 200);
}

KH_INTERN void
ogl45_reset(OglAPI *api) {
	Ogl_4_5 *ogl = &api->ver_4_5;
	ogl45_delete_skybox(ogl);
	ogl45_delete_light(ogl, api->sparse);
}

KH_INTERN void
ogl45_init(OglAPI *api, RenderManager *render, Assets *assets) {

	Ogl_4_5 *ogl = &api->ver_4_5;

	OglExtensionsHash *hash = &api->exts;
	ogl45_check_required_ogl_extensions(hash);

	u32 flags = GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT;
	u32 create_flags = flags | GL_DYNAMIC_STORAGE_BIT;

	api->wireframe = false;
	api->bindless = ogl_check_ext(hash, "GL_ARB_bindless_texture");
	api->sparse = ogl_check_ext(hash, "GL_ARB_sparse_texture");
	api->sparse = false;
	api->bindless = false;
	api->multisample = true;
	api->multisample_count = 4;
	api->zprepass_enabled = false;

	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &ogl->shader_storage_buffer_alignment);
	kh_assert(is_pow2(ogl->shader_storage_buffer_alignment));

	ogl->texture_address_alignment = ogl_get_texture_address_alignment(api->bindless);
	kh_assert(is_pow2(ogl->texture_address_alignment));

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(ogl45_debug_callback, 0);

	glEnable(GL_MULTISAMPLE);

	// TODO(flo): one above in the API ????
	ogl->shader_max_count = 32;
	ogl->shader_count = 0;
	ogl->shaders = kh_push_array(api->arena, ogl->shader_max_count, OglShader);


	// u16 test_pix[] = {
	// 	0, 1, 2, 3
	// };
	// GLuint test_tex;
	// glCreateTextures(GL_TEXTURE_2D, 1, &test_tex);
	// glTextureStorage2D(test_tex, 1, GL_R16UI, 4, 4);
	// glTextureSubImage2D(test_tex, 0, 0, 0, 2, 2, GL_RED_INTEGER, GL_UNSIGNED_SHORT, test_pix);

	// NOTE(flo): INIT MATERIALS AND VBOS
	// TODO(flo): program pipeline!
	{
		{
			char *files[] = {"shaders/gl450/phong.shader"};
			ogl45_create_shader(render, api, Shading_phong, files, array_count(files), VertexFormat_PNU, Type_Material_T1F3);
		}
		{
			char *files[] = {"shaders/gl450/phong.shader"};
			ogl45_create_shader(render, api, Shading_phong, files, array_count(files), VertexFormat_PNUS, Type_Material_T1F3);
		}
		{
			char *files[] = {"shaders/gl450/normalmapping.shader"};
			ogl45_create_shader(render, api, Shading_normalmap, files, array_count(files), VertexFormat_PNUTB, Type_Material_T2F2);
		}
		{
			char *files[] = {"shaders/gl450/normalmapping.shader"};
			ogl45_create_shader(render, api, Shading_normalmap, files, array_count(files), VertexFormat_PNUTBS, Type_Material_T2F2);
		}
		{
			char *files[] = {"shaders/gl450/pbr.shader"};
			ogl45_create_shader(render, api, Shading_pbr, files, array_count(files), VertexFormat_PNUTB, Type_Material_T5);
		}
		{
			char *files[] = {"shaders/gl450/pbr.shader"};
			ogl45_create_shader(render, api, Shading_pbr, files, array_count(files), VertexFormat_PNU, Type_Material_T5);
		}
		{
			char *files[] = {"shaders/gl450/pbr.shader"};
			ogl45_create_shader(render, api, Shading_pbr, files, array_count(files), VertexFormat_PNUTBS, Type_Material_T5);
		}
		{
			char *files[] = {"shaders/gl450/color.shader"};
			ogl45_create_shader(render, api, Shading_color, files, array_count(files), VertexFormat_PNU, Type_Material_F4);
		}
		{
			char *files[] = {"shaders/gl450/shadowmap.shader"};
			ogl->pass[Pass_shadow].prog_name = ogl45_create_vert_frag_prog(api, files, array_count(files));
		}
		{
			char *files[] = {"shaders/gl450/zprepass.shader"};
			ogl->pass[Pass_zpre].prog_name = ogl45_create_vert_frag_prog(api, files, array_count(files));
		}
		{
			char *files[] = {"shaders/gl450/diffuse.shader"};
			ogl45_create_shader(render, api, Shading_diffuse, files, array_count(files), VertexFormat_PNU, Type_Material_T1);
		}
	}

	// NOTE(flo): INIT TEXTURES
	{
		// TODO(flo): hash for this ? we need to create on fly
		OglTextureParam param = ogl_default_texture_param();
		ogl45_create_texture_2D_container(api, 8, 1, GL_RGB8, 4096, 4096, &param);
		ogl45_create_texture_2D_container(api, 2, 1, GL_RGB8, 512, 512, &param);
		ogl45_create_texture_2D_container(api, 2, 1, GL_RGB8, 1024, 1024, &param);
		ogl45_create_texture_2D_container(api, 1, 1, GL_DEPTH_COMPONENT24, 4096, 4096, &param);
		ogl45_create_texture_2D_container(api, 1, 1, GL_RGB8, 16, 16, &param);	
	}

	// NOTE(flo): INIT ANIMATIONS
	{
		glCreateBuffers(1, &ogl->joint_transform);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_BONETRANSFORM, ogl->joint_transform);

		GLuint bone_offset;
		glCreateBuffers(1, &bone_offset);
		glNamedBufferStorage(bone_offset, sizeof(u32) * MAX_ENTRIES, NULL, flags);
		ogl->map_boneoffset = (u32 *)glMapNamedBufferRange(bone_offset, 0, sizeof(u32) * MAX_ENTRIES, flags);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_BONEOFFSET, bone_offset);
	}

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// NOTE(flo): BLIT PASS
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
		char *files[] = {"shaders/gl450/fbo_texture.shader"};
		ogl->pass[Pass_blit].vertexbuffer_size = sizeof(fb_pos);
		ogl->pass[Pass_blit].vertexbuffer = vertices;
		ogl->pass[Pass_blit].prog_name = ogl45_create_vert_frag_prog(api, files, array_count(files));


		OglTextureParam param = ogl_default_texture_param();
		OglTexture2DContainer *container_fb = ogl45_create_texture_2D_container(api, 2, 1, GL_RGBA8, 
		                                                                  render->width, render->height, &param);
		ogl45_add_texture_to_container(container_fb, &api->target);

		if(api->multisample) {
			glCreateFramebuffers(1, &ogl->pass[Pass_blit].framebuffer);
			glNamedFramebufferTextureLayer(ogl->pass[Pass_blit].framebuffer, GL_COLOR_ATTACHMENT0, 
			                               api->target.name, 0, api->target.slice);
			ogl45_get_fb_status(ogl->pass[Pass_blit].framebuffer, GL_FRAMEBUFFER);
		} else {
			ogl->pass[Pass_blit].framebuffer = 0;
		}

		glCreateBuffers(1, &ogl->pass[Pass_blit].matbuffer);
		if(api->bindless) {
			OglBindlessTextureAddress addr = {api->target.bindless, (f32)api->target.slice};
			glNamedBufferData(ogl->pass[Pass_blit].matbuffer, sizeof(OglBindlessTextureAddress), &addr, GL_STATIC_DRAW);
			ogl->pass[Pass_blit].matbuffer_size = sizeof(OglBindlessTextureAddress);
		}
		else {
			OglTextureAddress addr = {api->target.container_id, (f32)api->target.slice};
			glNamedBufferData(ogl->pass[Pass_blit].matbuffer, sizeof(OglTextureAddress), &addr, GL_STATIC_DRAW);
			ogl->pass[Pass_blit].matbuffer_size = sizeof(OglTextureAddress);
		}
	}

	// v3 light_dir = kh_vec3(0.0f, -0.5f, 0.86602540378f);
	// f32 length_sqr = kh_dot_v3(light_dir, light_dir);

	// @NOTE(flo): 3D SCENE PASS
	{
		// @TODO(flo): pack in TEXTURE_2D_ARRAY all the textures that got the same 
		//  #levels, internalformat, width and height (we'll need a hash for this)
		// and keep the depth (zoffset and the handle)
		// @TODO(flo): allocate directly to vbo with bufferstorage and mapbuffer so we do not need texture->memory 
		// and mesh->memory, the entry will just need a buffer id and a buffer offset
		OglPass *scene_pass = ogl->pass + Pass_render_3d_scene;
		glCreateBuffers(1, &scene_pass->matbuffer);
		scene_pass->matbuffer_size = kilobytes(64);
		glNamedBufferStorage(scene_pass->matbuffer, scene_pass->matbuffer_size, 0, GL_DYNAMIC_STORAGE_BIT|GL_MAP_COHERENT_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_WRITE_BIT|GL_MAP_READ_BIT);
		
		GLuint rb;
		glCreateFramebuffers(1, &ogl->pass[Pass_render_3d_scene].framebuffer);
		glCreateRenderbuffers(1, &rb);

		if(api->multisample) {
			GLuint texMS;
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texMS);
			glTextureStorage2DMultisample(texMS, api->multisample_count, GL_RGBA8, render->width, render->height, GL_TRUE);
			glNamedRenderbufferStorageMultisample(rb, api->multisample_count, GL_DEPTH_COMPONENT, render->width, render->height);
			glNamedFramebufferTexture(ogl->pass[Pass_render_3d_scene].framebuffer, GL_COLOR_ATTACHMENT0, texMS, 0);
		} else {
			glNamedFramebufferTextureLayer(ogl->pass[Pass_render_3d_scene].framebuffer, GL_COLOR_ATTACHMENT0, 
			                               api->target.name, 0, api->target.slice);
			glNamedRenderbufferStorage(rb, GL_DEPTH_COMPONENT, render->width, render->height);
		}

		glNamedFramebufferRenderbuffer(ogl->pass[Pass_render_3d_scene].framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
		// glNamedFramebufferTextureLayer(ogl->pass[Pass_render_3d_scene].framebuffer, GL_COLOR_ATTACHMENT0, ogl->target.name, 0, ogl->target.slice);
		// NOTE(flo): we specify that we want to draw the COLOR_ATTACHMENT0 of our framebuffer, it seems not to be
		// necessary
		// GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
		// glDrawBuffers(1, bufs);
		ogl45_get_fb_status(ogl->pass[Pass_render_3d_scene].framebuffer, GL_FRAMEBUFFER);

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

		GLuint draw_id;
		glCreateBuffers(1, &draw_id);
		glNamedBufferStorage(draw_id, sizeof(u32) * MAX_ENTRIES, 0, create_flags);
		ogl->map_drawids = (u32 *)glMapNamedBufferRange(draw_id, 0, sizeof(u32) * MAX_ENTRIES, flags);
		glVertexAttribIFormat(LOC_DRAWID, 1, GL_UNSIGNED_INT, 0);
		glVertexAttribBinding(LOC_DRAWID, ATTRIB_DRAWID);
		glVertexBindingDivisor(ATTRIB_DRAWID, 1);
		glEnableVertexAttribArray(LOC_DRAWID);
		glBindVertexBuffer(ATTRIB_DRAWID, draw_id, 0, sizeof(GL_UNSIGNED_INT));

		GLuint indirect_cmd_id;
		glCreateBuffers(1, &indirect_cmd_id);
		glNamedBufferStorage(indirect_cmd_id, sizeof(u32) * MAX_ENTRIES, 0, create_flags);
		ogl->map_indirectcmdids = (u32 *)glMapNamedBufferRange(indirect_cmd_id, 0, sizeof(u32) * MAX_ENTRIES, flags);
		glVertexAttribIFormat(LOC_INDIRECTCMDID, 1, GL_UNSIGNED_INT, 0);
		glVertexAttribBinding(LOC_INDIRECTCMDID, ATTRIB_DRAWCMDID);
		glEnableVertexAttribArray(LOC_INDIRECTCMDID);
		glVertexBindingDivisor(ATTRIB_DRAWCMDID, 1);
		glBindVertexBuffer(ATTRIB_DRAWCMDID, indirect_cmd_id, 0, sizeof(GL_UNSIGNED_INT));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_TRANSFORM, transform);

		GLuint cam_tr;
		glCreateBuffers(1, &cam_tr);
		glNamedBufferStorage(cam_tr, sizeof(OglCameraTransform), 0, create_flags);
		ogl->map_cam_transform = (OglCameraTransform *)glMapNamedBufferRange(cam_tr, 0, sizeof(OglCameraTransform), flags);
		glBindBufferBase(GL_UNIFORM_BUFFER, BIND_CAMERATRANSFORM, cam_tr);

		glCreateBuffers(1, &ogl->cmds_buffer);
		glNamedBufferData(ogl->cmds_buffer, sizeof(DrawElementsIndirectCommand) * MAX_DRAWCMD_PER_FRAME, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ogl->cmds_buffer);
	}

	if(!api->bindless) {
		glBindTextures(BIND_TEXTURES, ogl->texture_container_count, ogl->texture_container_names);
	}
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

	// NOTE(flo): DISPLAY 
	{
		api->target_w = render->width;
		api->target_h = render->height;
		glCreateBuffers(1, &ogl->pixel_unpack_buffer);
		glNamedBufferData(ogl->pixel_unpack_buffer, api->target_w * api->target_h * sizeof(u32), 0, GL_DYNAMIC_COPY);

	}
}
#undef ogl_prog

KH_INTERN u32
ogl45_send_mat_textures(OglAPI *api, RenderManager *render, Assets *assets, 
                        AssetID *textures, u32 texture_count, u32 offset, GLuint matbuffer) {
	u32 res = offset;
	for(u32 i = 0; i < texture_count; ++i) {
		AssetID tex_id = textures[i];
		b32 tex_loaded = (get_asset_state(assets, tex_id) == AssetState_loaded);
		kh_assert(tex_loaded);
		if(tex_id.val && tex_loaded) {
			OglTexture2D *texture = ogl45_get_texture_2d(api, render, assets, tex_id);
			if(api->bindless) {
				OglBindlessTextureAddress texaddr = {texture->bindless, (f32)texture->slice};
				glNamedBufferSubData(matbuffer, res, sizeof(OglBindlessTextureAddress), &texaddr);
			} else {
				OglTextureAddress texaddr = {texture->container_id, (f32)texture->slice};
				glNamedBufferSubData(matbuffer, res, sizeof(OglTextureAddress), &texaddr);
			}
		}
		res += api->bindless ? sizeof(OglBindlessTextureAddress) : sizeof(OglTextureAddress); 
	}
	return(res);
}

KH_INTERN void
ogl45_update(OglAPI *api, RenderManager *render, Assets *assets) {

	Ogl_4_5 *ogl = &api->ver_4_5;
	u32 batch_count = render->batch_count;
	u32 cmd_total_count = 0;
	u32 base_instance = 0;
	u32 total_entry_count = 0;

	OglPass *scene_pass = ogl->pass + Pass_render_3d_scene;
	GLuint matbuffer = scene_pass->matbuffer;
	u32 matbuffer_size = scene_pass->matbuffer_size;

	ogl->used_shaders_count = 0;

	if(render->batch_count > 0 && render->entry_count > 0) {
		glNamedBufferData(ogl->joint_transform, render->joint_tr.count * sizeof(mat4), render->joint_tr.data, GL_DYNAMIC_DRAW);
		u32 mat_offset = 0;
		kh_lnext(VertexBuffer, buffer, render->first_vertex_buffer) {
			VertexFormat fmt = buffer->format;
			kh_lnext(Shading, shading, buffer->first) {
				ShadingType type = shading->type;
				kh_assert(shading->format == fmt);
				OglShader *ogl_shader = ogl->shaders + shading->shader_index;
				ogl_shader->render_count = 0;
				ogl_shader->cmd_count = 0;
				ogl_shader->cmd_offset = cmd_total_count;
				ogl_shader->instance_count = 0;
				ogl_shader->offset = mat_offset;
				ogl->used_shaders[ogl->used_shaders_count++] = shading->shader_index;
				kh_lnext(Material, mat, shading->first) {
					b32 instance_has_entry = false;
					kh_lnext(MeshRenderer, meshr, mat->first) {
						u32 entry_count = meshr->entry_count;
						if(entry_count > 0 && meshr->loaded) {	
							kh_assert(meshr->loaded);
							instance_has_entry = true;
							u32 cmd_ind = cmd_total_count++;
							kh_assert(cmd_total_count < MAX_DRAWCMD_PER_FRAME);

							// u32 mat_instance_ind = ogl->mat_instance_count++;
							u32 mat_instance_ind = ogl_shader->instance_count;

							ogl_shader->cmd_count++;
							ogl_shader->render_count += entry_count;

							OglTriangleMesh *mesh = ogl45_get_triangle_mesh(ogl, render, assets, meshr->mesh, fmt);

							DrawElementsIndirectCommand *cmd = ogl->cmds + cmd_ind;
							cmd->primCount = entry_count;
							cmd->count = mesh->ind_count;
							cmd->firstIndex = mesh->ibo_offset;
							cmd->baseVertex = mesh->vbo_offset; 
							cmd->baseInstance = base_instance;
							base_instance += cmd->primCount;

							u32 entry_ind = meshr->first_entry;
							kh_lu0(entry_i, entry_count) {
								RenderEntry *entry = render->render_entries + entry_ind;

								ogl->map_boneoffset[total_entry_count] = 0;
								if(entry->bone_transform_offset != INVALID_U32_OFFSET) {
									kh_assert(has_skin(fmt));
									ogl->map_boneoffset[total_entry_count] = entry->bone_transform_offset + 1;	
								}
								OglTransform *map_tr = ogl->map_transforms + total_entry_count;
								map_tr->model = entry->tr;
								ogl->map_drawids[total_entry_count] = total_entry_count;
								ogl->map_indirectcmdids[total_entry_count] = mat_instance_ind;
								total_entry_count++;
								entry_ind = entry->next_in_mesh_renderer;
							}
						}
					}

					if(instance_has_entry && (shading->texture_count || shading->size)) {
						u32 offset = mat_offset;

						u8 *mat_data = (u8 *)(mat + 1);
						if(shading->texture_count) {
							offset = ogl45_send_mat_textures(api, render, assets, (AssetID *)mat_data, shading->texture_count,
							                                 offset, matbuffer);
						}
						glNamedBufferSubData(matbuffer, offset, shading->size, (u8 *)(mat_data + (shading->texture_count * sizeof(AssetID))));
						offset += shading->size;
						u32 size = KH_ALIGN_POW2(offset, ogl->texture_address_alignment);
						kh_assert((size - mat_offset) == ogl_shader->size);
						mat_offset += ogl_shader->size;
						kh_assert(mat_offset <= matbuffer_size);
						ogl_shader->instance_count++;
					}
				}
				mat_offset = KH_ALIGN_POW2(mat_offset, ogl->shader_storage_buffer_alignment);
			}
		}
		kh_assert(total_entry_count < MAX_ENTRIES);
		kh_assert(total_entry_count <= render->entry_count);
		glFinish();
		glNamedBufferSubData(ogl->cmds_buffer, 0, sizeof(DrawElementsIndirectCommand) * cmd_total_count, ogl->cmds);
	}
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
}

KH_INLINE void
ogl45_render_command(OglShader *mat) {
	u64 indirect = mat->cmd_offset * sizeof(DrawElementsIndirectCommand);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)indirect, mat->cmd_count, 0);
}

KH_INLINE void
ogl45_render_material(OglShader *mat) {
	glUseProgram(mat->prog_name);
	u64 indirect = mat->cmd_offset * sizeof(DrawElementsIndirectCommand);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void *)indirect, mat->cmd_count, 0);
}

// TODO(flo): IMPORTANT(flo) : SYNC our map buffer!
KH_INTERN void
ogl45_render(OglAPI *api, RenderManager *render, Assets *assets) {

	Ogl_4_5 *ogl = &api->ver_4_5;

	ogl45_update(api, render, assets);

	Camera *cam = &render->camera;
	mat4 vp = cam->view * cam->projection;

	OglCameraTransform *cam_tr = ogl->map_cam_transform;
	// @TODO(flo): refactoring our camera stuff to avoid debugging garbage datas for some hours~!
	cam_tr->viewproj = vp;
	cam_tr->view = cam->view;
	mat4 rot_view = cam->view;
	rot_view.m30 = 0;
	rot_view.m31 = 0;
	rot_view.m32 = 0;
	rot_view.m33 = 1;
	cam_tr->rot_vp = rot_view * cam->projection;
	cam_tr->pos = kh_vec4(cam->tr.pos, 1.0f);

	glFinish();

	u32 vertex_format = 0xFFFFFFFF;

	// TODO(flo): do not like this
	if(!ogl->has_skybox && render->has_skybox) {
		ogl45_set_skybox_test(api, ogl, render, assets);
		// ogl45_set_skybox(api, ogl, render, assets);
		ogl->has_skybox = true;
	}

	if(ogl->light_count < render->light_count) {
		u32 light_index = ogl->light_count++;
		Light *light = render->lights + light_index;
		ogl45_set_light(api, light, light_index);
	}

	// if(ogl->light_count > 0) {
	// 	Light *light = render->lights + 0;
	// 	ogl45_set_light(api, light, 0);
	// }

	if(render->entry_count > 0) {

		glEnable(GL_DEPTH_TEST);
		if(ogl->light_count > 0) {
			u32 light_index = 0;
			kh_assert(ogl->light_count == 1);
			OglLight *ogl_light = ogl->lights + light_index;
			Light *light = render->lights + light_index;
			ogl45_update_light(ogl_light, light, light_index);
			OglPass *shadow_pass = &ogl->pass[Pass_shadow];
			glBindFramebuffer(GL_FRAMEBUFFER, ogl_light->framebuffer);
			glViewport(0, 0, 4096, 4096);
			glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(shadow_pass->prog_name);
			// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, 0);
			// OglVertexBuffer *vert_buffer = g_state->vertex_buffers + 0;
			// glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);	
			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);
			for(u32 i = 0; i < ogl->used_shaders_count; ++i) {
				OglShader *ogl_shader = ogl->shaders + ogl->used_shaders[i];
				if(ogl_shader->render_count > 0) {
					if(vertex_format != (u32)ogl_shader->format) {
						vertex_format = (u32)ogl_shader->format;
						OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
						if(vert_buffer->skinned) {
							glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, vert_buffer->anim_offset,
							                   vert_buffer->attrib_stride);
						} else {
							glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, 0, 0);
						}
						glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);	
					}
					ogl45_render_command(ogl_shader);
				}
			}
		}

		if(api->zprepass_enabled) {
			glUseProgram(ogl->pass[Pass_zpre].prog_name);
			for(u32 i = 0; i < ogl->used_shaders_count; ++i) {
				OglShader *ogl_shader = ogl->shaders + ogl->used_shaders[i];
				if(ogl_shader->render_count > 0) {
					if(vertex_format != (u32)ogl_shader->format) {
						vertex_format = (u32)ogl_shader->format;
						OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
						glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);	
					}
					ogl45_render_command(ogl_shader);
				}
			}
			glDepthFunc(GL_LEQUAL);
		}
	}
	OglPass *scene_pass = &ogl->pass[Pass_render_3d_scene];
	glBindFramebuffer(GL_FRAMEBUFFER, scene_pass->framebuffer);
	glViewport(0,0,render->width,render->height);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if(api->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if(render->entry_count > 0) {
		if(ogl->has_skybox) {
			glBindTextureUnit(BIND_SKYBOX, ogl->irradiance_tex);
		}
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, scene_pass->matbuffer);
		// glDrawBuffer(GL_COLOR_ATTACHMENT1);
		vertex_format = 0xFFFFFFFF;
		u32 offset = 0;

		for(u32 i = 0; i < ogl->used_shaders_count; ++i) {
			OglShader *ogl_shader = ogl->shaders + ogl->used_shaders[i];
			if(ogl_shader->render_count > 0) {
				u32 size = ogl_shader->size * ogl_shader->instance_count; 
				if(size) {
					glBindBufferRange(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, scene_pass->matbuffer, ogl_shader->offset, size);
				}
				if(vertex_format != (u32)ogl_shader->format) {
					vertex_format = (u32)ogl_shader->format;
					OglVertexBuffer *vert_buffer = ogl->vertex_buffers + vertex_format;
					if(vert_buffer->skinned) {
						glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, vert_buffer->anim_offset, vert_buffer->attrib_stride);
					}
					glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, vert_buffer->attrib_offset, vert_buffer->attrib_stride);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);
				}
				ogl45_render_material(ogl_shader);
			}
		}	
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	if(api->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

#if 1
	if(ogl->has_skybox) {
		glBindTextureUnit(BIND_SKYBOX, ogl->skybox_tex);
		glDepthFunc(GL_LEQUAL);
		OglPass *skybox_pass = &ogl->pass[Pass_skybox];
		glUseProgram(ogl->pass[Pass_skybox].prog_name);
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, skybox_pass->vertexbuffer, 0, 3 * sizeof(GLfloat));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);
	}
#endif
	// cam_tr->viewproj = vp;

	// glEnable(GL_LINE_SMOOTH);
	// glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	// ogl_DEBUG_draw_axis(ogl->materials[Material_notexture].prog_name, kh_vec3(0,0,0), kh_vec3(1,0,0), kh_vec3(0,1,0), kh_vec3(0,0,1), 2.0f);

}

// TODO(flo): OGL exit