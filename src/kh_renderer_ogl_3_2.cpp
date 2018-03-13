GL_DEBUG_CALLBACK(ogl32_debug_callback) {
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
    	// char *err_mess = (char *)message;
    	// OutputDebugStringA(message);
    	// OutputDebugStringA("\n");
    }
}

KH_INTERN void
ogl32_check_requred_ogl_extensions(OglExtensionsHash *hash) {

}

KH_INTERN GLuint
ogl32_create_vert_frag_prog(OglAPI *api, char **files, u32 file_count, VertexFormat format = VertexFormat_count_or_none, 
                            MaterialType instance_type = Type_Material_none) {

}

KH_INTERN void
ogl_create_shader(RenderManager *render, OglAPI *api, ShadingType type, char **files, u32 file_count, 
                  VertexFormat format, MaterialType mat_type = Type_Material_none) {
	Ogl_3_2 *ogl = &api->ver_3_2;

	u32 shader_index = add_shading(render, type, format, &ogl->shader_count, ogl->shader_max_count);
	OglShader *ogl_shader = ogl->shaders + shader_index;

	kh_assert(format < VertexFormat_count_or_none);
	// if()
}

KH_INTERN void
ogl32_init(OglAPI *api, RenderManager *render, Assets *assets) {
	Ogl_3_2 *ogl = &api->ver_3_2;

	ogl32_check_requred_ogl_extensions(&api->exts);

	api->wireframe = false;
	api->bindless = false;
	api->sparse = false;
	api->multisample = false;
	api->multisample_count = 4;
	api->zprepass_enabled = false;

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(ogl32_debug_callback, 0);

	ogl->shader_max_count = 32;
	ogl->shader_count = 0;
	ogl->shaders = kh_push_array(api->arena, ogl->shader_max_count, OglShader);
}

KH_INTERN void
ogl32_render(OglAPI *api, RenderManager *render, Assets *assets) {
	Ogl_3_2 *ogl = &api->ver_3_2;
	kh_lnext(VertexBuffer, buffer, render->first_vertex_buffer) {
		// glVertexAttribPointer
		kh_lnext(Shading, shading, buffer->first) {
			// OglShader *shader = ogl->shaders + shading->shader_index;
			// glUseProgram(shader->prog_name);
			kh_lnext(Material, mat, shading->first) {
				// bind material ??? glGetUniformLocation??? how to auto this? GL_UNIFORM_BUFFER glUniformBlockBinding
				// glBindBufferBase allowed in gl3 with GL_UNIFORM_BUFFER??
				// kh_lu0(tex_i, shading->texture_count) {
				//  OglTexture2D *texture = ogl32_get_texture_2d();
				// 	glBindTexture(??);
				// }
				kh_lnext(MeshRenderer, meshr, mat->first) {
					u32 entry_count = meshr->entry_count;
					// OglTriangleMesh *mesh = ogl32_get_triangle_mesh();
					// glBindBuffer(GL_ARRAY_BUFFER);
					// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER);
					kh_lu0(entry_i, entry_count) {
						// glDrawXX();
					}	
				}
			}
		}
	}
}

KH_INTERN void
ogl32_display(OglAPI *api, u32 w, u32 h, void *pixels, b32 blit) {
	Ogl_3_2 *ogl = &api->ver_3_2;

}

KH_INTERN void
ogl32_reset(OglAPI *api) {
	Ogl_3_2 *ogl = &api->ver_3_2;

}