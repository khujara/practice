// TODO(flo): CLEANUP(flo): IMPORTANT(flo): remove all references to ogl45

KH_INTERN void
ogl_debug_draw_texture(OglTexture2D *texture, u32 off_x, u32 off_y, u32 w, u32 h, b32 bindless) {
	glDisable(GL_DEPTH_TEST);
	glViewport(off_x, off_y, w, h);
	GLuint buffer;
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, sizeof(OglBindlessTextureAddress), 0, GL_STREAM_DRAW);
	if(bindless) {
		OglBindlessTextureAddress texaddr = {texture->bindless, (f32)texture->slice};
		glNamedBufferSubData(buffer, 0, sizeof(OglBindlessTextureAddress), &texaddr);
	} else {
		OglTextureAddress texaddr = {texture->container_id, (f32)texture->slice};
		glNamedBufferSubData(buffer, 0, sizeof(OglTextureAddress), &texaddr);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, buffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDeleteBuffers(1, &buffer);
	glEnable(GL_DEPTH_TEST);
}

// TODO(flo): the z-buffer should work even if we set multisample to true!
KH_INTERN void
ogl_debug_lines(OglDebugState *ogl_debug, GLuint buffer, u32 line_count, void *lines, u32 size) {
	glUseProgram(ogl_debug->notex_prog);
	glNamedBufferData(buffer, size, lines, GL_DYNAMIC_DRAW);
	glBindVertexBuffer(ATTRIB_VERTEX_DATA, buffer, 0, 6 * sizeof(GLfloat));
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, line_count);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

KH_INTERN void
ogl_debug_create_hemisphere(OglDebugState *ogl_debug) {
	glCreateBuffers(1, &ogl_debug->hemisphere_vbo);
	glCreateBuffers(1, &ogl_debug->hemisphere_ibo);
	const u32 MATRIX_SIZE = 8;
	const u32 size = MATRIX_SIZE - 1;
	const u32 NUM_VERTS = 4 * size * (size + 1) + ((size - 1) * (size - 1));

	v3 center = kh_vec3(0,0,0);
	v3 color = kh_vec3(1,0,0);

	v3 min = kh_vec3(-1, 0, -1);
	v3 max = kh_vec3(1, 1, 1);
	v3 d = (max - min) / (f32)size;

	v3 vertices[NUM_VERTS];
	u32 v = 0;
	for(u32 y = 0; y <= size; ++y) {
		f32 y_pos = min.y + (y * d.y);
		for(u32 x = 0; x <= size; ++x) {
			kh_assert(v < NUM_VERTS);
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)x * d.x), y_pos, min.z));
		}
		for(u32 z = 1; z < size; ++z) {
			kh_assert(v < NUM_VERTS);
			vertices[v++] = kh_normalize_v3(kh_vec3(max.x, y_pos, min.z + ((f32)z * d.z)));
		}
		for(i32 x = size; x >= 0; --x) {
			kh_assert(v < NUM_VERTS);
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)x * d.x), y_pos, max.z));
		}
		for(i32 z = (size - 1); z >= 1; --z) {
			kh_assert(v < NUM_VERTS);
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x, y_pos, min.z + ((f32)z * d.z)));
		}
	}
	u32 half_size = (size - 1) / 2;
	for(u32 i = 0; i < half_size; ++i) {
		i32 start = i + 1;
		i32 end = size - (i + 1);

		for(i32 x = start; x <= end; ++x) {
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)x * d.x), max.y, min.z + ((f32)start * d.z)));
		}
		for(i32 z = (start + 1); z < end; ++z) {
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)end * d.x), max.y, min.z + ((f32)z * d.z)));
		}
		for(i32 x = end; x >= start; --x) {
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)x * d.x), max.y, min.z + ((f32)end * d.z)));
		}
		for(i32 z = (end - 1); z >= (start + 1); --z) {
			vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)start * d.x), max.y, min.z + ((f32)z * d.z)));
		}
	}
	if(size % 2 == 0) {
		vertices[v++] = kh_normalize_v3(kh_vec3(min.x + ((f32)(half_size + 1) * d.x), max.y, min.z + ((f32)(half_size + 1) * d.z)));
	}
	kh_assert(v == NUM_VERTS);
	glNamedBufferData(ogl_debug->hemisphere_vbo, NUM_VERTS * sizeof(v3), (GLfloat *)vertices, GL_DYNAMIC_DRAW);

	u32 hash_indices[MATRIX_SIZE * MATRIX_SIZE];
	u32 c = 0;
	half_size = MATRIX_SIZE / 2;
	for(u32 i = 0; i < half_size; ++i) {
		i32 min_i = i;
		i32 max_i = size - i;
		for(i32 x = min_i; x <= max_i; ++x) {
			u32 ind = x + min_i * MATRIX_SIZE; 
			hash_indices[ind] = c++;
		}
		for(i32 z = (min_i + 1); z < max_i; ++z) {
			u32 ind = max_i + z * MATRIX_SIZE;
			hash_indices[ind] = c++;
		}
		for(i32 x = max_i; x >= min_i; --x) {
			u32 ind = x + max_i * MATRIX_SIZE;
			hash_indices[ind] = c++;
		}
		for(i32 z = (max_i - 1); z >= (min_i + 1); --z) {
			u32 ind = min_i + z * MATRIX_SIZE; 
			hash_indices[ind] = c++;
		}
	}
	if(size % 2 == 0) {
		u32 ind = half_size + half_size * MATRIX_SIZE;
		hash_indices[ind] = c++;
	}

	const u32 TOP_TRIANGLES = size * size * 2;
	const u32 SIDE_TRIANGLES = 4 * size * size * 2;
	const u32 NUM_TRIANGLES = SIDE_TRIANGLES + TOP_TRIANGLES;
	const u32 NUM_INDICES = NUM_TRIANGLES * 3;
	u32 indices[NUM_INDICES];
	u32 ind_count = 0;
	for(u32 i = 0; i < (SIDE_TRIANGLES / 2); ++i) {
		u32 ind_0, ind_1, ind_2, ind_3;
		if(((i + 1) % (4 * size)) == 0) {
			ind_0 = i; 
			ind_1 = i - (4 * size) + 1;
			ind_2 = i + 1;
			ind_3 = i + (4 * size);

		} else {
			ind_0 = i;	
			ind_1 = i + 1;
			ind_2 = i + (4 * size) + 1; 
			ind_3 = i + (4 * size);
		}
		indices[ind_count++] = ind_0;
		indices[ind_count++] = ind_1;
		indices[ind_count++] = ind_2;
		indices[ind_count++] = ind_0;
		indices[ind_count++] = ind_2;
		indices[ind_count++] = ind_3;
	}

	half_size = size / 2;
	f32 first_top_i = 4 * size * size;
	for(u32 i = 0; i < half_size; ++i) {
		i32 min_i = i;
		i32 max_i = size - i ;
		u32 hash_ind_0, hash_ind_1, hash_ind_2, hash_ind_3;
		for(i32 x = min_i; x < max_i; ++x) {
			hash_ind_0 = x + min_i * MATRIX_SIZE;
			hash_ind_1 = x + 1 + min_i * MATRIX_SIZE;
			hash_ind_2 = x + 1 + (min_i + 1) * MATRIX_SIZE;
			hash_ind_3 = x + (min_i + 1) * MATRIX_SIZE;
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_1];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_3];
		}
		for(i32 z = (min_i + 1); z < (max_i - 1); ++z) {
			hash_ind_0 = max_i + z * MATRIX_SIZE;
			hash_ind_1 = max_i + (z + 1) * MATRIX_SIZE;
			hash_ind_2 = (max_i - 1) + (z + 1) * MATRIX_SIZE;
			hash_ind_3 = (max_i - 1) + z * MATRIX_SIZE; 
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_1];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_3];
		}
		for(i32 x = max_i; x > min_i; --x) {
			hash_ind_0 = x + max_i * MATRIX_SIZE;
			hash_ind_1 = x - 1 + max_i * MATRIX_SIZE;
			hash_ind_2 = x - 1 + (max_i - 1) * MATRIX_SIZE;
			hash_ind_3 = x + (max_i - 1) * MATRIX_SIZE;
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_1];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_3];
		}
		for(i32 z = (max_i - 1); z > (min_i + 1); --z) {
			hash_ind_0 = min_i + z * MATRIX_SIZE;
			hash_ind_1 = min_i + (z - 1) * MATRIX_SIZE;
			hash_ind_2 = (min_i + 1) + (z - 1) * MATRIX_SIZE;
			hash_ind_3 = (min_i + 1) + z * MATRIX_SIZE; 
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_1];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
			indices[ind_count++] = first_top_i + hash_indices[hash_ind_3];
		}
	}
	if(size % 2 != 0) {
		u32 hash_ind_0 = half_size + half_size * MATRIX_SIZE;
		u32 hash_ind_1 = (half_size + 1) + half_size * MATRIX_SIZE;
		u32 hash_ind_2 = (half_size + 1) + (half_size + 1) * MATRIX_SIZE;
		u32 hash_ind_3 = half_size + (half_size + 1) * MATRIX_SIZE;

		indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
		indices[ind_count++] = first_top_i + hash_indices[hash_ind_1];
		indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
		indices[ind_count++] = first_top_i + hash_indices[hash_ind_0];
		indices[ind_count++] = first_top_i + hash_indices[hash_ind_2];
		indices[ind_count++] = first_top_i + hash_indices[hash_ind_3];
	}

	kh_assert(ind_count == NUM_INDICES);
	ogl_debug->hemisphere_indices_count = NUM_INDICES;
	glNamedBufferData(ogl_debug->hemisphere_ibo, NUM_INDICES * sizeof(u32), indices, GL_DYNAMIC_DRAW);
}

KH_INTERN void
ogl_debug_init(OglDebugState *ogl_debug, OglAPI *api, RenderManager *render) {
	// TODO(flo): remove this
	Ogl_4_5 *ogl = &api->ver_4_5; 

	ogl_debug->api = api;

	glCreateBuffers(1, &ogl_debug->line_buffer);
	char *notex_files[] = {"shaders/gl450/debug/notexture.shader"};
	ogl_debug->notex_prog = api->create_vert_frag_prog(api, notex_files, array_count(notex_files), VertexFormat_count_or_none, Type_Material_none);

	char *tex_files[] = {"shaders/gl450/debug/texture.shader"};
	ogl_debug->tex_prog = api->create_vert_frag_prog(api, tex_files, array_count(tex_files), VertexFormat_count_or_none, Type_Material_T1);
	ogl_debug->entity_pick = {};
	OglDebugPickEntity *pick = &ogl_debug->entity_pick;

	OglTexture2DContainer *container = ogl45_get_texture_container(api, render->width, render->height, 1,  GL_R32UI);
	// OglTexture2DContainer *container = ogl45_get_texture_container(api, render->width, render->height, 1,  GL_RGBA8);
	ogl45_add_texture_to_container(container, &pick->texture);
	GLuint rb;
	glCreateFramebuffers(1, &pick->framebuffer);
	glCreateRenderbuffers(1, &rb);
	glNamedFramebufferTextureLayer(pick->framebuffer, GL_COLOR_ATTACHMENT0, 
	                               pick->texture.name, 0, pick->texture.slice);
	glNamedRenderbufferStorage(rb, GL_DEPTH_COMPONENT, render->width, render->height);
	glNamedFramebufferRenderbuffer(pick->framebuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);

	u32 flags = GL_MAP_READ_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT;
	glCreateBuffers(1, &pick->pbo);
	glNamedBufferStorage(pick->pbo, render->width * render->height * sizeof(u32), NULL, GL_MAP_READ_BIT|GL_CLIENT_STORAGE_BIT);

	char *ind_files[] = {"shaders/gl450/debug/entity_index.shader"};
	pick->prog = api->create_vert_frag_prog(api, ind_files, array_count(ind_files), VertexFormat_count_or_none, Type_Material_none);

	pick->index_binding       = glGetUniformLocation(pick->prog, "uni_index");
	pick->wld_binding         = glGetUniformLocation(pick->prog, "wld");
	pick->vp_binding          = glGetUniformLocation(pick->prog, "vp");
	pick->bone_offset_binding = glGetUniformLocation(pick->prog, "bone_offset");
	pick->bones_binding       = glGetUniformLocation(pick->prog, "bones");
	// u32 flags = GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT;//| GL_MAP_READ_BIT 
	// u32 create_flags = flags | GL_DYNAMIC_STORAGE_BIT;
	// pick->texture_memory = (u32 *)glMapNamedBufferRange(pick->pbo, 0, render->width * render->height * sizeof(u32), flags); 
	pick->texture_memory      = (u32 *)VirtualAlloc(0, 1920*1080*sizeof(u32), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	OglColorProgram *color = &ogl_debug->color_prog;
	char *col_files[] = {"shaders/gl450/debug/debug_color.shader"};
	color->name = api->create_vert_frag_prog(api, col_files, array_count(col_files), VertexFormat_count_or_none, Type_Material_none);
	color->color_loc = glGetUniformLocation(color->name, "color");
	color->wld_mat_loc = glGetUniformLocation(color->name, "wld_mat");

	ogl_debug_create_hemisphere(ogl_debug);

	OglDebugImGui *imgui = &ogl_debug->imgui;
	imgui->init = false;
	char *imgui_files[] = {"shaders/gl450/debug/imgui.shader"};
	imgui->prog = api->create_vert_frag_prog(api, imgui_files, arr_len(imgui_files), VertexFormat_count_or_none, Type_Material_T1);
}

KH_INTERN void
ogl_debug_draw(OglDebugState *ogl_debug, DebugState *debug, RenderManager *render, b32 blit) {

	OglAPI *api = ogl_debug->api;
	// TODO(flo): remove this
	Ogl_4_5 *ogl = &api->ver_4_5;

	Input *input = debug->input;

	api->target_w = render->width;
	api->target_h = render->height;
	Camera *cam = &render->camera;
	mat4 vp = cam->view * cam->projection;
	mat4 view = cam->view;
	view.c3 = kh_vec4(0,0,0,1);
	OglCameraTransform *cam_tr = ogl->map_cam_transform;
	cam_tr->viewproj = vp;
	cam_tr->view = view * cam->projection;
	cam_tr->pos = kh_vec4(cam->tr.pos, 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	ogl_debug_lines(ogl_debug, ogl_debug->line_buffer, debug->lines.count, debug->lines.data, debug->lines.count * sizeof(DebugLine));
	if(debug->hemispheres.count > 0) {
		OglColorProgram *color_prog = &ogl_debug->color_prog;
		glUseProgram(color_prog->name);	
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, ogl_debug->hemisphere_vbo, 0, 3 * sizeof(GLfloat));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl_debug->hemisphere_ibo);
		for(u32 i = 0; i < debug->hemispheres.count; ++i) {
			DebugHemisphere *hemisphere = debug->hemispheres.data + i;
			glUniform3fv(color_prog->color_loc, 1, hemisphere->color.e); 
			glUniformMatrix4fv(color_prog->wld_mat_loc, 1, GL_FALSE, (GLfloat *)&hemisphere->tr); 
			glDrawElements(GL_TRIANGLES, ogl_debug->hemisphere_indices_count, GL_UNSIGNED_INT, 0);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}	

	OglDebugPickEntity *pick = &ogl_debug->entity_pick;

	b32 ask_for_entity = debug->check_entity_on_click && was_pressed(debug->input->mouse_buttons[MouseButton_left]);;
	if(!debug->input->modifier_down[Modifier_ctrl]) {
		ask_for_entity = false;
	}
	// ask_for_entity &= debug->input->modifier_down[Modifier_ctrl];
	if(ask_for_entity && render->entry_count > 0) {
		glUseProgram(pick->prog);
		glBindFramebuffer(GL_FRAMEBUFFER, pick->framebuffer);
		GLint old_viewport[4];
		glGetIntegerv(GL_VIEWPORT, old_viewport);
		glViewport(0, 0, render->width, render->height);
		u32 val = 0;
		// u32 val = 0xFFFFFFFF;
		glClearBufferuiv(GL_COLOR, 0, &val); 
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Camera *cam = &render->camera;
		// mat4 vp = cam->view * cam->projection;
		glUniformMatrix4fv(pick->vp_binding, 1, GL_FALSE, (GLfloat *)&vp);


		u32 offset = 0;
		u32 total_entry_count = 0;
		for(VertexBuffer *buffer = render->first_vertex_buffer; buffer; buffer = buffer->next) {
			VertexFormat fmt = buffer->format;
			OglVertexBuffer *vert_buffer = ogl->vertex_buffers + fmt;
			if(vert_buffer->skinned) {
				glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, vert_buffer->anim_offset, vert_buffer->attrib_stride);
			} else {
				glBindVertexBuffer(ATTRIB_ANIMATION, vert_buffer->verts.name, 0, 0);
			}
			glBindVertexBuffer(ATTRIB_VERTEX_DATA, vert_buffer->verts.name, 
			                   vert_buffer->attrib_offset, vert_buffer->attrib_stride);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vert_buffer->inds.name);
			for(Shading *shading = buffer->first; shading; shading = shading->next) {
				for(Material *mat = shading->first; mat; mat = mat->next) {
					for(MeshRenderer *meshr = mat->first; meshr; meshr = meshr->next) {
						u32 entry_count = meshr->entry_count;
						if(entry_count > 0) {
							u32 entry_ind = meshr->first_entry;
							OglTriangleMesh *mesh = ogl45_get_triangle_mesh(ogl, render, debug->assets, meshr->mesh, fmt);
							for(u32 i = 0; i < entry_count; ++i) {
								RenderEntry *entry = render->render_entries + entry_ind;
								if(entry->bone_transform_offset != INVALID_U32_OFFSET) {
									kh_assert(has_skin(fmt));
									glUniform1ui(pick->bone_offset_binding, entry->bone_transform_offset + 1);
								} else {
									glUniform1ui(pick->bone_offset_binding, 0);
								}
								glUniform1ui(pick->index_binding, entry->scene_index);
								glUniformMatrix4fv(pick->wld_binding, 1, GL_FALSE, (GLfloat *)&entry->tr); 
								glDrawElementsBaseVertex(GL_TRIANGLES, mesh->ind_count, GL_UNSIGNED_INT, 
								                         (void *)(sizeof(u32) * mesh->ibo_offset), mesh->vbo_offset);
								entry_ind = entry->next_in_mesh_renderer;
							}
						}
					}
				}
			}
		}
		glReadPixels(0, 0, render->width, render->height, GL_RED_INTEGER, GL_UNSIGNED_INT, pick->texture_memory);
		u32 pick_x = (u32)(input->mouse_rx * (f32)render->width);
		u32 pick_y = kh_min_u32((u32)(input->mouse_ry * (f32)(render->height)), render->height - 1);
		u32 *pixels = pick->texture_memory + pick_x + pick_y * render->width;
		// if(*pixels != 0) {
		debug->last_entity_index = (*pixels);
		// }
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
	}

	OglDebugImGui *imgui = &ogl_debug->imgui;
	if(!imgui->init) {
		Texture2D tex;
		tex.width = debug->imgui_font_w;
		tex.height = debug->imgui_font_h;
		tex.bytes_per_pixel = 4;

		OglTextureParam param = ogl_default_texture_param();
		param.swizzle = true;
		for(u32 i = 0; i < array_count(param.swizzles); ++i) {
			param.swizzles[i] = GL_RED;
		}
		OglTexture2DContainer *font_container = ogl45_get_texture_container(api, tex.width, tex.height, 1, GL_R8, param);
		ogl45_add_texture_2d_to_container(font_container, &imgui->font_tex, &tex, debug->imgui_font_px, GL_RED);

		debug->imgui_font_id = (void *)(umm)imgui->font_tex.name;
		imgui->init = true;
		glCreateBuffers(1, &imgui->vbo);
		glCreateBuffers(1, &imgui->ibo);
		glCreateBuffers(1, &imgui->tbo);
		glNamedBufferData(imgui->tbo, sizeof(OglBindlessTextureAddress), 0, GL_STATIC_DRAW);
		if(api->bindless) {
			OglBindlessTextureAddress texaddr = {imgui->font_tex.bindless, (f32)imgui->font_tex.slice};
			glNamedBufferSubData(imgui->tbo, 0, sizeof(OglBindlessTextureAddress), &texaddr);
		} else {
			OglTextureAddress texaddr = {imgui->font_tex.container_id, (f32)imgui->font_tex.slice};
			glNamedBufferSubData(imgui->tbo, 0, sizeof(OglTextureAddress), &texaddr);
		}
		imgui->texture_loc = glGetUniformLocation(imgui->prog, "tex_sampler");
		imgui->proj_mat_loc = glGetUniformLocation(imgui->prog, "proj_mat");

		GLint last_vao;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
		glCreateVertexArrays(1, &imgui->vao);	
		glBindVertexArray(imgui->vao);
		glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, KH_OFFSETOF(ImDrawVert,pos));
		glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, KH_OFFSETOF(ImDrawVert,uv));
		glVertexAttribFormat(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, KH_OFFSETOF(ImDrawVert,col));

		glVertexAttribBinding(0, 0);
		glVertexAttribBinding(1, 0);
		glVertexAttribBinding(2, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glBindVertexArray(last_vao);

	} else {
		ImDrawData *draw_data = debug->imgui_data;

		if(draw_data) {
			glEnable(GL_BLEND);
	    // glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
	    // glEnable(GL_SCISSOR_TEST);

			GLint last_vao;
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
			glBindVertexArray(imgui->vao);	
			glUseProgram(imgui->prog);



		// glBindTexture(GL_TEXTURE_2D, imgui->font_tex.name);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, imgui->tbo);
			const float ortho_projection[4][4] =
			{
				{ 2.0f/(f32)api->target_w, 	0.0f,                   	0.0f, 0.0f },
				{ 0.0f,                  	2.0f/-(f32)api->target_h, 	0.0f, 0.0f },
				{ 0.0f,                  	0.0f,                       -1.0f, 0.0f },
				{-1.0f,                  	1.0f,                   	0.0f, 1.0f },
			};
			glUniform1i(imgui->texture_loc, 0);
			glUniformMatrix4fv(imgui->proj_mat_loc, 1, GL_FALSE, (GLfloat *)ortho_projection);

			kh_ls0(i, draw_data->CmdListsCount) {
				const ImDrawList *cmd_list = draw_data->CmdLists[i];
				const ImDrawIdx *idx_buffer_offset = 0;

				glNamedBufferData(imgui->vbo, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), 
				                  (const GLvoid *)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
				glNamedBufferData(imgui->ibo, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), 
				                  (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);
				glBindVertexBuffer(0, imgui->vbo, 0, sizeof(ImDrawVert));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imgui->ibo);

				// GLuint this_tex = imgui->font_tex.name;
				// GLuint last_tex = this_tex;
				kh_ls0(cmd_i, cmd_list->CmdBuffer.Size) {
					const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];

					// this_tex = (GLuint)(umm)pcmd->TextureId;
					// if(this_tex != last_tex) {
					// 	NOT_IMPLEMENTED;
					// 	last_tex = this_tex;
					// }
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
					idx_buffer_offset += pcmd->ElemCount;
				}
			}
			glBindVertexArray(last_vao);	
			glUseProgram(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);

		}
	}

	if(ogl->light_count > 0) {
		glBindVertexBuffer(ATTRIB_VERTEX_DATA, ogl->pass[Pass_blit].vertexbuffer, 0, 3 * sizeof(GLfloat)); 
		glUseProgram(ogl->pass[Pass_blit].prog_name);
		OglLight *light = ogl->lights + 0;
		ogl_debug_draw_texture(&light->shadow_tex, 0, 0, 200, 200, api->bindless);
	}
	// ogl_debug_draw_texture(&ogl_debug->imgui.font_tex, 0, 0, 200, 200, api->bindless);

#if 0
	struct Vertex_PC {
		v3 pos;
		v3 color;
	};
	const u32 NUM_TRIANGLES = 50;	
	f32 r = 1.0f;

	GLuint buffer;
	GLuint tbo;
	glCreateBuffers(1, &buffer);
	glCreateBuffers(1, &tbo);
	glNamedBufferData(tbo, sizeof(OglBindlessTextureAddress), 0, GL_STATIC_DRAW);
	AssetID tex_id = get_first_asset(debug->assets, AssetName_randy_albedo);
	if(asset_loaded(debug->assets, tex_id)) {
		OglTexture2D *texture = ogl45_get_texture_2d(api, render, debug->assets, tex_id);
		if(api->bindless) {
			OglBindlessTextureAddress texaddr = {texture->bindless, (f32)texture->slice};
			glNamedBufferSubData(tbo, 0, sizeof(OglBindlessTextureAddress), &texaddr);
		} else {
			OglTextureAddress texaddr = {texture->container_id, (f32)texture->slice};
			glNamedBufferSubData(tbo, 0, sizeof(OglTextureAddress), &texaddr);
		}
	}

	// glUseProgram(ogl_debug->tex_proj_prog);
	glUseProgram(ogl_debug->tex_prog);
	glDisable(GL_DEPTH_TEST);
	glNamedBufferData(buffer, sizeof(Vertex_PC) * NUM_TRIANGLES * 3, 0, GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIND_MATERIAL, tbo);
	glBindVertexBuffer(ATTRIB_VERTEX_DATA, buffer, 0, 6 * sizeof(GLfloat));
	f32 dtheta = TAU32 / NUM_TRIANGLES;
	v3 center = kh_vec3(0,0,0);
	v3 color = kh_vec3(1,0,0);

	mat4 vp = render->camera.view * render->camera.projection;

	u32 offset = 0;
	for(u32 i = 0; i < (NUM_TRIANGLES ); ++i) {
		f32 theta0 = dtheta * i;
		f32 theta1 = dtheta * (i + 1);

		f32 c0 = kh_cos_f32(theta0);
		f32 c1 = kh_cos_f32(theta1);
		f32 s0 = kh_sin_f32(theta0);
		f32 s1 = kh_sin_f32(theta1);

		Vertex_PC vertices[3];
		v4 p0 = vp * kh_vec4(center, 1.0f);
		v4 p1 = vp * kh_vec4(center + r * kh_vec3(c0, s0, 0), 1.0f);
		v4 p2 = vp * kh_vec4(center + r * kh_vec3(c1, s1, 0), 1.0f);
		p0 = p0 / p0.w;
		p1 = p1 / p1.w;
		p2 = p2 / p2.w;

		vertices[0].pos = kh_vec3(p0);
		vertices[1].pos = kh_vec3(p1);
		vertices[2].pos = kh_vec3(p2);
		// vertices[0].pos = center;
		// vertices[1].pos = center + r * kh_vec3(c0, s0, 0);
		// vertices[2].pos = center + r * kh_vec3(c1, s1, 0);

		// vertices[0].color = color;
		// vertices[1].color = color;
		// vertices[2].color = color;
		vertices[0].color = kh_vec3(0.5f, 0.5f, 0.0f);
		vertices[1].color = kh_vec3(0.5f*c0+0.5f, 0.5f*s0+0.5f, 0.0f);
		vertices[2].color = kh_vec3(0.5f*c1+0.5f, 0.5f*s1+0.5f, 0.0f);

		glNamedBufferSubData(buffer, offset, sizeof(Vertex_PC) * 3, vertices);
		offset += sizeof(Vertex_PC) * 3;


	}
	glDrawArrays(GL_TRIANGLES, 0, NUM_TRIANGLES * 3);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(0);
	glDeleteBuffers(1, &tbo);
	glDeleteBuffers(1, &buffer);
#endif
}