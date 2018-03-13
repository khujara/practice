#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = 1) in vec3 in_color;

out vec3 out_color;

void main() {
	out_color = in_color;
	vec4 pos = cam.vp * vec4(in_pos, 1.0);
	gl_Position = pos;
}

#shader fragment
in vec3 out_color;
out vec4 final_color;

void main() {
	final_color = vec4(out_color, 1.0);
}