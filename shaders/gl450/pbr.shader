#shader vertex
out VS_OUT {
#ifdef HAS_TB
	mat3 TBN;
#endif
	vec3 normal;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}vs_out;

void main()
{
	vec4 obj_pos       = vec4(in_pos, 1.0);
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
	vs_out.normal = normal.xyz;
	vs_out.uv0 = in_uv;
	vs_out.indirect_cmd_id = IndirectDrawCommandID;
	vs_out.lightspace_pos = light_matrix * vec4(frag_pos, 1.0);
	vs_out.view_pos = cam.pos;
	vs_out.wld_pos = wld_pos;

	gl_Position = cam.vp * wld_pos;
}

#shader fragment
in VS_OUT {
#ifdef HAS_TB
	mat3 TBN;
#endif
	vec3 normal;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}fs_in;

out vec4 final_color;

layout (binding = BIND_PREFILTER) uniform samplerCube sampler_prefilter;
layout (binding = BIND_BRDF_LUT) uniform sampler2D sampler_brdf_lut;

#define M_PI 3.1415926535897932384626433832795

float distribution_GGX(float n_dot_h, float roughness) {
	float m = roughness*roughness;
	float m2 = m*m;
	float c2 = n_dot_h * n_dot_h;

	float r = (c2 * (m2 - 1.0) + 1.0);
	float r2 = r * r * M_PI;

	float res = m2 / r2;
	return(res);
}

float geometry_schlick_GGX(float n_dot_v, float k) {
	float r = n_dot_v * (1.0 - k) + k;
	float res = n_dot_v / r;
	return(res);
}

float geometry_smith(float n_dot_v, float n_dot_l, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
    float ggx2 = geometry_schlick_GGX(n_dot_v, k);
    float ggx1 = geometry_schlick_GGX(n_dot_l, k);
    float res = ggx1 * ggx2;
    return (res);
}

vec3 fresnel_schlick(float c, vec3 R0) {
	vec3 res = R0 + (1.0 - R0) * pow(1.0 - c, 5.0);
	return(res);
}

vec3 fresnel_schlick_roughness(float c, vec3 R0, float roughness) {
	vec3 res = R0 + (max(vec3(1.0 - roughness), R0) - R0) * pow(1.0 - c, 5.0);
	return(res);
}

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

void main() {
	Material curmat = mat[fs_in.indirect_cmd_id];

	float roughness = custom_texture(curmat.roughness, fs_in.uv0).r;
	// float roughness = custom_texture(curmat.roughness, fs_in.uv0).r;
	float metallic = custom_texture(curmat.metallic, fs_in.uv0).r;
	float ao = custom_texture(curmat.ao, fs_in.uv0).r;
	vec4 albedo_tex = custom_texture(curmat.diffuse, fs_in.uv0);
	vec3 albedo = albedo_tex.rgb;
	albedo = pow(albedo, vec3(2.2));
	vec3 N = get_normal();
	
	// roughness = 0.05;
	// metallic = 1.0;
	// albedo = vec3(1.0, 0.765557, 0.336057);
	// ao = 1.0;
	// N = normalize(fs_in.normal);

	vec3 V = normalize(fs_in.view_pos.xyz - fs_in.wld_pos.xyz);
	vec3 R = reflect(-V, N);
	float n_dot_v = max(dot(N, V), 0.0);

	vec3 R0 = vec3(0.04);
	R0 = mix(R0, albedo, metallic);

	vec3 proj_coords = fs_in.lightspace_pos.xyz / fs_in.lightspace_pos.w;
	proj_coords = 0.5 * proj_coords + vec3(0.5);
	float cur_depth = proj_coords.z;

	vec3 Lo = vec3(0.0);
	float shadow = 0.0;
	{
		LightProperty light_prop = get_light_property(light, fs_in.wld_pos.xyz);
		light_prop.color = light_prop.color;
		vec3 L = light_prop.L;
		vec3 H = normalize(L + V);
		vec3 radiance = light_prop.color * light_prop.attenuation * light_prop.intensity;

		float h_dot_v = max(dot(H, V), 0.0);
		float n_dot_l = max(dot(N, L), 0.0);
		// float n_dot_l = pow(max(dot(N, L), 0.0), 0.1);
		float n_dot_h = max(dot(N, H), 0.0);

		float ndf = distribution_GGX(n_dot_h, roughness); 
		float g = geometry_smith(n_dot_v, n_dot_l, roughness);
		vec3 f = fresnel_schlick(h_dot_v, R0);

		vec3 brdf_nom = ndf * g * f;
		float brdf_denom = 4 * n_dot_v * n_dot_l + 0.001;
		vec3 specular = brdf_nom / brdf_denom;

		vec3 ks = f;
		vec3 kd = vec3(1.0) - ks;
		kd = kd * (1.0 - metallic);

		Lo += (kd * albedo / M_PI + specular) * radiance * n_dot_l;
		// Lo += (kd * albedo / M_PI + specular) * radiance * pow(n_dot_l, 0.05);

		float closest_depth = custom_texture(shadow_map, proj_coords.xy).r;
		float bias = max(0.005 * (1.0 - dot(N, L)), 0.005);
		shadow = 1.0 - step(closest_depth, cur_depth - bias);
	}

	vec3 f = fresnel_schlick(max(dot(N, V), 0.0), R0);
	vec3 ks = f;
	vec3 kd = 1.0 - ks;
	kd = kd * (1.0 - metallic);
	vec3 irradiance = texture(skybox, N).rgb;
	vec3 diffuse = irradiance * albedo;

	const float MAX_REFLECTION_LOD = 5.0;
	vec3 prefiltered_color = textureLod(sampler_prefilter, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(sampler_brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefiltered_color * (f * brdf.x + brdf.y);
	// NOTE(flo): brdf.x = scale F0 and brdf.y = bias to F0

	vec3 ambient = (kd * diffuse + specular) * ao;

	// vec3 color = Lo * shadow + ambient;
	vec3 color = Lo + ambient;

	color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
	final_color = vec4(color, 1.0);
}