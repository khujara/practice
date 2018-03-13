layout (location = 0) in vec3 in_pos;
out vec2 uv;

void main() {
	uv = (in_pos.xy + 1.0) * 0.5;
	gl_Position = vec4(in_pos.xy, 0.0, 1.0);
}