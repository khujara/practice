out vec4 final_color;
in vec3 wld_pos;

layout(binding = 0) uniform samplerCube envmap;
uniform float roughness;
uniform float resolution;

#define PI32 3.14159265359

float distribution_GGX(float n_dot_h, float roughness) {
	float m = roughness*roughness;
	float m2 = m*m;
	float c2 = n_dot_h * n_dot_h;

	float r = (c2 * (m2 - 1.0) + 1.0);
	float r2 = r * r * PI32;

	float res = m2 / r2;
	return(res);
}

float radical_inverse_van_der_corpus(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float res = float(bits) * 2.3283064365386963e-10; // / 0x100000000 
    return (res);
}

vec2 hammersley(uint i, uint count) {
	vec2 res = vec2(float(i)/float(count), radical_inverse_van_der_corpus(i));
	return(res);
}

// NOTE(flo): Importance Sampling see http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 importance_sampling_ggx(vec2 x_i, vec3 N, float roughness) {

	float a = roughness * roughness;

	// NOTE(flo) : Solid angle from generated number
	float phi = 2.0f * PI32 * x_i.x;
	float cos_theta = sqrt((1.0 - x_i.y) / (1.0 + (a * a - 1.0) * x_i.y));
	float sin_theta = sqrt(1.0 - cos_theta*cos_theta);

	vec3 H;
	H.x = cos(phi) * sin_theta;
	H.y = sin(phi) * sin_theta;
	H.z = cos_theta;

	vec3 up = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1,0,0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 uv = tangent * H.x + bitangent * H.y + N * H.z;
	return(normalize(uv));
}

void main() {
	vec3 N = normalize(wld_pos);
	vec3 V = N;

	const uint SAMPLE_COUNT = 1024u;
	float total_weight = 0.0;
	vec3 Li = vec3(0.0);

	for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
		vec2 x_i = hammersley(i, SAMPLE_COUNT);
		vec3 H = importance_sampling_ggx(x_i, N, roughness);
		vec3 L = normalize(2.0 * dot(V,H) * H - V);
		float n_dot_l = max(dot(N, L), 0.0);

		float n_dot_h = max(dot(N, H), 0.0);
		float h_dot_v = max(dot(H, V), 0.0);
		float D = distribution_GGX(n_dot_h, roughness);
		float pdf = D * n_dot_h / ((4.0 * h_dot_v) + 0.0001); 
		float sa_texel = 4.0 * PI32 / (6.0 * resolution * resolution);
		float sa_sample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
		float mip_level = (roughness == 0.0) ? 0.0 : 0.5 * log2(sa_sample / sa_texel);

		// Li += texture(envmap, L).rgb * n_dot_l;
		Li += textureLod(envmap, L, mip_level).rgb * n_dot_l;
		total_weight += n_dot_l;
	}
	Li = Li / total_weight;
	final_color = vec4(Li, 1.0);
}