#ifdef KH_IN_DEVELOPMENT
struct LevelEditor {
	DebugState *debug;

	b32 scene_wnd;
	b32 assets_wnd;
	b32 material_wnd;
	b32 choose_tex_wnd;
	b32 mat_list_wnd;

	AssetID *cur_asset;
	Material *cur_material;

	RenderManager *render;
	Assets *assets;
	Scene *cur_scene;
};

KH_INTERN LevelEditor
get_level_editor(DebugState *debug, RenderManager *render, Assets *assets, Scene *scene) {
	LevelEditor res = {};
	res.debug = debug;
	res.render = render;
	res.assets = assets;
	res.cur_scene = scene;
	res.assets_wnd = false;
	res.scene_wnd = true;
	res.material_wnd = false;
	res.choose_tex_wnd = false;
	res.mat_list_wnd = false;
	res.cur_material = 0;
	return(res);
} 

KH_INTERN void
edit_material(LevelEditor *editor, Material *mat, MaterialType type) {
	kh_assert(mat);
	Assets *assets = editor->assets;
	DebugState *debug = editor->debug;
	char *str = G_MATERIAL_TYPES[type];
	char *at = str; 
	char *prefix = "Type_Material_";
	u32 prefix_len = string_length(prefix);
	kh_lu0(i, prefix_len) {
		kh_assert(*at++ == prefix[i]);
	}
	kh_assert(str[prefix_len] != '\0');
	at = str + prefix_len;

	u8 *mat_at = (u8 *)(mat + 1);
	b32 state = 0;
	char cur_type = 0;
	char debug_txt[256];
	u32 el_count = 0;
	for(char *c = at; *c; ++c) {
		if(state == 0) {
			state = 1;
			cur_type = *c;
		} else {
			u32 count = *c - '0';
			// TODO(flo): show edit
			switch(cur_type) {
				case 'T' : {
					kh_lu0(i, count) {
						AssetID *id = (AssetID *)mat_at;
						Asset *asset = assets->arr + id->val;
						if(id->val < assets->count_from_package) {
							kh_printf(debug_txt, "%s", asset->name);
						} else {
							kh_printf(debug_txt, "%s", asset->cname);
						}
						if(debug_button(debug, debug_txt)) {
							editor->cur_asset = id;
							editor->choose_tex_wnd = true;
						}
						mat_at += sizeof(AssetID);
						el_count++;
					}
				} break;
				case 'F' : {
					kh_lu0(i, count) {
						f32 *val = (f32 *)mat_at;
						// kh_printf(debug_txt, "%f", *val);
						kh_printf(debug_txt, "%u", el_count);
						debug_input_f32(debug, debug_txt, 0.0f, 100.0f, val);
						mat_at += sizeof(f32);
						el_count++;
					}
				} break;
				case 'S' : {
					kh_lu0(i, count) {
						i32 *val = (i32 *)mat_at;
						// kh_printf(debug_txt, "%i", *val);
						kh_printf(debug_txt, "%u", el_count);
						debug_input_i32(debug, debug_txt, 0, 100, val);
						mat_at += sizeof(s32);
						el_count++;
					}
				} break;
				case 'U' : {
					kh_lu0(i, count) {
						u32 *val = (u32 *)mat_at;
						kh_printf(debug_txt, "%u", *val);
						debug_text(debug, debug_txt);
						mat_at += sizeof(u32);
						el_count++;
					}
				} break;
				INVALID_DEFAULT_CASE;
			}	
			state = 0;
		}
	}
	if(editor->choose_tex_wnd && editor->cur_asset) {
		debug_begin_wnd(debug, "Choose Texture", &editor->choose_tex_wnd);
		kh_lu0(assetname_i, AssetName_count) {
			AssetNameArray *name = assets->name_arr + assetname_i;
			kh_lu(asset_i, name->first_asset, name->one_past_last_asset) {
				u32 id = asset_i;
				Asset *asset = assets->arr + id;
				if(asset->source.type.key == AssetType_tex2d) {
					if(debug_button(debug, G_ASSET_NAMES[assetname_i])) {
						if(id != editor->cur_asset->val) {
							for(MeshRenderer *meshr = mat->first; meshr; meshr = meshr->next) {
								meshr->loaded = false;
							}
							editor->cur_asset->val = id;
							editor->choose_tex_wnd = false;
							editor->cur_asset = 0;
						}
					}
				}
			}
		}

		debug_end_wnd(debug);
	}
}

/*TODO(flo): Editor features
	Scene :
		highlight selected entity in the 3d scene
		We can change the meshr id of the entity by display available meshr for this entity in a new window (select and validate to change)

	add a list of available mesh renderer so we can choose what mesh renderer we should use when we're adding new entities
	DON'T FORGET ANIMATION/ANIMATOR!!!! 
	DON'T FORGET ANIMATION/ANIMATOR!!!! 
	DON'T FORGET ANIMATION/ANIMATOR!!!! 
	DON'T FORGET ANIMATION/ANIMATOR!!!! 
*/
KH_INTERN void
edit_level(LevelEditor *editor) {
	u32 button_count = 0;
	DebugState *debug = editor->debug;
	RenderManager *render = editor->render;
	Assets *assets = editor->assets;
	Scene *scene = editor->cur_scene;	
	b32 show = true;
	debug_begin_wnd(debug, "Editor", 0);
	char entity_txt[64];
	kh_printf(entity_txt, "selected entity %u", debug->last_entity_index);
	debug_text(debug, entity_txt);
	if(debug_button(debug, "Show/Hide Assets")) {
		editor->assets_wnd = !editor->assets_wnd;
	}
	if(debug_button(debug, "Show/Hide Scene")) {
		editor->scene_wnd = !editor->scene_wnd;
	}
	if(debug_button(debug, "Show/Hide Materials")) {
		editor->mat_list_wnd = !editor->mat_list_wnd;
	}
	debug_end_wnd(debug);

	if(editor->assets_wnd) {
		debug_begin_wnd(debug, "Assets", &editor->assets_wnd);	
		for(u32 assetname_i = 0; assetname_i < AssetName_count; ++assetname_i) {
			AssetNameArray *name = assets->name_arr + assetname_i;
			debug_text(debug, G_ASSET_NAMES[assetname_i]);
			kh_lu(asset_i, name->first_asset, name->one_past_last_asset) {
				u32 id = asset_i;
				Asset *asset = assets->arr + id;
				SourceAsset asset_src = asset->source;
				debug_text(debug, G_ASSET_TYPES[asset_src.type.key]);
			}
		}	
		debug_end_wnd(debug);
	}
	if(editor->scene_wnd && scene) {
		debug_begin_wnd(debug, "Scene", &editor->scene_wnd);
		// TODO(flo): display add and remove entities
		// set values for entities (transform, mesh renderer, precalc)
		char current_txt[128];
		u32 count = 0;
		kh_lu0(entity_i, scene->pool_count) {
			SceneEntity *scene_entity = scene->entity_pool + entity_i;			
			b32 selected = false;
			if(!scene_entity->next_free) {
				if(debug->last_entity_index != 0) {
					if(debug->last_entity_index - 1 == entity_i) {
						selected = true;
					}
				}
				Entity entity = scene_entity->entity;
				MeshRenderer *meshr = get_entity_meshr(render, &entity);
				Material *mat = get_entity_material(render, &entity);
				if(mat == editor->cur_material) {
					debug_push_color(debug, ImGuiCol_Text, kh_vec4(1,0,0,1));
				}
				Shading *shading = get_entity_shading(render, &entity);
				Asset *asset = assets->arr + meshr->mesh.val;
				char *mesh_str = 0;
				if(meshr->mesh.val < assets->count_from_package) {
					mesh_str = G_ASSET_NAMES[asset->name];
				} else {
					mesh_str = asset->cname;
				}
				if(selected) {
					if(mat == editor->cur_material) {
						debug_pop_color(debug);
						debug_push_color(debug, ImGuiCol_Text, kh_vec4(1, 0.5f, 0, 1));
					} else {
						debug_push_color(debug, ImGuiCol_Text, kh_vec4(1,1,0,1));
					}
				}

				kh_printf(current_txt, "Entity %u (%s)", count++, mesh_str);
				if(debug_collapse(debug, current_txt)) {
					if(selected || mat == editor->cur_material) {
						debug_pop_color(debug);
					}
					debug_text(debug, G_VERTEX_FORMATS[shading->format]);
					debug_text(debug, G_SHADING_TYPES[shading->type]);
					debug_text(debug, G_MATERIAL_TYPES[shading->mat_type]);
					// TODO(flo): edit rotations!
					v3 entity_pos = kh_vec3(entity.transform.c3);
					kh_lu0(pos_i, 3) {
						kh_printf(current_txt, "%u", entity_i * 3 + pos_i);
						debug_input_f32(debug, current_txt, -100.0f, 100.0f, &scene_entity->entity.transform.c3.e[pos_i]);
					}
					kh_printf(current_txt, "Edit (%u)", entity.mesh_renderer);
					if(debug_button(debug, current_txt)) {
						editor->cur_material = mat;	
						editor->material_wnd = true;
					}
				} else {
					if(selected || mat == editor->cur_material) {
						debug_pop_color(debug);
					}
				}
			}
		}
		debug_end_wnd(debug);
	}

	if(editor->mat_list_wnd) {
		debug_begin_wnd(debug, "Materials", &editor->mat_list_wnd);
		char txt[32];
		u32 mat_count = 0;
		kh_lnext(VertexBuffer, buffer, render->first_vertex_buffer) {
			kh_lnext(Shading, shading, buffer->first) {
				kh_lnext(Material, mat, shading->first) {
					kh_printf(txt, "Material %u", mat_count++);
					if(debug_button(debug, txt)) {
						editor->cur_material = mat;
						editor->material_wnd = true;
					}
				}	
			}

		}
		debug_end_wnd(debug);
	}
	if(editor->material_wnd && editor->cur_material) {
		debug_begin_wnd(debug, "Edit Material", &editor->material_wnd);
		// TODO(flo): show a window with the texture when we click on it
		// TODO(flo): can edit the material
		edit_material(editor, editor->cur_material, get_shading_for_material(render, editor->cur_material)->mat_type);
		debug_end_wnd(debug);
		// if(debug_collapse(debug, "Add Material")) {
			// VertexFormat chosen_fmt;
			// ShadingType chosen_shading;
			// MaterialType chosen_mat;
		// }
	}

	// TODO(flo): show a wnd with the list of mesh renderer and the possibility to add entity to selected one
}

KH_INTERN void
save_scene(RenderManager *render, Assets *assets, char *filename) {

	// TODO(flo): Animator component
	FileHandle file = g_platform.open_file(filename, FileAccess_write, FileCreation_override);	
	kh_assert(!file.error);

	u32 total_tri_count = 0;
	u32 total_entity_count = 0;
	u64 file_offset = 0;
	char str_buff[4096];
	const u32 count_reserved_size = 16;
	for(u32 i = 0; i < count_reserved_size; ++i) {
		str_buff[i] = ' ';
	}
	str_buff[count_reserved_size - 1] = '\n';
	file_offset += g_platform.write_bytes_to_file(&file, file_offset, count_reserved_size, str_buff);	
	file_offset += g_platform.write_bytes_to_file(&file, file_offset, count_reserved_size, str_buff);	

	// TODO(flo) IMPORTANT(flo): we need to textify the tag once we used it
	kh_lnext(VertexBuffer, buffer, render->first_vertex_buffer) {
		VertexFormat fmt = buffer->format;
		stbsp_sprintf(str_buff, "VERTEX_FORMAT %s\n", G_VERTEX_FORMATS[fmt]);
		file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);
		kh_lnext(Shading, shading, buffer->first) {
			u32 texture_count = shading->texture_count;
			u32 mat_size = shading->size;
			MaterialType mat_instance_type = shading->mat_type;

			stbsp_sprintf(str_buff, "MATERIAL %s,%u,%u,%s\n", 
			              G_SHADING_TYPES[shading->type], mat_size, texture_count, 
			              G_MATERIAL_TYPES[mat_instance_type]);
			file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);

			char mat_reserved[count_reserved_size + 1];
			kh_lu0(i, count_reserved_size) {
				mat_reserved[i] = ' ';
			}
			mat_reserved[count_reserved_size] = '\n';
			u32 mat_instance_offset = file_offset;
			file_offset += g_platform.write_bytes_to_file(&file, file_offset, count_reserved_size + 1, mat_reserved);

			u32 mat_instance_count = 0;
			kh_lnext(Material, mat, shading->first) {
				u8 *mat_data = (u8 *)(mat + 1) + (sizeof(AssetID) * texture_count);

				file_offset += g_platform.write_bytes_to_file(&file, file_offset, mat_size, mat_data);
				char ret = '\n';
				file_offset += g_platform.write_bytes_to_file(&file, file_offset, 1, &ret);

				stbsp_sprintf(str_buff, "TEXTURES\n");
				file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);
				if(texture_count) {
					AssetID *textures = (AssetID *)(mat + 1);
					kh_lu0(t_i, texture_count) {
						Asset *asset = get_asset(assets, textures[t_i], AssetType_tex2d);
						char *name;
						if(textures[t_i].val < assets->count_from_package) {
							name = G_ASSET_NAMES[asset->name];
						} else {
							name = asset->cname;
						}
						stbsp_sprintf(str_buff, "%s\n", name);
						file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);
					}
				}

				kh_lnext(MeshRenderer, meshr, mat->first) {
					AssetID mesh_id = meshr->mesh;
					TriangleMesh trimesh = get_trimesh(assets, mesh_id);
					Asset *mesh_asset = get_asset(assets, mesh_id, AssetType_trimesh);
					char *mesh_name;
					if(mesh_id.val < assets->count_from_package) {
						mesh_name = G_ASSET_NAMES[mesh_asset->name];
					} else {
						mesh_name = mesh_asset->cname;
					}
					stbsp_sprintf(str_buff, "MESH %s, %u\n", mesh_name, meshr->entry_count);
					file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);
					u32 entry_ind = meshr->first_entry;
					kh_lu0(m_i, meshr->entry_count) {
						RenderEntry *entry = render->render_entries + entry_ind;
						mat4 wld = entry->tr;
						stbsp_sprintf(str_buff, "{{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f}}\n",
						              wld.e[0], wld.e[1], wld.e[2], wld.e[3],
						              wld.e[4], wld.e[5], wld.e[8], wld.e[7],
						              wld.e[8], wld.e[9], wld.e[10], wld.e[11],
						              wld.e[12], wld.e[13], wld.e[14], wld.e[15]);
						file_offset += g_platform.write_bytes_to_file(&file, file_offset, string_length(str_buff), str_buff);
						entry_ind = entry->next_in_mesh_renderer;
						total_tri_count += trimesh.tri_c;
						total_entity_count++;
					}
				}
				mat_instance_count++;
			}
			char mat_str[count_reserved_size];
			i32_to_string(mat_instance_count, mat_str);
			g_platform.write_bytes_to_file(&file, mat_instance_offset, string_length(mat_str), mat_str);
		}	
	}


	char ind_str[count_reserved_size];
	i32_to_string(total_tri_count, ind_str);
	g_platform.write_bytes_to_file(&file, 0, string_length(ind_str), ind_str);

	char ent_str[count_reserved_size];
	i32_to_string(total_entity_count, ent_str);
	g_platform.write_bytes_to_file(&file, count_reserved_size, string_length(ent_str), ent_str);

	file_offset += g_platform.write_bytes_to_file(&file, file_offset, 1, '\0');

	g_platform.close_file(&file);
}

KH_INTERN void
load_scene_for_game(Scene *scene, RenderManager *render, Assets *assets, char *filename) {

	// TODO(flo): make the hash
	TransientLinear tmp = kh_begin_transient(&assets->arena);
	FileHandle file = g_platform.open_file(filename, FileAccess_read, FileCreation_only_open);
	kh_assert(!file.error);

	u32 file_size = g_platform.get_file_size(&file);
	char *file_contents = (char *)kh_push(&assets->arena, file_size);
	g_platform.read_bytes_of_file(&file, 0, file_size, file_contents);

	StringTokenizer str_tok;
	str_tok.pos = file_contents;


	// NOTE(flo): HEADER
	Token tok = get_token_and_next(&str_tok);	
	kh_assert(token_fit(tok, Token_numeric));
	u32 tri_count = token_to_u32(tok);

	tok = get_token_and_next(&str_tok);
	kh_assert(token_fit(tok, Token_numeric));
	u32 total_entity_count = token_to_u32(tok);

	// NOTE(flo): VERTEX_FORMAT
	tok = get_token_and_next(&str_tok);
	kh_assert(word_fit_NNT(tok, "VERTEX_FORMAT"));

	u32 entity_count = 0;
	char *asset_name_prefix = "AssetName_";

	while(word_fit_NNT(tok, "VERTEX_FORMAT")) {

		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_word));

		VertexFormat vert_format_enum = (VertexFormat)get_enum_value_from_hash(assets, tok.text, (u32)tok.text_length);

		// NOTE(flo): MATERIAL
		tok = get_token_and_next(&str_tok);
		kh_assert(word_fit_NNT(tok, "MATERIAL"));
		tok = get_token_and_next(&str_tok);
		ShadingType shading_type = (ShadingType)get_enum_value_from_hash(assets, tok.text, (u32)tok.text_length);
		kh_assert(shading_type < Shading_count);
		// TODO(flo): TEMP(flo): remove this
		kh_assert(shading_type == Shading_phong);
		kh_assert(token_fit(tok, Token_word));
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_comma));
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_numeric));
		u32 mat_size = token_to_u32(tok);
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_comma));
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_numeric));
		u32 texture_count = token_to_u32(tok);
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_comma));
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_word));
		MaterialType mat_type = (MaterialType)get_enum_value_from_hash(assets, tok.text, 
		                                                                                        (u32)tok.text_length);
		tok = get_token_and_next(&str_tok);
		kh_assert(token_fit(tok, Token_numeric));
		u32 mat_instance_count = token_to_u32(tok);

		u32 textures_size = texture_count * sizeof(AssetID);
		u32 total_mat_size = textures_size + mat_size;

		for(u32 inst_i = 0; inst_i < mat_instance_count; ++inst_i) {

			tok = get_token_and_next(&str_tok);
			u8 *material_ptr = 0;
			material_ptr = (u8 *)kh_push(&assets->arena, total_mat_size);
			u8 *mat_data = (u8 *)tok.text;
			u8 *dst_mat = material_ptr + textures_size;
			for(u32 size_i = 0; size_i < mat_size; ++size_i) {
				dst_mat[size_i] = mat_data[size_i];
			}
			str_tok.pos += mat_size - tok.text_length;

			kh_assert(material_ptr);

			// NOTE(flo): TEXTURES
			tok = get_token_and_next(&str_tok);
			kh_assert(word_fit_NNT(tok, "TEXTURES"));
			AssetID *texture_ids = (AssetID *)material_ptr;
			for(u32 tex_i = 0; tex_i < texture_count; ++tex_i) {
				tok = get_token_and_next(&str_tok);
				kh_assert(token_fit(tok, Token_word));

				if(strings_equals_on_size(string_length(asset_name_prefix), tok.text, asset_name_prefix)) {
					AssetName name = (AssetName)get_enum_value_from_hash(assets, tok.text, (u32)tok.text_length);
					// TODO(flo): ASSET TAGS!
					// TODO(flo): ASSET TAGS!
					// TODO(flo): ASSET TAGS!
					// TODO(flo): ASSET TAGS!
					AssetID tex_id = get_first_asset(assets, name);
					texture_ids[tex_i] = tex_id; 

				} else {
					AssetID tex_id = {get_asset_id_from_name(assets, tok.text, (u32)tok.text_length)};	
					texture_ids[tex_i] = tex_id; 
				}
			}

			u8 *instance = add_mat_instance_(render, shading_type, vert_format_enum, total_mat_size, mat_type);
			copy_mat_instance_(material_ptr, instance, total_mat_size);

			// TODO(flo): MESHES and ENTRIES
			tok = get_token_and_next(&str_tok);
			kh_assert(word_fit_NNT(tok, "MESH"));
			tok = get_token_and_next(&str_tok);
			kh_assert(token_fit(tok, Token_word));

			AssetID mesh_id;
			if(strings_equals_on_size(string_length(asset_name_prefix), tok.text, asset_name_prefix)) {
				AssetName name = (AssetName)get_enum_value_from_hash(assets, tok.text, (u32)tok.text_length);
				// TODO(flo): ASSET TAGS!
				// TODO(flo): ASSET TAGS!
				// TODO(flo): ASSET TAGS!
				// TODO(flo): ASSET TAGS!
				mesh_id = get_first_asset(assets, name);

			} else {
				mesh_id = {get_asset_id_from_name(assets, tok.text, (u32)tok.text_length)};	
			}
			u32 mesh_renderer = add_mesh_renderer(render, instance, mesh_id);

			tok = get_token_and_next(&str_tok);
			kh_assert(token_fit(tok, Token_comma));
			tok = get_token_and_next(&str_tok);
			kh_assert(token_fit(tok, Token_numeric));
			u32 entry_count_for_mesh = token_to_u32(tok);

			for(u32 entry_i = 0; entry_i < entry_count_for_mesh; ++entry_i) {
				tok = get_token_and_next(&str_tok);	
				kh_assert(token_fit(tok, Token_open_brace));
				mat4 transform;
				u32 mat_el_count = 0;
				for(u32 col_i = 0; col_i < 4; ++col_i) {
					tok = get_token_and_next(&str_tok);
					kh_assert(token_fit(tok, Token_open_brace));

					for(u32 row_i = 0; row_i < 4; ++row_i) {
						tok = get_token_and_next(&str_tok);
						b32 minus = token_fit(tok, Token_minus);
						f32 mul = 1.0f;
						if(minus) {
							mul = -1.0f;
							tok = get_token_and_next(&str_tok);
						}
						kh_assert(token_fit(tok, Token_decimal));
						transform.e[mat_el_count++] = token_to_f32(tok) * mul;

						if(row_i != 3) {
							tok = get_token_and_next(&str_tok);
							kh_assert(token_fit(tok, Token_comma));
						}
					}

					tok = get_token_and_next(&str_tok);
					kh_assert(token_fit(tok, Token_close_brace));

					if(col_i != 3) {
						tok = get_token_and_next(&str_tok);
						kh_assert(token_fit(tok, Token_comma));
					}
				}
				tok = get_token_and_next(&str_tok);
				kh_assert(token_fit(tok, Token_close_brace));
				// TODO(flo): ANIMATORS!
				// TODO(flo): ANIMATORS!
				// TODO(flo): ANIMATORS!
				// TODO(flo): ANIMATORS!
				// TODO(flo): ANIMATORS!
				add_entity(scene, mesh_renderer, transform, INVALID_U32_OFFSET);
				entity_count++;
			}
		}
		tok = get_token_and_next(&str_tok);
	}

	// TODO(flo): what is that?!!!!!!!!!!!!!!!!!!
	tok = get_token_and_next(&str_tok);

	kh_assert(token_fit(tok, Token_end_of_file));

	g_platform.close_file(&file);
	kh_end_transient(&tmp);
}
#else
#define load_scene_for_game(...)
#define save_scene(...)
#define get_default_level_editor(...)
#define edit_level(...)
#endif