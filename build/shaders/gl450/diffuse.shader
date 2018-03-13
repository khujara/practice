#shader vertex
out VS_OUT {
	vec2 uv0;
	flat int indirect_cmd_id;
}vs_out;
void main() {
	vec4 obj_pos = vec4(in_pos, 1.0);
	vec4 wld_pos = tr[in_drawid].model * obj_pos;
	vs_out.uv0 = in_uv;
	vs_out.indirect_cmd_id = IndirectDrawCommandID;

	gl_Position = cam.vp * wld_pos;
}

#shader fragment
in VS_OUT {
	vec2 uv0;
	flat int indirect_cmd_id;
}fs_in;
out vec4 final_color;
void main() {
	Material curmat = mat[fs_in.indirect_cmd_id];
	final_color = custom_texture(curmat.tex, fs_in.uv0);
}