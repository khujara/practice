out vec4 final_color;
in vec3 wld_pos;

layout(binding = 0) uniform samplerCube envmap;

#define PI32 3.14159265359

void main() {

	vec3 N = normalize(wld_pos);

	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, N);
	up = cross(N, right);

	float dx = 0.025;
	float sample_count = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI32; phi += dx) {
		for(float theta = 0.0; theta < 0.5 * PI32; theta += dx) {
			vec3 tangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 uv = tangent.x * right + tangent.y * up + tangent.z * N;
			irradiance += texture(envmap, uv).rgb * cos(theta) * sin(theta);
			sample_count++;
		}
	}
	irradiance = PI32 * irradiance * (1.0 / float(sample_count));
	final_color = vec4(irradiance, 1.0);
}