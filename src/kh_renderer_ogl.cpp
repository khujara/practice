KH_INTERN void
ogl_add_ext_to_hash(OglExtensionsHash *hash, char *name, LinearArena *arena) {

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

	OglExtension *app = kh_push_struct(arena, OglExtension);

	// TODO(flo): store the hash val instead of the name and check that it is unique in our hash??
	u32 str_len = string_length(name);
	app->name = (char *)kh_push(arena, str_len + 1);
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
ogl_set_extensions_hash(OglExtensionsHash *hash, LinearArena *arena) {
	GLint num_ext;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	hash->empty = array_count(hash->hash) - 1;
	hash->collision_count = 0;

	for(GLint i = 0; i < num_ext; ++i) {
		char *ext = (char *)glGetStringi(GL_EXTENSIONS, i);
		ogl_add_ext_to_hash(hash, ext, arena);
	}
}

KH_INTERN void
ogl_check_ext_list(OglExtensionsHash *hash, char **name, u32 count) {
	kh_lu0(i, count) {
		char *ext = *(name + i);
		kh_assert(ogl_check_ext(hash, ext));
	}
}

KH_INTERN GLchar *
ogl_load_shader_from_file(char *header, u32 header_size, char *filename, LinearArena *arena) {
	FileHandle shader = g_platform.open_file(filename, FileAccess_read, FileCreation_only_open);
	kh_assert(!shader.error);
	u32 size = g_platform.get_file_size(&shader);

	u32 total_size = header_size + size + 1;

	GLchar *res = (GLchar *)kh_push(arena, total_size);

	strings_copy_NNT(header_size, header, (char *)res);

	u8 *dst = (u8 *)res + header_size; 

	g_platform.read_bytes_of_file(&shader, 0, size, dst);
	res[total_size - 1] = '\0';
	g_platform.close_file(&shader);
	
	return(res);
}

KH_INTERN GLuint
ogl_compile_shader(GLenum type, GLsizei count, char *src, GLint *length) {
	GLuint res = glCreateShader(type);
	glShaderSource(res, count, &src, length);
	glCompileShader(res);
	return(res);
}

KH_INLINE OglTextureParam 
ogl_default_texture_param() {
	OglTextureParam param;
	param.filter = GL_LINEAR;
	param.wrap = GL_CLAMP_TO_EDGE;
	param.swizzle = false;
	return(param);
}

// TODO(flo): maybe we should assert that they have the same size?
KH_INLINE u32
ogl_get_texture_address_size(b32 bindless) {
	u32 res = (bindless) ? sizeof(OglBindlessTextureAddress) : sizeof(OglTextureAddress);
	return(res);
}

KH_INLINE u32
ogl_get_texture_address_alignment(b32 bindless) {
	u32 res;
	if(bindless) {
		res = KH_SIZEOF(OglBindlessTextureAddress, handle);
	} else {
		res = KH_SIZEOF(OglTextureAddress, container);
	}
	return(res);
}

#ifdef WIN32
KH_INTERN void
DEBUG_ogl_get_shader_log(GLuint shader) {
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		GLchar info_log[4096];
		glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
		kh_assert(!"shader compilation failed");
	}
}

KH_INTERN void
DEBUG_ogl_get_prog_log(GLuint prog) {
	GLint success;
	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if(!success) {
		GLchar info_log[4096];
		glGetProgramInfoLog(prog, sizeof(info_log), NULL, info_log);
		OutputDebugStringA(info_log);
		kh_assert(!"program linking failed");
	}
}
#else
#define DEBUG_ogl_get_shader_log(...)
#define DEBUG_ogl_get_prog_log(...)
#endif