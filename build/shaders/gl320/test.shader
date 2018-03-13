#shader vertex
out VS_OUT {
	// vec4 wld_pos;
	// vec4 normal0;
	// vec4 view_pos;
	// vec4 lightspace_pos;
	// vec2 uv0;
}vs_out;

void main() {
	vec4 obj_pos    = vec4(in_pos, 1.0);
	// vec4 obj_normal = vec4(in_normal, 0.0);

	vec4 wld_pos = model * obj_pos;
	// vec4 normal  = model * obj_normal;
	// vec3 frag_pos = wld_pos.xyz;
	// vs_out.normal0 = normal;
	// vs_out.wld_pos = wld_pos;
	// vs_out.uv0 = in_uv;
	// vs_out.view_pos = cam.pos;
	// vs_out.lightspace_pos = lightmatrix * vec4(frag_pos, 1.0);

	gl_Position = viewproj * wld_pos;
}


#shader fragment
out VS_OUT {
	// vec4 wld_pos;
	// vec4 normal0;
	// vec4 view_pos;
	// vec4 lightspace_pos;
	// vec2 uv0;
}fs_in;

void main() {
	final_color = vec4(1,0,0,1);
}