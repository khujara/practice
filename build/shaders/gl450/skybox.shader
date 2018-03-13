#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;
out vec3 tex_coords;

void main() {
	tex_coords = in_pos;
	tex_coords.y = -tex_coords.y;
	vec4 pos = cam.rot_vp * vec4(in_pos, 1.0);
	gl_Position = pos.xyww;
}

#shader fragment
in vec3 tex_coords;
out vec4 color;

void main() {
	color = texture(skybox, tex_coords);
	// color = textureLod(skybox, tex_coords, 0);
}