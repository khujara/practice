layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_DRAWID) in int in_drawid;

struct Transform {
	mat4 model;
};

struct CameraTransform {
	mat4 vp;
	mat4 view;
	vec4 pos;
};

layout (std140, binding = BIND_TRANSFORM) buffer TRANSFORM_BLOCK {
	Transform tr[];
};

layout (std140, binding = BIND_CAMERATRANSFORM) uniform CAMERA_BLOCK {
	CameraTransform cam;
};

void main() {
	vec4 obj_pos = vec4(in_pos, 1.0);
	vec4 wld_pos = tr[in_drawid].model * obj_pos;
	gl_Position = cam.vp * wld_pos;
}