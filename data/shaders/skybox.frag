in vec3 tex_coords;
out vec4 color;

layout (binding = BIND_SKYBOX) uniform samplerCube skybox;

void main() {
	color = texture(skybox, tex_coords);
}