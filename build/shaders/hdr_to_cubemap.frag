out vec4 final_color;
in vec3 wld_pos;

layout(binding = 0) uniform sampler2D equirect_map;

#define inv_atan vec2(0.1591, 0.3183)

vec2 sample_spherical_map(vec3 v) {
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= inv_atan;
	uv += 0.5;
	return(uv);
}

void main() {
	vec2 uv = sample_spherical_map(normalize(wld_pos));
	vec3 color = texture(equirect_map, uv).rgb;
	final_color = vec4(color, 1.0);
	// final_color = vec4(1,0,0,1);
}