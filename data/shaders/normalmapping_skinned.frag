in VS_OUT {
	mat3 TBN;
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

uniform float spec_power = 128.0;
uniform float spec_intensity = 1.0;

// TODO(flo): we need to pass values (lightPos, viewPos, tangent fragpos) in TBN space to avoid having 
// a lot of matrix multiplication in the fragment shader, but we do not have light position for now
// we only pass light direction

// NOTATIONS : L for light direction, V for view direction, N for normal direction, H for halfway (L+V) direction 
void main() {
	// vec4 albedo = custom_texture(mat[fs_in.indirect_cmd_id].albedo, fs_in.uv0);
	vec4 albedo = custom_texture(tex[fs_in.indirect_cmd_id][0], fs_in.uv0);
	// vec4 albedo = vec4(1,1,1,1);

	vec3 ambient = light.color * light.ambient_intensity;

	// @NOTE(flo): point light calculation
	// vec3 L = normalize(light.dir - fs_in.wld_pos.xyz);
	vec3 L = normalize(-light.dir);

	// vec3 N = fs_in.TBN[2];
	// vec3 N = custom_texture(mat[fs_in.indirect_cmd_id].normal, fs_in.uv0).rgb;
	vec3 N = custom_texture(tex[fs_in.indirect_cmd_id][1], fs_in.uv0).rgb;
	N = normalize(2.0 * N - vec3(1.0, 1.0, 1.0));
	N = normalize(fs_in.TBN * N);

	float normal_angle = dot(N, L);
	float diffuse_factor = max(normal_angle, 0.0);
	vec3 diffuse = light.color * light.diffuse_intensity * diffuse_factor;

	vec3 V = normalize(fs_in.view_pos.xyz - fs_in.wld_pos.xyz);
	vec3 H = normalize(L + V);
	// vec3 R = reflect(-L, N);
	// @NOTE(flo): same as above with less calculation
	// vec3 P = dot(N, L)*N;
	// vec3 R = L + 2 * (P - L);

	// float spec_factor = max(dot(R, V), 0.0);
	float spec_factor = max(dot(N, H), 0.0);
	vec3 specular = light.color * pow(spec_factor, spec_power) * spec_intensity;

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