layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = 1) in vec3 in_color;

out vec3 out_color;

struct CameraTransform {
	mat4 vp;
	mat4 vptest;
	vec4 pos;
};

layout (std140, binding = BIND_CAMERATRANSFORM) uniform CAMERA_BLOCK {
	CameraTransform cam;
};

void main() {
	out_color = in_color;
	vec4 pos = cam.vp * vec4(in_pos, 1.0);
	gl_Position = pos;
}