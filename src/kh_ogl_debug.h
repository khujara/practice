struct OglDebugPickEntity {
	GLuint pbo;
	GLuint framebuffer;
	GLuint prog;
	OglTexture2D texture;
	u32 *texture_memory;
	GLuint index_binding;
	GLuint wld_binding;
	GLuint vp_binding;
	GLuint bone_offset_binding;
	GLuint bones_binding;
};

struct OglColorProgram {
	GLuint name;
	GLuint color_loc;
	GLuint wld_mat_loc;
};

struct OglDebugImGui {
	b32 init;
	GLuint prog;
	OglTexture2D font_tex;
	GLuint tbo;
	GLuint texture_loc;
	GLuint vbo;
	GLuint ibo;
	GLuint vao;
	GLuint proj_mat_loc;
};

struct OglDebugState {
	GLuint notex_prog;
	GLuint tex_prog;

	OglColorProgram color_prog;

	GLuint line_buffer;

	OglDebugPickEntity entity_pick;

	GLuint hemisphere_vbo;
	GLuint hemisphere_ibo;
	GLuint hemisphere_indices_count;

	OglDebugImGui imgui;

	OglAPI *api;
};