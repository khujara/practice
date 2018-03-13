#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;

uniform mat4 wld_mat; 

void main() {
	vec4 wld_pos = wld_mat * vec4(in_pos, 1.0);
	gl_Position = cam.vp * wld_pos;
}

#shader fragment
uniform vec3 color;
out vec4 final_color;

void main() {
	final_color = vec4(color, 1.0);
}