layout (location = LOC_POSITION) in vec3 in_pos;

layout (location = LOC_BONE_IDS) in ivec4 in_bone_ids;
layout (location = LOC_BONE_WEIGHTS) in vec4 in_bone_weights;

layout (location = LOC_DRAWID) in int in_drawid;

struct Transform {
	mat4 model;
};

layout (std140, binding = BIND_BONETRANSFORM) buffer BONE_TRANSFORM {
	mat4 bones[];
};

layout (std430, binding = BIND_BONEOFFSET) buffer BONE_OFFSET {
	uint offset[];
};

layout (std140, binding = BIND_TRANSFORM) buffer TRANSFORM_BLOCK {
	Transform tr[];
};

layout (std140, binding = BIND_LIGHTTRANSFORM) uniform LIGHTMATRIX_BLOCK {
	mat4 light_matrix;
};

void main() {

	uint offset = offset[in_drawid];

	vec4 obj_pos = vec4(in_pos, 1.0);

	if(offset > 0) {
		mat4 bone_transform  = bones[in_bone_ids[0] + (offset - 1)] * in_bone_weights[0];
		bone_transform      += bones[in_bone_ids[1] + (offset - 1)] * in_bone_weights[1];
		bone_transform      += bones[in_bone_ids[2] + (offset - 1)] * in_bone_weights[2];
		bone_transform      += bones[in_bone_ids[3] + (offset - 1)] * in_bone_weights[3];
		obj_pos = bone_transform * obj_pos;
	}

	vec4 wld_pos = tr[in_drawid].model * obj_pos;
	gl_Position = light_matrix * wld_pos;
}