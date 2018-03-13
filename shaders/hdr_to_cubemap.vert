layout (location = 0) in vec3 in_pos;

out vec3 wld_pos;

uniform mat4 proj;
uniform mat4 view;

void main() {
	wld_pos = in_pos;
	gl_Position = proj * view * vec4(wld_pos, 1.0);
}