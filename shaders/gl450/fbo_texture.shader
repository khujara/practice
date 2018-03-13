#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;
out vec2 uv;

void main() {
	uv = (in_pos.xy + 1.0) * 0.5;
	gl_Position = vec4(in_pos.xy, 0.0, 1.0);
}

#shader fragment
out vec4 color;
in vec2 uv;

layout (std430, binding = BIND_MATERIAL) buffer MATERIAL_BLOCK {
	TexAddress texture;
};

void main() {
	color = custom_texture(texture, uv);
	// color = texture(tex, uv);
}