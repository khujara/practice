#shader vertex
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec4 in_color;

out vec2 uv;
out vec4 color;

uniform mat4 proj_mat;

void main() {
	uv = in_uv;
	color = in_color;
	gl_Position = proj_mat * vec4(in_pos, 0.0, 1.0);	
}

#shader fragment
in vec2 uv;
in vec4 color;

out vec4 out_color;

void main() {
	out_color = color * custom_texture(mat[0].tex, uv);
	// out_color = color * texture(tex_sampler, uv);
}
