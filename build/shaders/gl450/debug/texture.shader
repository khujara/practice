#shader vertex
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_uv;
out vec2 uv;

void main() {
	uv = in_uv.xy;
	gl_Position = vec4(in_pos, 1.0);
}

#shader fragment
out vec4 color;
in vec2 uv;

void main() {
	color = custom_texture(mat[0].tex, uv);
}