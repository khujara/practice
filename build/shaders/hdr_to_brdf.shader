#shader vertex
layout(location = 0) in vec3 in_pos;

out vec2 uv;

void main() {
	// uv = in_uv.xy;
	uv = (in_pos.xy + 1.0) * 0.5;
	gl_Position = vec4(in_pos, 1.0);
}

#shader fragment
out vec2 final_color;
#define M_PI 3.1415926535897932384626433832795

in vec2 uv;

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

float geometry_schlick_GGX(float n_dot_v, float k) {
	float r = n_dot_v * (1.0 - k) + k;
	float res = n_dot_v / r;
	return(res);
}

float geometry_smith(float n_dot_v, float n_dot_l, float roughness)
{
	// float r = (roughness + 1.0);
	// float k = (r*r) / 8.0;
	float k = (roughness * roughness) / 2.0;
    float ggx2 = geometry_schlick_GGX(n_dot_v, k);
    float ggx1 = geometry_schlick_GGX(n_dot_l, k);
    float res = ggx1 * ggx2;
    return (res);
}

// NOTE(flo): Importance Sampling see http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 importance_sampling_ggx(vec2 x_i, vec3 N, float roughness) {

	float a = roughness * roughness;

	// NOTE(flo) : Solid angle from generated low-discrepancy sequence
	float phi = 2.0f * M_PI * x_i.x;
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

vec2 integrate_brdf(float n_dot_v, float roughness) {
    vec3 V;
    V.x = sqrt(1.0 - n_dot_v*n_dot_v);
    V.y = 0.0;
    V.z = n_dot_v;

	float A = 0.0;
	float B = 0.0;

	vec3 N = vec3(0.0,0.0,1.0);

	const uint SAMPLE_COUNT = 1024u;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
		vec2 x_i = hammersley(i, SAMPLE_COUNT);
		vec3 H = importance_sampling_ggx(x_i, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float n_dot_l = max(L.z, 0.0);
		float n_dot_h = max(H.z, 0.0);
		float h_dot_v = max(dot(H, V), 0.0);
		if(n_dot_l > 0.0) {
			float G = geometry_smith(n_dot_v, n_dot_l, roughness);
			float G_Vis = (G * h_dot_v) / (n_dot_h * n_dot_v);
			float Fc = pow(1.0 - h_dot_v, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A = A / float(SAMPLE_COUNT);
	B = B / float(SAMPLE_COUNT);
	vec2 res = vec2(A, B);
	return(res);
}

void main() {
	final_color = integrate_brdf(uv.x, uv.y);
}
