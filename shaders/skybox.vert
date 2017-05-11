layout (location = LOC_POSITION) in vec3 in_pos;
out vec3 tex_coords;

struct CameraTransform {
	mat4 vp;
	mat4 vptest;
	vec4 pos;
};

layout (std140, binding = BIND_CAMERATRANSFORM) uniform CAMERA_BLOCK {
	CameraTransform cam;
};

void main() {
	tex_coords = in_pos;
	tex_coords.y = -tex_coords.y;
	vec4 pos = cam.vptest * vec4(in_pos, 1.0);
	gl_Position = pos.xyww;
}