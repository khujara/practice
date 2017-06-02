out vec4 color;
in vec2 uv;

layout (std140, binding = BIND_MATERIAL) buffer TEXTURE_BLOCK {
	TexAddress texture;
};

void main() {
	color = custom_texture(texture, uv);
}