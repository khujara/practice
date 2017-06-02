layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_NORMAL) in vec3 in_normal;
layout (location = LOC_UV0) in vec2 in_uv;
layout (location = LOC_DRAWID) in int in_drawid; //NOTE(flo): this is the id of each instance of the primCount

// #ifndef GL_ARB_shader_draw_parameters 
layout (location = LOC_INDIRECTCMDID) in int in_indirect_cmdid; //NOTE(flo):this is the id of each call from glMultiDraw
	// #endif

struct CameraTransform {
	mat4 vp;
	mat4 view;
	vec4 pos;
};

struct Transform {
	mat4 model;
};

layout (std140, binding = BIND_CAMERATRANSFORM) uniform CAMERA_BLOCK {
	CameraTransform cam;
};

layout (std140, binding = BIND_TRANSFORM) buffer TRANSFORM_BLOCK {
	Transform tr[];
};

layout (std140, binding = BIND_LIGHTTRANSFORM) uniform LIGHTMATRIX_BLOCK {
	mat4 light_matrix;
};

out VS_OUT {
	vec4 normal0;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}vs_out;

// #ifdef GL_ARB_shader_draw_parameters
// #define DrawCommandID gl_DrawIDARB
// #else
#define IndirectDrawCommandID in_indirect_cmdid;
// #endif

void main()
{
	vec4 obj_pos = vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
	vec4 wld_pos = tr[in_drawid].model * obj_pos;  
	vec4 normal = tr[in_drawid].model * vec4(in_normal, 0.0);
	vec3 frag_pos = vec3(tr[in_drawid].model * vec4(in_pos, 1.0));

	vs_out.normal0 = normal;
	vs_out.wld_pos = wld_pos;
	vs_out.uv0 = in_uv;
	vs_out.view_pos = cam.pos;
	vs_out.indirect_cmd_id = IndirectDrawCommandID;
	vs_out.lightspace_pos = light_matrix * vec4(frag_pos, 1.0);

	gl_Position = cam.vp * wld_pos;

}