layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_NORMAL) in vec3 in_normal;
layout (location = LOC_UV0) in vec2 in_uv;
layout (location = LOC_TANGENT) in vec3 in_tangent;
layout (location = LOC_BITANGENT) in vec3 in_bitangent;
layout (location = LOC_BONE_IDS) in ivec4 in_bone_ids;
layout (location = LOC_BONE_WEIGHTS) in vec4 in_bone_weights;

//NOTE(flo): this is the id of each instance of the primCount
layout (location = LOC_DRAWID) in int in_drawid;

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

layout (std140, binding = BIND_BONETRANSFORM) buffer BONE_TRANSFORM {
	mat4 bones[];
};

layout (std430, binding = BIND_BONEOFFSET) buffer BONE_OFFSET {
	uint offset[];
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
	mat3 TBN;
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
// #endfi

void main()
{
	uint offset = offset[in_drawid];

	vec4 obj_pos       = vec4(in_pos, 1.0);
	vec4 obj_normal    = vec4(in_normal, 0.0);
	vec4 obj_tangent   = vec4(in_tangent, 0.0);
	vec4 obj_bitangent = vec4(in_bitangent, 0.0);

	if(offset > 0) {
		mat4 bone_transform  = bones[in_bone_ids[0] + (offset - 1)] * in_bone_weights[0];
		bone_transform      += bones[in_bone_ids[1] + (offset - 1)] * in_bone_weights[1];
		bone_transform      += bones[in_bone_ids[2] + (offset - 1)] * in_bone_weights[2];
		bone_transform      += bones[in_bone_ids[3] + (offset - 1)] * in_bone_weights[3];
		obj_pos       = bone_transform * obj_pos;
		obj_normal    = bone_transform * obj_normal;
		obj_tangent   = bone_transform * obj_tangent;
		obj_bitangent = bone_transform * obj_bitangent;
	}

	// bone_transform = bone_transform * inverse(bone_transform);

	vec4 wld_pos = tr[in_drawid].model * obj_pos; 
	vec4 normal = tr[in_drawid].model * obj_normal;
	vec4 tangent = tr[in_drawid].model * obj_tangent;
	vec4 bitangent = tr[in_drawid].model * obj_bitangent;

	vec3 frag_pos = wld_pos.xyz;

	vs_out.TBN = mat3(normalize(tangent.xyz), normalize(bitangent.xyz), normalize(normal.xyz));
	vs_out.wld_pos = wld_pos;
	vs_out.uv0 = in_uv;
	vs_out.view_pos = cam.pos;
	vs_out.lightspace_pos = light_matrix * vec4(frag_pos, 1.0);
	vs_out.indirect_cmd_id = IndirectDrawCommandID;

	gl_Position = cam.vp * wld_pos;
}