#shader vertex
out VS_OUT {
#ifdef HAS_TB
	mat3 TBN;
#endif
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}vs_out;

void main()
{
	vec4 obj_pos = vec4(in_pos, 1.0);
	vec4 obj_normal    = vec4(in_normal, 0.0);
#ifdef HAS_TB
	vec4 obj_tangent   = vec4(in_tangent, 0.0);
	vec4 obj_bitangent = vec4(in_bitangent, 0.0);
#endif

#ifdef HAS_SKIN
	uint offset = get_offset();
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
#endif

	vec4 wld_pos = tr[in_drawid].model * obj_pos;  
	vec4 normal = tr[in_drawid].model * obj_normal;

#ifdef HAS_TB
	vec4 tangent = tr[in_drawid].model * obj_tangent;
	vec4 bitangent = tr[in_drawid].model * obj_bitangent;
	vs_out.TBN = mat3(normalize(tangent.xyz), normalize(bitangent.xyz), normalize(normal.xyz));
#endif
	vec3 frag_pos = wld_pos.xyz;
	vs_out.TBN = mat3(normalize(tangent.xyz), normalize(bitangent.xyz), normalize(normal.xyz));
	vs_out.wld_pos = wld_pos;
	vs_out.uv0 = in_uv;
	vs_out.view_pos = cam.pos;
	vs_out.lightspace_pos = light_matrix * vec4(frag_pos, 1.0);
	vs_out.indirect_cmd_id = IndirectDrawCommandID;

	gl_Position = cam.vp * wld_pos;
}

#shader fragment
in VS_OUT {
#ifdef HAS_TB
	mat3 TBN;
#endif
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}fs_in;

out vec4 final_color;

#ifdef HAS_TB
vec3 get_normal() {
	Material curmat = mat[fs_in.indirect_cmd_id];
	vec3 res = custom_texture(curmat.normal, fs_in.uv0).rgb;
	res = normalize(2.0 * normalize(res) - vec3(1.0));
	res = normalize(fs_in.TBN * res);
	return(res);
}
#else
vec3 get_normal() {
	Material curmat = mat[fs_in.indirect_cmd_id];
	vec3 tangent_normal = custom_texture(curmat.normal, fs_in.uv0).xyz * 2.0 - 1.0;

	vec3 q_1 = dFdx(fs_in.wld_pos.xyz);
	vec3 q_2 = dFdy(fs_in.wld_pos.xyz);
	vec2 st_1 = dFdx(fs_in.uv0);
	vec2 st_2 = dFdy(fs_in.uv0);

	vec3 N = normalize(fs_in.normal);
	vec3 T = normalize(q_1 * st_2.t - q_2 * st_1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T,B,N);

	return(normalize(TBN * tangent_normal));
}
#endif 

// TODO(flo): we need to pass values (lightPos, viewPos, tangent fragpos) in TBN space to avoid having 
// a lot of matrix multiplication in the fragment shader, but we do not have light position for now
// we only pass light direction

// NOTATIONS : L for light direction, V for view direction, N for normal direction, H for halfway (L+V) direction 
void main() {
	float ambient_intensity = 0.15f;
	float diffuse_intensity = 0.8f;

	LightProperty light_prop = get_light_property(light, fs_in.wld_pos.xyz);

	Material curmat = mat[fs_in.indirect_cmd_id];

	// vec4 albedo = custom_texture(mat[fs_in.indirect_cmd_id].albedo, fs_in.uv0);
	vec4 albedo = custom_texture(curmat.diffuse, fs_in.uv0);
	// vec4 albedo = vec4(1,1,1,1);

	vec3 ambient = light_prop.color * ambient_intensity;

	// @NOTE(flo): point light calculation
	// vec3 L = normalize(light_pos_or_dir - fs_in.wld_pos.xyz);
	vec3 L = light_prop.L;

	// vec3 N = fs_in.TBN[2];
	// vec3 N = custom_texture(curmat.normal, fs_in.uv0).rgb;
	vec3 N = get_normal();

	float normal_angle = dot(N, L);
	float diffuse_factor = max(normal_angle, 0.0);
	vec3 diffuse = light_prop.color * diffuse_intensity * diffuse_factor;

	vec3 V = normalize(fs_in.view_pos.xyz - fs_in.wld_pos.xyz);
	vec3 H = normalize(L + V);
	// vec3 R = reflect(-L, N);
	// @NOTE(flo): same as above with less calculation
	// vec3 P = dot(N, L)*N;
	// vec3 R = L + 2 * (P - L);

	// float spec_factor = max(dot(R, V), 0.0);
	float spec_power = curmat.spec_power;
	float spec_intensity = curmat.spec_intensity;

	float spec_factor = max(dot(N, H), 0.0);
	vec3 specular = light_prop.color * pow(spec_factor, spec_power) * spec_intensity;

	vec3 proj_coords = fs_in.lightspace_pos.xyz / fs_in.lightspace_pos.w;
	proj_coords = 0.5 * proj_coords + vec3(0.5, 0.5, 0.5);
	float closest_depth = custom_texture(shadow_map, proj_coords.xy).r;
	// float closest_depth = 1;
	float cur_depth = proj_coords.z;
	// float bias = 0.005;
	float bias = max(0.005 * (1.0 - dot(N, L)), 0.005);
	float shadow = 1.0 - step(closest_depth, cur_depth - bias);
	// shadow = 1.0;

	final_color = albedo * vec4(ambient + shadow * (specular + diffuse), 1.0);
}