layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_DRAWID) in int in_drawid;

struct Transform {
	mat4 model;
};

layout (std140, binding = BIND_TRANSFORM) buffer TRANSFORM_BLOCK {
	Transform tr[];
};

layout (std140, binding = BIND_LIGHTTRANSFORM) uniform LIGHTMATRIX_BLOCK {
	mat4 light_matrix;
};

void main() {
	vec4 obj_pos = vec4(in_pos, 1.0);
	vec4 wld_pos = tr[in_drawid].model * obj_pos;
	gl_Position = light_matrix * wld_pos;
}