#shader vertex
layout (location = LOC_POSITION) in vec3 in_pos;
layout (location = LOC_BONE_IDS) in ivec4 in_bone_ids;
layout (location = LOC_BONE_WEIGHTS) in vec4 in_bone_weights;

out flat uint index;

layout (std140, binding = BIND_BONETRANSFORM) buffer BONE_TRANSFORM {
	mat4 bones[];
};

uniform mat4 vp;
uniform mat4 wld;
uniform uint uni_index;
uniform uint bone_offset;

void main() {
	vec4 obj_pos = vec4(in_pos, 1.0);
	if(bone_offset > 0) {
		mat4 bone_transform  = bones[in_bone_ids[0] + (bone_offset - 1)] * in_bone_weights[0];
		bone_transform      += bones[in_bone_ids[1] + (bone_offset - 1)] * in_bone_weights[1];
		bone_transform      += bones[in_bone_ids[2] + (bone_offset - 1)] * in_bone_weights[2];
		bone_transform      += bones[in_bone_ids[3] + (bone_offset - 1)] * in_bone_weights[3];
		obj_pos       = bone_transform * obj_pos;
	}
	index = uni_index;
	vec4 wld_pos = wld * obj_pos; 
	gl_Position = vp * wld_pos;
}

#shader fragment
out uint color;
// out vec4 color;

in flat uint index;

void main() {
	color = index;
	// color = vec4(1,0,0,1);
}