#ifdef KH_IN_DEVELOPMENT
KH_INLINE void
debug_line(DebugState *debug, v3 line_0, v3 line_1, v3 color) {
	if(debug && debug->show) {
		kh_assert(debug->lines.count + 2 <= debug->lines.max_count);
		DebugLine *lines = debug->lines.data + debug->lines.count;
		lines[0].pos = line_0;
		lines[0].color = color;
		lines[1].pos = line_1;
		lines[1].color = color;
		debug->lines.count += 2;
	}
}

KH_INLINE void
debug_box(DebugState *debug, AABB box, v3 color) {
	if(debug && debug->show) {
		debug_line(debug, kh_vec3(box.min.x, box.min.y, box.min.z), kh_vec3(box.max.x, box.min.y, box.min.z), color);
		debug_line(debug, kh_vec3(box.min.x, box.min.y, box.min.z), kh_vec3(box.min.x, box.min.y, box.max.z), color);
		debug_line(debug, kh_vec3(box.max.x, box.min.y, box.min.z), kh_vec3(box.max.x, box.min.y, box.max.z), color);
		debug_line(debug, kh_vec3(box.min.x, box.min.y, box.max.z), kh_vec3(box.max.x, box.min.y, box.max.z), color);

		debug_line(debug, kh_vec3(box.min.x, box.min.y, box.min.z), kh_vec3(box.min.x, box.max.y, box.min.z), color);
		debug_line(debug, kh_vec3(box.max.x, box.min.y, box.min.z), kh_vec3(box.max.x, box.max.y, box.min.z), color);
		debug_line(debug, kh_vec3(box.min.x, box.min.y, box.max.z), kh_vec3(box.min.x, box.max.y, box.max.z), color);
		debug_line(debug, kh_vec3(box.max.x, box.min.y, box.max.z), kh_vec3(box.max.x, box.max.y, box.max.z), color);

		debug_line(debug, kh_vec3(box.min.x, box.max.y, box.min.z), kh_vec3(box.min.x, box.max.y, box.max.z), color);
		debug_line(debug, kh_vec3(box.min.x, box.max.y, box.min.z), kh_vec3(box.max.x, box.max.y, box.min.z), color);
		debug_line(debug, kh_vec3(box.max.x, box.max.y, box.min.z), kh_vec3(box.max.x, box.max.y, box.max.z), color);
		debug_line(debug, kh_vec3(box.min.x, box.max.y, box.max.z), kh_vec3(box.max.x, box.max.y, box.max.z), color);
	}
}

KH_INLINE void
debug_hemisphere(DebugState *debug, mat4 tr, v3 color, f32 scale) {
	if(debug && debug->show) {
		kh_assert(debug->hemispheres.count + 1 < debug->hemispheres.max_count);
		DebugHemisphere *hemisphere = debug->hemispheres.data + debug->hemispheres.count++;
		hemisphere->tr = tr;
		hemisphere->color = color;
		hemisphere->scale = scale;
	}
}

KH_INTERN void
debug_circle_mesh(DebugState *debug, v3 color, f32 r) {
	if(debug && debug->show) {
		const u32 NUM_TRIANGLES = 50;	

		f32 dtheta = TAU32 / NUM_TRIANGLES;

		v3 center = kh_vec3(0,0,0);

		for(u32 i = 0; i < (NUM_TRIANGLES ); ++i) {
			f32 theta0 = dtheta * i;
			f32 theta1 = dtheta * (i + 1);

			v3 vert_0 = center;
			v3 vert_1 = center + r * kh_vec3(kh_cos_f32(theta0), kh_sin_f32(theta0), 0);
			v3 vert_2 = center + r * kh_vec3(kh_cos_f32(theta1), kh_sin_f32(theta1), 0);

			debug_line(debug, vert_0, vert_1, color);
			debug_line(debug, vert_1, vert_2, color);
		}
	}
}

KH_INTERN b32
debug_button(DebugState *debug, char *name) {
	b32 res = false;
	if(debug && debug->show) {
		res = ImGui::Button(name);
	}
	return(res);
}

KH_INTERN void
debug_begin_wnd(DebugState *debug, char *name, b32 *show) {
	if(debug && debug->show) {
		ImGui::Begin(name, (bool *)show);
	}
}

KH_INTERN void
debug_text(DebugState *debug, char *txt) {
	if(debug && debug->show) {
		ImGui::Text(txt);
	}
}

KH_INTERN void
debug_same_line(DebugState *debug) {
	if(debug && debug->show) {
		ImGui::SameLine();
	}
}

KH_INTERN void
debug_radio_button(DebugState *debug, char *name, u32 val, u32 *dst) {
	if(debug && debug->show) {
		ImGui::RadioButton(name, (int *)dst, val);
	}
}

KH_INTERN void
debug_combo_box(DebugState *debug) {
	NOT_IMPLEMENTED;	
}

KH_INTERN void
debug_text_input_f32(DebugState *debug, char *name, char *dst, u32 dst_size, f32 *in) {
	if(debug && debug->show) {
		if(ImGui::InputText(name, dst, dst_size, ImGuiInputTextFlags_CharsDecimal)) {
			*in = str_to_f32(dst, string_length(dst));
		}
	}
}

KH_INTERN void
debug_input_i32(DebugState *debug, char *name, i32 min, i32 max, i32 *in) {
	if(debug && debug->show) {
		ImGui::InputInt(name, in, min, max);
	}
}

KH_INTERN void
debug_input_f32(DebugState *debug, char *name, f32 min, f32 max, f32 *in) {
	if(debug && debug->show) {
		ImGui::InputFloat(name, in, min, max);
	}
}

KH_INTERN b32
debug_collapse(DebugState *debug, char *name) {
	b32 res = false;
	if(debug && debug->show) {
		res = (b32)ImGui::CollapsingHeader(name);
	}
	return(res);
}

KH_INTERN b32
debug_tree_node(DebugState *debug, char *name) {
	b32 res = false;
	if(debug && debug->show) {
		res = (b32)ImGui::TreeNode(debug, name);
	}
	return(res);
}

KH_INTERN void
debug_open_next_tree_node(DebugState *debug, b32 open) {
	if(debug && debug->show) {
		ImGui::SetNextTreeNodeOpen((bool)open, 0);
	}
}

KH_INTERN b32
debug_tree_node(DebugState *debug, char *name, u32 val) {
	b32 res = false;
	if(debug && debug->show) {
		res = (b32)ImGui::TreeNode((void *)(umm)val, name);
	}
	return(res);
}

KH_INTERN void
debug_tree_pop(DebugState *debug) {
	if(debug && debug->show) {
		ImGui::TreePop();
	}
}

KH_INTERN void
debug_color_hdr(DebugState *debug, char *name, v3 *color) {
	if(debug && debug->show) {
		ImGui::ColorEdit3(name, (f32 *)color, ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_Float);
	}
}

KH_INTERN void
debug_color(DebugState *debug, char *name, v3 *color) {
	if(debug && debug->show) {
		ImGui::ColorEdit3(name, (f32 *)color, ImGuiColorEditFlags_Float);
	}
}

KH_INTERN void
debug_push_color(DebugState *debug, ImGuiCol idx, v4 color) {
	if(debug && debug->show) {
		ImVec4 val = {color.x, color.y, color.z, color.w};
		ImGui::PushStyleColor(idx, val);
	}
}

KH_INTERN void
debug_pop_color(DebugState *debug) {
	if(debug && debug->show) {
		ImGui::PopStyleColor(1);
	}
}

KH_INTERN void
debug_slider_f32(DebugState *debug, char *name, f32 min, f32 max, f32 *in) {
	if(debug && debug->show) {
		ImGui::SliderFloat(name, in, min, max);
	}
}

KH_INTERN f32
debug_slider_f32(DebugState *debug, char *name, f32 min, f32 max, f32 in) {
	f32 res = in;
	if(debug && debug->show) {
		f32 change = in;
		if(ImGui::SliderFloat(name, &change, min, max)) {
			res = change;
		}
	}
	return(res);
}

KH_INTERN void
debug_check_box(DebugState *debug, char *name, b32 *check) {
	if(debug && debug->show) {
		ImGui::Checkbox(name, (bool *)check);
	}
}

KH_INTERN void
debug_end_wnd(DebugState *debug) {
	if(debug && debug->show) {
		ImGui::End();
	}
}

KH_INTERN void
debug_begin_frame(DebugState *debug, u32 render_w, u32 render_h, u32 wnd_w, u32 wnd_h, f32 dt) {
	if(debug && debug->show) {
		Input *input = debug->input;
		debug->render_w = render_w;
		debug->render_h = render_h;
		debug->wnd_w = wnd_w;
		debug->wnd_h = wnd_h;
		debug->lines.count = 0;
		debug->hemispheres.count = 0;

		// ImGui::SetCurrentContext(debug->imctx);

		ImGuiIO &io = ImGui::GetIO();
		io.DisplaySize = ImVec2((f32)render_w, (f32)render_h);
		io.DeltaTime = dt;
		io.Fonts->TexID = debug->imgui_font_id;

		io.MouseDown[0] = is_down(input->mouse_buttons[MouseButton_left]);
		io.MousePos = ImVec2(input->mouse_rx * render_w, render_h - input->mouse_ry * render_h);

		kh_lu0(i, array_count(input->debug_buttons)) {
			io.KeysDown[i] = is_down(input->debug_buttons[i]);
			if(was_pressed(input->debug_buttons[i])) {
				io.AddInputCharacter(i);
			}
		}

		ImGui::NewFrame();
	}
}

KH_INTERN void
debug_end_frame(DebugState *debug) {
	if(debug && debug->show) {
		ImGui::Render();
		debug->imgui_data = ImGui::GetDrawData();
	}
}
#else
#define debug_line(...)
#define debug_hemisphere(...)
#define debug_circle_mesh(...)
#define debug_button(...) 0
#define debug_text(...)
#define debug_text_input_decimal(...) 0
#define debug_begin_wnd(...)
#define debug_end_wnd(...)
#define debug_begin_frame(...)
#define debug_end_frame(...)
#endif