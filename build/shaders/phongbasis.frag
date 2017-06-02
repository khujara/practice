in VS_OUT {
	vec4 normal0;
	vec4 wld_pos;
	vec4 view_pos;
	vec4 lightspace_pos;
	vec2 uv0;
	flat int indirect_cmd_id;
}fs_in;

out vec4 final_color;

struct DiffuseLight {
	vec3 color;
	float ambient_intensity;
	vec3 dir;
	float diffuse_intensity;
};

layout (std140, binding = BIND_LIGHT) uniform LIGHT_BLOCK {
	DiffuseLight light;
};

layout (std140, binding = BIND_COLORS) uniform COLOR_BLOCK {
	vec4 color[MAX_COLORS];
};

layout (std140, binding = BIND_MATERIAL) buffer TEXTURE_BLOCK {
    TexAddress tex[][2];
};

layout (std140, binding = BIND_SHADOWMAP) uniform SHADOW_BLOCK {
	TexAddress shadow_map;
};

layout (binding = BIND_SKYBOX) uniform samplerCube skybox;

uniform float spec_power = 128.0;
uniform float spec_intensity = 1.0;

void main() {
	// vec4 albedo = color[fs_in.indirect_cmd_id] * custom_texture(mat[fs_in.indirect_cmd_id].albedo, fs_in.uv0);
	vec4 albedo = color[fs_in.indirect_cmd_id] * custom_texture(tex[fs_in.indirect_cmd_id][0], fs_in.uv0);

	vec3 ambient = light.color * light.ambient_intensity;

	// @NOTE(flo): point light calculation
	// vec3 L = normalize(light.dir - fs_in.wld_pos.xyz);
	vec3 L = normalize(-light.dir);
	vec3 N = normalize(fs_in.normal0.xyz);

	float normal_angle = dot(N, L);
	float diffuse_factor = max(normal_angle, 0.0);
	vec3 diffuse = light.color * light.diffuse_intensity * diffuse_factor;

	vec3 V = normalize(fs_in.view_pos.xyz - fs_in.wld_pos.xyz);
	// vec3 R = reflect(-L, N);
	vec3 H = normalize(L + V);
	// @NOTE(flo): same as above with less calculation
	// vec3 P = dot(N, L)*N;
	// vec3 R = L + 2 * (P - L);

	// float spec_factor = max(dot(R, V), 0.0);
	float spec_factor = max(dot(N, H), 0.0);
	vec3 specular = light.color * pow(spec_factor, spec_power) * spec_intensity;

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

	float reflect_intensity = 0.2;
	vec4 reflect_color = texture(skybox, R) * reflect_intensity;

	final_color = (albedo + reflect_color) * vec4(ambient + shadow * (specular + diffuse), 1.0);

	// vec3 I = normalize(fs_in.wld_pos.xyz - fs_in.view_pos.xyz);
	// // vec3 R = reflect(I, N);
	// R = refract(I, N, 1 / 2.42);
	// R.y = -R.y;	
	// final_color = texture(skybox, R);
}