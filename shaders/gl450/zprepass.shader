#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_DRAWID) in int in_drawid;

void main() {
	vec4 obj_pos = vec4(in_pos, 1.0);
	vec4 wld_pos = tr[in_drawid].model * obj_pos;
	gl_Position = cam.vp * wld_pos;
}

#shader fragment
void main() {
}