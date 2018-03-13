#shader vertex
out VS_OUT {
	vec4 normal0;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}vs_out;

void main() {
	vec4 obj_pos    = vec4(in_pos, 1.0);
	vec4 obj_normal = vec4(in_normal, 0.0);
#ifdef HAS_SKIN
	uint offset = get_offset();
	if(offset > 0) {
		mat4 bone_transform  = bones[in_bone_ids[0] + (offset - 1)] * in_bone_weights[0];
		bone_transform      += bones[in_bone_ids[1] + (offset - 1)] * in_bone_weights[1];
		bone_transform      += bones[in_bone_ids[2] + (offset - 1)] * in_bone_weights[2];
		bone_transform      += bones[in_bone_ids[3] + (offset - 1)] * in_bone_weights[3];
		obj_pos       = bone_transform * obj_pos;
		obj_normal    = bone_transform * obj_normal;
	}
#endif
	vec4 wld_pos = tr[in_drawid].model * obj_pos; 
	vec4 normal = tr[in_drawid].model * obj_normal;
	vec3 frag_pos = wld_pos.xyz;
	vs_out.normal0 = normal;
	vs_out.wld_pos = wld_pos;
	vs_out.uv0 = in_uv;
	vs_out.view_pos = cam.pos;
	vs_out.indirect_cmd_id = IndirectDrawCommandID;
	vs_out.lightspace_pos = light_matrix * vec4(frag_pos, 1.0);
	// vs_out.normal_view = cam.view * vec4(in_normal, 0.0);

	gl_Position = cam.vp * wld_pos;
}

#shader fragment
in VS_OUT {
	vec4 normal0;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}fs_in;

out vec4 final_color;
#define M_PI 3.1415926535897932384626433832795


void main() {
	float ambient_intensity = 0.15f;
	float diffuse_intensity = 0.8f;

	LightProperty light_prop = get_light_property(light, fs_in.wld_pos.xyz);

	Material curmat = mat[fs_in.indirect_cmd_id];

	vec4 albedo = custom_texture(curmat.diffuse, fs_in.uv0);

	vec3 ambient = light_prop.color * ambient_intensity;

	// @NOTE(flo): point light calculation
	// vec3 L = normalize(light_pos_or_dir - fs_in.wld_pos.xyz);
	vec3 L = light_prop.L;
	vec3 N = normalize(fs_in.normal0.xyz);

	float n_dot_l = max(dot(N,L), 0.0);
	vec3 diffuse = light_prop.color * diffuse_intensity * n_dot_l;
	// vec3 diffuse = light_prop.color * diffuse_intensity * pow(n_dot_l, 0.05);

	vec3 V = normalize(fs_in.view_pos.xyz - fs_in.wld_pos.xyz);
	// vec3 R = reflect(-L, N);
	vec3 H = normalize(L + V);
	// @NOTE(flo): same as above with less calculation
	// vec3 P = dot(N, L)*N;
	// vec3 R = L + 2 * (P - L);

	// float spec_factor = max(dot(R, V), 0.0);
	// float spec_power = 128.0;
	// float spec_intensity = 1.0;
	float spec_power = curmat.spec_power;
	float spec_intensity = curmat.spec_intensity;

	float n_dot_h = max(dot(N, H), 0.0);
	vec3 specular = light_prop.color * pow(n_dot_h, spec_power) * spec_intensity;

	vec3 proj_coords = fs_in.lightspace_pos.xyz / fs_in.lightspace_pos.w;
	proj_coords = 0.5 * proj_coords + vec3(0.5, 0.5, 0.5);
	float closest_depth = custom_texture(shadow_map, proj_coords.xy).r;
	float cur_depth = proj_coords.z;
	// float bias = 0.005;
	float bias = max(0.005 * (1.0 - dot(N, L)), 0.005);
	float shadow = 1.0 - step(closest_depth, cur_depth - bias);

	vec3 I = normalize(fs_in.wld_pos.xyz - fs_in.view_pos.xyz);
	vec3 R = reflect(I, N);
	R.y = -R.y;
	// clamp(R.x, 0.0, 1.0);
	// clamp(R.y, 0.0, 1.0);

	float reflect_intensity = curmat.reflect_intensity;
	vec4 reflect_color = texture(skybox, R) * reflect_intensity;

	// final_color = (albedo + reflect_color) * vec4(ambient + shadow * (specular + diffuse), 1.0);
	// final_color = vec4(diffuse, 1.0);
	// final_color = vec4(albedo.xyz, 1.0);
	final_color = shadow * ((vec4(light_prop.color, 1.0) * light_prop.attenuation) * ((1.0f / M_PI) * albedo) * n_dot_l);

	// vec3 I = normalize(fs_in.wld_pos.xyz - fs_in.view_pos.xyz);
	// // vec3 R = reflect(I, N);
	// R = refract(I, N, 1 / 2.42);
	// R.y = -R.y;	
	// final_color = texture(skybox, R);
}